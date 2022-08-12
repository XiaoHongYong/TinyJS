//
//  JsArray.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "JsArray.hpp"


const uint32_t ARRAY_BLOCK_SIZE = 1024 * 32; // 32768
const uint32_t ARRAY_RESERVE_MAX_SIZE = ARRAY_BLOCK_SIZE * 10; // 327680 个
const uint32_t ARRAY_MAX_INDEX = 4294967294; // 2 ** 32 - 2


class BlockLessCompare {
public:
    bool operator()(uint32_t index, const JsArray::Block *other) const {
        return index < other->index + (uint32_t)other->items.size();
    }
    
    bool operator()(const JsArray::Block *other, uint32_t index) const {
        return index > other->index + (uint32_t)other->items.size();
    }

};

inline uint32_t roundIndexToBlock(uint32_t index) {
    if (index < ARRAY_RESERVE_MAX_SIZE) {
        index -= index % ARRAY_BLOCK_SIZE;
    }

    return index;
}

JsArray::JsArray(uint32_t length) {
    if (length > 0) {
        reserveSize(length);
    } else {
        auto block = new Block();
        _blocks.push_back(block);
        _length = 0;
    }

    _needGC = false;
    _firstBlock = _blocks.front();
    _firstBlockItems = &_firstBlock->items;
    _obj = nullptr;
}

JsArray::~JsArray() {
    for (auto b : _blocks) {
        delete b;
    }
    _blocks.clear();

    if (_obj) {
        delete _obj;
    }
}

void JsArray::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return definePropertyByIndex(ctx, (uint32_t)n, descriptor, setter);
        }
    }

    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, prop, descriptor, setter);
}

void JsArray::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    auto block = findToModifyBlock(index);
    index -= block->index;
    assert(index < block->items.size());
    block->items[index] = descriptor;

    if (setter.type > JDT_OBJECT) {
        if (!block->setters) {
            block->setters = new DequeJsValue;
        }

        if (index >= block->setters->size()) {
            block->setters->resize(index + 1, JsUndefinedValue);
        }
        (*block->setters)[index] = setter;
    }
}

void JsArray::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor, setter);
}

bool JsArray::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return getOwnPropertyDescriptorByIndex(ctx, (uint32_t)n, descriptorOut, setterOut);
        }
    }

    if (_obj) {
        return _obj->getOwnPropertyDescriptorByName(ctx, prop, descriptorOut, setterOut);
    }

    return false;
}

bool JsArray::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    descriptorOut = JsProperty(JsUndefinedValue);
    setterOut = JsUndefinedValue;

    auto block = findBlock(index);
    if (!block) {
        return false;
    }

    index -= block->index;
    assert(index < block->items.size());
    descriptorOut = block->items[index];

    if (block->setters && index < block->setters->size()) {
        setterOut = (*block->setters)[index];
    }

    return true;
}

bool JsArray::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorBySymbol(ctx, index, descriptorOut, setterOut);
    }

    return false;
}

JsValue JsArray::getSetterByName(VMContext *ctx, const SizedString &prop) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsArray::getSetterByIndex(VMContext *ctx, uint32_t index) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsArray::getSetterBySymbol(VMContext *ctx, uint32_t index) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsArray::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return getByIndex(ctx, thiz, (uint32_t)n);
        }
    }

    if (prop.equal(SS_LENGTH)) {
        return JsValue(JDT_INT32, _length);
    }

    if (_obj) {
        return _obj->getByName(ctx, thiz, prop);
    }

    return JsUndefinedValue;
}

JsValue JsArray::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    Block *block;
    if (index < ARRAY_BLOCK_SIZE) {
        // 大多数情况都是在第一块内
        block = _firstBlock;
        if (index >= _firstBlock->items.size()) {
            assert(_blocks.size() == 1);
            return JsUndefinedValue;
        }
    } else {
        block = findBlock(index);
        if (!block) {
            return JsUndefinedValue;
        }

        index -= block->index;
    }

    assert(index < block->items.size());
    auto &propValue = block->items[index];
    if (propValue.isGetter) {
        ctx->vm->callMember(ctx, thiz, propValue.value, Arguments());
        auto ret = ctx->stack.back();
        ctx->stack.pop_back();
        return ret;
    }

    return propValue.value;
}

