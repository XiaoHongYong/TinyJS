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

/**
 * 遍历 Array
 */
class JsArrayIterator : public IJsIterator {
public:
    JsArrayIterator(VMContext *ctx, JsArray *arr, bool includeProtoProp) : _keyBuf(0) {
        _ctx = ctx;
        _arr = arr;
        _pos = 0;
        _itObj = nullptr;
        _includeProtoProp = includeProtoProp;
    }

    ~JsArrayIterator() {
        if (_itObj) {
            delete _itObj;
        }
    }

    inline uint32_t _len() { return _arr->_length; }

    virtual bool nextOf(JsValue &valueOut) override {
        if (_pos >= _len()) {
            return false;
        }

        valueOut = _arr->getByIndex(_ctx, _arr->self, _pos);
        _pos++;

        return true;
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        JsProperty *prop = nullptr;
        while (true) {
            if (_pos >= _len()) {
                if (_itObj == nullptr) {
                    if (_arr->_obj) {
                        _itObj = _arr->_obj->getIteratorObject(_ctx, _includeProtoProp);
                    } else {
                        if (_includeProtoProp) {
                            // _arr 的 __proto__ 一定没有修改，使用系统的. 如果修改了 __proto__ 就会创建 _obj.
                            // 使用 proto 的 iterator.
                            _itObj = _ctx->runtime->objPrototypeArray->getIteratorObject(_ctx);
                        } else {
                            return false;
                        }
                    }
                }
                return _itObj->next(strKeyOut, keyOut, valueOut);
            }

            prop = _arr->getRawByIndex(_ctx, _pos, false);
            if (prop->isEnumerable) {
                break;
            }

            _pos++;
        }

        if (strKeyOut || keyOut) {
            _keyBuf.set(_pos);
            if (strKeyOut) {
                *strKeyOut = _keyBuf.str();
            }

            if (keyOut) {
                *keyOut = _ctx->runtime->pushString(_keyBuf.str());
            }
        }

        if (valueOut) {
            *valueOut = getPropertyValue(_ctx, prop, _arr->self);
        }

        _pos++;

        return true;
    }

protected:
    VMContext                       *_ctx;
    JsArray                         *_arr;
    uint32_t                        _pos;
    NumberToSizedString             _keyBuf;

    IJsIterator                     *_itObj;
    bool                            _includeProtoProp;

};


JsArray::JsArray(uint32_t length) {
    type = JDT_ARRAY;

    if (length > 0) {
        reserveSize(length);
    } else {
        auto block = new Block();
        _blocks.push_back(block);
        _length = 0;
    }

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

void JsArray::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return definePropertyByIndex(ctx, (uint32_t)n, descriptor);
        }
    }

    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyByName(ctx, name, descriptor);
}

void JsArray::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (isPreventedExtensions && index >= _length) {
        return;
    }

    auto block = findToModifyBlock(index);
    index -= block->index;
    assert(index < block->items.size());

    defineIndexProperty(ctx, &(block->items[index]), descriptor, index);
}

void JsArray::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

void JsArray::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return setByIndex(ctx, thiz, (uint32_t)n, value);
        }
    }

    if (name.equal(SS_LENGTH)) {
        auto tmp = ctx->runtime->toNumber(ctx, value);
        auto length = (uint32_t)tmp;
        if (length != tmp) {
            ctx->throwException(PE_RANGE_ERROR, "Invalid array length");
            return;
        }

        setLength(length);
        return;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setByName(ctx, thiz, name, value);
}

void JsArray::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (isPreventedExtensions && index >= _length) {
        return;
    }

    Block *block = nullptr;
    if (index < ARRAY_BLOCK_SIZE) {
        // 大多数情况都是在第一块内
        block = _firstBlock;

        if (index >= _firstBlockItems->size()) {
            _firstBlockItems->resize(index + 1, jsValueNotInitialized);
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

    auto prop = &block->items[index];
    set(ctx, prop, thiz, value);
}

void JsArray::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsArray::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return increaseByIndex(ctx, thiz, (uint32_t)index, n, isPost);
        }
    }

    if (name.equal(SS_LENGTH)) {
        setLength(_length + 1);
        return JsValue(JDT_INT32, isPost ? _length - 1 : _length);
    }

    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsArray::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (isPreventedExtensions && index >= _length) {
        return jsValueNaN;
    }

    Block *block = nullptr;
    if (index < ARRAY_BLOCK_SIZE) {
        // 大多数情况都是在第一块内
        block = _firstBlock;

        if (index >= _firstBlockItems->size()) {
            _firstBlockItems->resize(index + 1, jsValueNotInitialized);
            assert(_blocks.size() <= 1 || index < _blocks[1]->index);

            if (index >= _length) {
                _length = index + 1;
            }

            // 没有 setter，直接修改
            _firstBlockItems->at(index) = jsValueNaN;
            return jsValueNaN;
        }
    } else {
        block = findToModifyBlock(index);
        index -= block->index;
    }

    auto prop = &block->items[index];
    return increase(ctx, prop, thiz, n, isPost);
}