JsValue JsArray::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (_obj) {
        return _obj->getBySymbol(ctx, thiz, index);
    }

    return JsUndefinedValue;
}

void JsArray::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n < ARRAY_MAX_INDEX) {
            setByIndex(ctx, thiz, uint32_t(n), value);
            return;
        }
    }

    if (prop.equal(SS_LENGTH)) {
        auto tmp = ctx->runtime->toNumber(ctx, value);
        auto length = (uint32_t)tmp;
        if (length != tmp) {
            ctx->throwException(PE_RANGE_ERROR, "Invalid array length");
            return;
        }

        if (length > _length) {
            // 扩大 length
            _length = length;
        } else if (length < _length) {
            // 缩小 length
            _length = length;

            auto it = lower_bound(_blocks.begin(), _blocks.end(), length, BlockLessCompare());
            if (it != _blocks.end()) {
                // 分配了空间，先删除 length 所在 block 的
                auto block = *it;
                length -= block->index;
                block->items.resize(length);

                if (block->setters && length > block->setters->size()) {
                    // 删除 setters
                    block->setters->resize(length);
                }

                // 删除剩下的 block
                ++it;
                auto itStart = it;
                for (; it != _blocks.end(); ++it) {
                    auto block = *it;
                    delete block;
                }
                _blocks.erase(itStart, _blocks.end());
            }
        }
        return;
    }

    if (!_obj) {
        _newObject();
    }
    _obj->setByName(ctx, thiz, prop, value);
}

void JsArray::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    Block *block = nullptr;
    if (index < ARRAY_BLOCK_SIZE) {
        // 大多数情况都是在第一块内
        block = _firstBlock;

        if (index >= _firstBlockItems->size()) {
            _firstBlockItems->resize(index + 1, JsNotInitializedValue);
            assert(_blocks.size() <= 1 || index < _blocks[1]->index);

            if (index >= _length) {
                _length = index + 1;
            }

            // 没有 setter，直接修改
            _firstBlockItems->at(index) = value;
            return;
        }
    } else {
        block = findToModifyBlock(index);
        index -= block->index;
    }

    if (block->setters && index < block->setters->size()) {
        // 有 setter 的情况
        auto setter = (*block->setters)[index];
        ArgumentsX args(value);
        ctx->vm->callMember(ctx, thiz, setter, args);
        return;
    }

    block->items[index] = value;
}

void JsArray::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

bool JsArray::removeByName(VMContext *ctx, const SizedString &prop) {
    if (_obj) {
        return _obj->removeByName(ctx, prop);
    }

    return true;
}

bool JsArray::removeByIndex(VMContext *ctx, uint32_t index) {
    if (_obj) {
        _obj->removeByIndex(ctx, index);
    }

    return true;
}

bool JsArray::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        _obj->removeBySymbol(ctx, index);
    }

    return true;
}

IJsObject *JsArray::clone() {
    assert(0 && "NOT supported.");

    return nullptr;
}

void JsArray::push(VMContext *ctx, const JsValue &value) {
    setByIndex(ctx, JsNullValue, _length, value);
}

void JsArray::_newObject() {
    assert(_obj == nullptr);
    _obj = new JsObject();
}

JsArray::Block *JsArray::findBlock(uint32_t index) {
    auto it = lower_bound(_blocks.begin(), _blocks.end(), index, BlockLessCompare());
    if (it != _blocks.end()) {
        auto block = *it;
        if (index >= block->index) {
            return block;
        }
    }

    return nullptr;
}

JsArray::Block *JsArray::findToModifyBlock(uint32_t index) {
    if (index < ARRAY_BLOCK_SIZE) {
        // 在第一个 block 内
        if (index >= _firstBlockItems->size()) {
            _firstBlockItems->resize(index + 1, JsNotInitializedValue);
            assert(_blocks.size() <= 1 || index < _blocks[1]->index);

            if (index >= _length) {
                _length = index + 1;
            }
        }

        return _firstBlock;
    }

    auto it = upper_bound(_blocks.begin(), _blocks.end(), index, BlockLessCompare());
    if (it == _blocks.end()) {
        // 已经超出了最后一个 block: [last_block] index
        auto block = _blocks.back();
        if (index < block->index + ARRAY_BLOCK_SIZE && block->index + block->items.size() - index < ARRAY_BLOCK_SIZE / 2) {
            // 插入到最后一个 block
        } else {
            // 创建一个新的 block
            block = new Block;
            _blocks.push_back(block);
            block->index = roundIndexToBlock(index);
        }

        block->items.resize(index - block->index + 1, JsNotInitializedValue);

        if (index >= _length) {
            _length = index + 1;
        }

        return block;
    }

    Block *block = *it;
    if (index >= block->index) {
        // 在 block 之内: [ block (index) ]
        assert(index < block->index + block->items.size());
        return block;
    }

    //
    // index 在 block 之前
    // [prev]  index [block]
    //
    
    // 一定有 prev block, 因为要么进 if (index < ARRAY_BLOCK_SIZE)，要么在第一个 block 之后
    assert(it != _blocks.begin());
    Block *prev = *(it - 1);

    if (index < ARRAY_RESERVE_MAX_SIZE) {
        // 在 ARRAY_RESERVE_MAX_SIZE 之前的 index 都严格按照 ARRAY_BLOCK_SIZE 分块
        if (index < prev->index + ARRAY_BLOCK_SIZE) {
            // 在前一个 block 中
        } else {
            // 插入新的 block
            prev = new Block;
            _blocks.insert(it, prev);
            prev->index = roundIndexToBlock(index);
            prev->items.push_back(JsProperty(JsNotInitializedValue));
        }
        prev->items.resize(index - prev->index + 1, JsNotInitializedValue);
        return prev;
    }

    block = *it;

    uint32_t prevDistance = index - (prev->index + (uint32_t)prev->items.size());
    uint32_t nextDistance = block->index - index;

    // 前一个满了 或者 后一个满了
    if (prevDistance <= nextDistance && index - prev->index < ARRAY_BLOCK_SIZE
            && index - (prev->index + prev->items.size()) < ARRAY_BLOCK_SIZE / 2) {
        // 距离前一个近，而且前一个还没满，且添加的距离不小于 ARRAY_BLOCK_SIZE / 2
        prev->items.resize(index - prev->index + 1);
        return prev;
    } else if (block->index + block->items.size() - index < ARRAY_BLOCK_SIZE
            && block->index - index < ARRAY_BLOCK_SIZE / 2) {
        // 距离后一个近，而且后一个还没满，且添加的距离不小于 ARRAY_BLOCK_SIZE / 2
        auto insertCount = block->index - index;
        block->items.insert(block->items.begin(), insertCount, JsProperty(JsNotInitializedValue));
        if (block->setters) {
            block->setters->insert(block->setters->begin(), insertCount, JsNotInitializedValue);
        }
        block->index = index;
        return block;
    } else {
        // 在 block 所在的位置插入一个新的 block
        block = new Block;
        _blocks.insert(it, block);
        block->index = index;
        block->items.push_back(JsProperty(JsNotInitializedValue));
        return block;
    }
}

void JsArray::reserveSize(uint32_t length) {
    assert(length <= ARRAY_RESERVE_MAX_SIZE);

    assert(_blocks.empty());

    auto index = 0;
    while (index < length) {
        auto size = min(ARRAY_BLOCK_SIZE, length - index);

        auto b = new Block();
        _blocks.push_back(b);
        b->index = index;
        b->items.resize(size, JsNotInitializedValue);

        index += size;
        length -= size;
    }

    _length = length;
}