JsValue JsArray::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsProperty *JsArray::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    funcGetterOut = nullptr;
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return getRawByIndex(ctx, (uint32_t)n, includeProtoProp);
        }
    }

    if (name.equal(SS_LENGTH)) {
        static JsProperty prop;
        prop = JsProperty(JsValue(JDT_INT32, _length), false, false, false);
        return &prop;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        static JsProperty prop;
        prop = JsProperty(jsValuePrototypeArray, false, false, false, true);
        return &prop;
    }

    if (includeProtoProp) {
        return ctx->runtime->objPrototypeArray->getRawByName(ctx, name, funcGetterOut, true);
    }
    return nullptr;
}

JsProperty *JsArray::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    Block *block;
    if (index < ARRAY_BLOCK_SIZE) {
        // 大多数情况都是在第一块内
        block = _firstBlock;
        if (index >= _firstBlock->items.size()) {
            assert(_blocks.size() == 1);
            return nullptr;
        }
    } else {
        block = findBlock(index);
        if (!block) {
            return nullptr;
        }

        index -= block->index;
    }

    assert(index < block->items.size());
    return &block->items[index];
}

JsProperty *JsArray::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsArray::removeByName(VMContext *ctx, const SizedString &name) {
    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsArray::removeByIndex(VMContext *ctx, uint32_t index) {
    auto block = findBlock(index);
    if (!block) {
        if (_obj) {
            return _obj->removeByIndex(ctx, index);
        }

        return true;
    }
    index -= block->index;

    assert(index < block->items.size());
    auto &prop = block->items[index];

    if (prop.isConfigurable) {
        prop = JsProperty(jsValueNotInitialized);
    }

    return true;
}

bool JsArray::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void JsArray::changeAllProperties(VMContext *ctx, int8_t configurable, int8_t writable) {
    for (auto block : _blocks) {
        for (auto &item : block->items) {
            item.changeProperty(configurable, writable);
        }
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, configurable, writable);
    }
}

bool JsArray::hasAnyProperty(VMContext *ctx, bool configurable, bool writable) {
    for (auto block : _blocks) {
        for (auto &item : block->items) {
            if (item.isPropertyAny(configurable, writable)) {
                return true;
            }
        }
    }

    return _obj && _obj->hasAnyProperty(ctx, configurable, writable);
}

void JsArray::preventExtensions(VMContext *ctx) {
    IJsObject::preventExtensions(ctx);

    if (_obj) {
        _obj->preventExtensions(ctx);
    }
}

IJsObject *JsArray::clone() {
    assert(0 && "NOT supported.");

    return nullptr;
}

IJsIterator *JsArray::getIteratorObject(VMContext *ctx, bool includeProtoProp) {
    return new JsArrayIterator(ctx, this, includeProtoProp);
}

void JsArray::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    for (auto block : _blocks) {
        for (auto &prop : block->items) {
            rt->markReferIdx(prop);
        }
    }

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

void JsArray::push(VMContext *ctx, const JsValue &value) {
    setByIndex(ctx, jsValueNull, _length, value);
}

void JsArray::push(VMContext *ctx, const JsValue *first, uint32_t count) {
    for (int i = 0; i < count; i++) {
        setByIndex(ctx, jsValueNull, _length, first[i]);
    }
}

static set<uint32_t> toStringCallStack;

void JsArray::toString(VMContext *ctx, const JsValue &thiz, BinaryOutputStream &stream) {
    uint32_t lastIdx = 0;
    string buf;
    bool empty = true;

    auto it = toStringCallStack.find(thiz.value.index);
    if (it != toStringCallStack.end()) {
        return;
    }
    toStringCallStack.insert(thiz.value.index);

    for (auto b : _blocks) {
        for (auto i = lastIdx; i < b->index; i++) {
            if (empty) {
                empty = false;
            } else {
                stream.writeUInt8(',');
            }
        }

        for (auto &item : b->items) {
            auto v = item.value;
            if (item.isGSetter && item.value.type >= JDT_FUNCTION) {
                ctx->vm->callMember(ctx, thiz, item.value, Arguments());
                v = ctx->retValue;
            }

            SizedString s;
            if (v.type == JDT_ARRAY) {
                auto arr = (JsArray *)ctx->runtime->getObject(v);
                arr->toString(ctx, v, stream);
            } else {
                s = ctx->runtime->toSizedString(ctx, v, buf);
            }

            if (empty) {
                empty = false;
            } else {
                stream.writeUInt8(',');
            }
            stream.write(s.data, s.len);
        }
    }

    toStringCallStack.erase(thiz.value.index);
}

void JsArray::setLength(uint32_t length) {
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
}

void JsArray::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);
    _obj = new JsObject(jsValuePrototypeArray);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
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
            _firstBlockItems->resize(index + 1, jsValueNotInitialized);
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

        block->items.resize(index - block->index + 1, jsValueNotInitialized);

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
            prev->items.push_back(JsProperty(jsValueNotInitialized));
        }
        prev->items.resize(index - prev->index + 1, jsValueNotInitialized);
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
        block->items.insert(block->items.begin(), insertCount, JsProperty(jsValueNotInitialized));
        block->index = index;
        return block;
    } else {
        // 在 block 所在的位置插入一个新的 block
        block = new Block;
        _blocks.insert(it, block);
        block->index = index;
        block->items.push_back(JsProperty(jsValueNotInitialized));
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
        b->items.resize(size, jsValueNotInitialized);

        index += size;
        length -= size;
    }

    _length = length;
}
