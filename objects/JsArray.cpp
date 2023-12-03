//
//  JsArray.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "JsArray.hpp"
#include "interpreter/BinaryOperation.hpp"


const uint32_t ARRAY_BLOCK_SIZE = 1024 * 32; // 32768
const uint32_t ARRAY_RESERVE_MAX_SIZE = ARRAY_BLOCK_SIZE * 10; // 327680 个
const uint32_t ARRAY_MAX_INDEX = 4294967294; // 2 ** 32 - 2

const JsValue jsPropertyNotInitialized = jsValueEmpty.asProperty(JP_CONFIGURABLE | JP_WRITABLE | JP_EMPTY);

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
    JsArrayIterator(VMContext *ctx, JsArray *arr, bool includeProtoProp, bool includeNoneEnumerable) :  IJsIterator(includeProtoProp, includeNoneEnumerable), _keyBuf(0)
    {
        _isOfIterable = true;
        _ctx = ctx;
        _arr = arr;
        _pos = 0;
        _itObj = nullptr;
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

    virtual bool next(StringView *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        JsValue *prop = nullptr;
        while (true) {
            if (_pos >= _len()) {
                if (_pos == _len() && _includeNoneEnumerable) {
                    // length
                    _pos++;
                    if (strKeyOut) { *strKeyOut = SS_LENGTH; }
                    if (keyOut) { *keyOut = jsStringValueLength; }
                    if (valueOut) { *valueOut = makeJsValueInt32(_len()); }
                    return true;
                }

                if (_itObj == nullptr) {
                    if (_arr->_obj) {
                        _itObj = _arr->_obj->getIteratorObject(_ctx, _includeProtoProp, _includeNoneEnumerable);
                    } else {
                        if (_includeProtoProp) {
                            // _arr 的 __proto__ 一定没有修改，使用系统的. 如果修改了 __proto__ 就会创建 _obj.
                            // 使用 proto 的 iterator.
                            _itObj = _ctx->runtime->objPrototypeArray()->getIteratorObject(_ctx, _includeProtoProp, _includeNoneEnumerable);
                        } else {
                            return false;
                        }
                    }
                }
                return _itObj->next(strKeyOut, keyOut, valueOut);
            }

            prop = _arr->getRawByIndex(_ctx, _pos, false);
            if (prop == nullptr) {
                break;
            } else if (prop->isEnumerable() || _includeNoneEnumerable) {
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
            *valueOut = prop ? getPropertyValue(_ctx, _arr->self, prop) : jsValueUndefined;
        }

        _pos++;

        return true;
    }

protected:
    VMContext                       *_ctx;
    JsArray                         *_arr;
    uint32_t                        _pos;
    NumberToStringView             _keyBuf;

    IJsIterator                     *_itObj;

};


JsArray::JsArray(uint32_t length) : IJsObject(jsValuePrototypeArray, JDT_ARRAY) {
    _isOfIterable = true;

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

void JsArray::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return setPropertyByIndex(ctx, (uint32_t)n, descriptor);
        }
    }

    if (name.equal(SS_LENGTH)) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot redefine property: length");
        return;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyByName(ctx, name, descriptor);
}

void JsArray::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (isPreventedExtensions && index >= _length) {
        return;
    }

    auto block = findToModifyBlock(index);
    index -= block->index;
    assert(index < block->items.size());
    block->hasPropDescriptor = true;
    block->items[index] = descriptor;
}

void JsArray::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError JsArray::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
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
            ctx->throwException(JE_RANGE_ERROR, "Invalid array length");
            return JE_TYPE_INVALID_LENGTH;
        }

        setLength(length);
        return JE_OK;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->setByName(ctx, thiz, name, value);
}

JsError JsArray::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (isPreventedExtensions && index >= _length) {
        return JE_TYPE_PREVENTED_EXTENSION;
    }

    Block *block = nullptr;
    if (index < ARRAY_BLOCK_SIZE) {
        // 大多数情况都是在第一块内
        block = _firstBlock;

        if (index >= _firstBlockItems->size()) {
            _firstBlockItems->resize(index + 1, jsPropertyNotInitialized);
            assert(_blocks.size() <= 1 || index < _blocks[1]->index);

            if (index >= _length) {
                _length = index + 1;
            }

            // 没有 setter 等属性限制，直接修改
            _firstBlockItems->at(index) = value.asProperty();
            return JE_OK;
        }
    } else {
        block = findToModifyBlock(index);
        index -= block->index;
    }

    auto prop = &block->items[index];
    return setPropertyValue(ctx, prop, thiz, value);
}

JsError JsArray::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsArray::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return increaseByIndex(ctx, thiz, (uint32_t)index, n, isPost);
        }
    }

    if (name.equal(SS_LENGTH)) {
        setLength(_length + n);
        return makeJsValueInt32(isPost ? _length - n : _length);
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
            _firstBlockItems->resize(index + 1, jsPropertyNotInitialized);
            assert(_blocks.size() <= 1 || index < _blocks[1]->index);

            if (index >= _length) {
                _length = index + 1;
            }

            // 没有 setter 等属性限制，直接修改
            _firstBlockItems->at(index) = jsValueNaN.asProperty();
            return jsValueNaN;
        }
    } else {
        block = findToModifyBlock(index);
        index -= block->index;
    }

    auto prop = &block->items[index];
    return increasePropertyValue(ctx, prop, thiz, n, isPost);
}

JsValue JsArray::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsValue *JsArray::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n <= ARRAY_MAX_INDEX) {
            return getRawByIndex(ctx, (uint32_t)n, includeProtoProp);
        }
    }

    if (name.equal(SS_LENGTH)) {
        static JsValue prop;
        prop = makeJsValueInt32(_length).asProperty(JP_WRITABLE);
        return &prop;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        return ctx->runtime->objPrototypeArray()->getRawByName(ctx, name, true);
    }
    return nullptr;
}

JsValue *JsArray::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
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

JsValue *JsArray::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsArray::removeByName(VMContext *ctx, const StringView &name) {
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

    if (prop.isConfigurable()) {
        prop = jsPropertyNotInitialized;
        return true;
    } else {
        return false;
    }
}

bool JsArray::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void JsArray::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    for (auto block : _blocks) {
        for (auto &item : block->items) {
            item.changeProperty(toAdd, toRemove);
        }
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, toAdd, toRemove);
    }
}

bool JsArray::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    for (auto block : _blocks) {
        for (auto &item : block->items) {
            if (item.isPropertyAny(flags)) {
                return true;
            }
        }
    }

    return _obj && _obj->hasAnyProperty(ctx, flags);
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

IJsIterator *JsArray::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    return new JsArrayIterator(ctx, this, includeProtoProp, includeNoneEnumerable);
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

std::pair<JsError, int> JsArray::popFront(VMContext *ctx, Block *b) {
    auto &items = b->items;
    uint32_t count = (uint32_t)items.size();
    if (count == 0) {
        return { JE_OK, 0 };
    }

    if (!b->hasPropDescriptor) {
        // 直接去掉第一个即可
        items.pop_front();
        return { JE_OK, 0 };
    }

    // 有 property descriptor，只能逐个移动
    JsError err = JE_OK;
    int i = 1;
    for (; i < count && err == JE_OK; i++) {
        auto &prop = items[i];
        err = JE_OK;

        JsValue v = getPropertyValue(ctx, self, prop, jsValueEmpty);
        if (v.isEmpty()) {
            // 删除前一个
            auto &prop = items[i - 1];
            if (prop.isConfigurable()) {
                prop = jsPropertyNotInitialized;
            } else {
                err = JE_TYPE_PROP_NO_DELETABLE;
            }
        } else {
            err = setPropertyValue(ctx, &items[i - 1], self, v);
        }
    }

    items.pop_back();
    return { err, i - 2 };
}

JsValue JsArray::front(VMContext *ctx, Block *b) {
    auto &items = b->items;
    if (!items.empty()) {
        auto &prop = items[0];
        return getPropertyValue(ctx, self, prop);
    }

    return jsValueUndefined;
}

std::tuple<JsValue, JsError, int> JsArray::popFront(VMContext *ctx) {
    if (_length == 0) {
        return { jsValueUndefined, JE_OK, 0 };
    }

    auto retValue = front(ctx, _firstBlock);
    if (retValue.isEmpty()) {
        retValue = jsValueUndefined;
    }

    for (int i = 0; i < _blocks.size(); i++) {
        auto b = _blocks[i];
        if (i > 0) {
            auto v = front(ctx, b);
            auto err = setByIndex(ctx, self, b->index - 1, v);
            if (err != JE_OK) {
                return { retValue, err, b->index - 1 };
            }

            if (b != _blocks[i]) {
                // 因为 setByIndex 插入了新的 block.
                i++;
                b = _blocks[i];
            }
        }

        auto ret = popFront(ctx, b);
        if (ret.first != JE_OK) {
            return { retValue, ret.first, ret.second };
        }
    }

    setLength(_length - 1);
    return { retValue, JE_OK, 0 };
}

void JsArray::sort(VMContext *ctx, const JsValue &callback) {
    VecJsValues values;
    int countEmpty = 0, countUndefined = 0;

    for (auto b : _blocks) {
        for (auto &item : b->items) {
            auto v = getPropertyValue(ctx, self, item, jsValueEmpty);
            if (v.isEmpty()) {
                countEmpty++;
            } else if (v.type == JDT_UNDEFINED) {
                countUndefined++;
            } else {
                values.push_back(v);
            }
        }
    }

    auto vm = ctx->vm;

    if (callback.isFunction()) {
        stable_sort(values.begin(), values.end(), [vm, ctx, callback](const JsValue &a, const JsValue &b) {
            ArgumentsX args(a, b);
            vm->callMember(ctx, jsValueGlobalThis, callback, args);
            return ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue);
        });
    } else {
        stable_sort(values.begin(), values.end(), [vm, ctx, callback](const JsValue &a, const JsValue &b) {
            return ctx->error == JE_OK && convertToStringLessCmp(ctx, a, b);
        });
    }

    // 排好序的属性
    int i = 0;
    while (i < (int)values.size() && ctx->error == JE_OK) {
        auto b = findToModifyBlock(i);
        assert(b->index == i);
        int end = b->index + ARRAY_BLOCK_SIZE;
        if (end > (int)values.size()) {
            end = (int)values.size();
        }

        auto &items = b->items;
        int index = i - b->index;
        if (items.size() < end - i) {
            // 增加空间
            items.resize(end - i, jsPropertyNotInitialized);
        }

        for (; i < end; i++, index++) {
            auto &prop = items[index];
            auto value = values[i];
            auto err = setPropertyValue(ctx, &prop, self, value);
            if (err == JE_TYPE_NO_PROP_SETTER) {
                ctx->throwException(JE_TYPE_ERROR,
                    "Cannot set property %d of [object Array] which has only a getter", index);
            } else if (err == JE_TYPE_PROP_READ_ONLY) {
                ctx->throwException(JE_TYPE_ERROR,
                        "Cannot assign to read only property '%d' of object '[object Array]'", index);
            }
        }
    }

    // undefined
    countUndefined += i;
    while (i < countUndefined) {
        auto b = findToModifyBlock(i);
        int end = b->index + ARRAY_BLOCK_SIZE;
        if (end > countUndefined) {
            end = countUndefined;
        }

        auto &items = b->items;
        int index = i - b->index;
        if (items.size() < end - b->index) {
            // 增加空间
            items.resize(end - b->index, jsPropertyNotInitialized);
        }

        for (; i < end; i++, index++) {
            auto &prop = items[index];
            auto err = setPropertyValue(ctx, &prop, self, jsValueUndefined);
            if (err == JE_TYPE_NO_PROP_SETTER) {
                ctx->throwException(JE_TYPE_ERROR,
                    "Cannot set property %d of [object Array] which has only a getter", index);
            } else if (err == JE_TYPE_PROP_READ_ONLY) {
                ctx->throwException(JE_TYPE_ERROR,
                        "Cannot assign to read only property '%d' of object '[object Array]'", index);
            }
        }
    }

    // empty
    countEmpty += i;
    while (i < countEmpty && ctx->error == JE_OK) {
        auto b = findBlock(i);
        auto &items = b->items;
        int end = b->index + (int)items.size();
        int index = i - b->index;
        int start = index;
        for (; i < end; i++, index++) {
            auto &prop = items[index];
            if (prop.isConfigurable()) {
                prop = jsPropertyNotInitialized;
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '%d' of [object Array]", i);
                break;
            }
        }

        if (ctx->error == JE_OK) {
            // 从 index 之后的元素都是 jsValueEmpty，可以直接删除
            items.resize(start, jsPropertyNotInitialized);
        }
    }
}

void JsArray::pushEmpty() {
    findToModifyBlock(_length);
}

JsError JsArray::push(VMContext *ctx, const JsValue &value) {
    return setByIndex(ctx, self, _length, value);
}

JsError JsArray::push(VMContext *ctx, const JsValue *first, uint32_t count) {
    if (isPreventedExtensions) {
        return JE_TYPE_PREVENTED_EXTENSION;
    }

    while (count > 0) {
        auto b = findToModifyBlock(_length); // 这里会将 _length + 1
        uint32_t bound = roundIndexToBlock(b->index + ARRAY_BLOCK_SIZE);

        uint32_t n = _length + count;
        n = n < bound ? count : bound - b->index;

        _firstBlock->items[_length - 1 - b->index] = first[0].asProperty(); // 第一个元素直接修改
        for (int i = 1; i < n; i++) {
            _firstBlock->items.push_back(first[i].asProperty());
        }

        first += n;
        count -= n;
        _length += n - 1;
    }

    return JE_OK;
}

JsError JsArray::extend(VMContext *ctx, const JsArray *other) {
    if (isPreventedExtensions) {
        return JE_TYPE_PREVENTED_EXTENSION;
    }

    for (auto b : other->_blocks) {
        VecJsValues values;
        values.reserve(b->items.size());

        for (auto &prop : b->items) {
            values.push_back(getPropertyValue(ctx, self, prop, jsValueEmpty));
        }

        push(ctx, values.data(), (uint32_t)values.size());
    }

    return JE_OK;
}

JsError JsArray::extend(VMContext *ctx, const JsValue &other, uint32_t depth) {
    if (isPreventedExtensions) {
        return JE_TYPE_PREVENTED_EXTENSION;
    }

    assert(other.type == JDT_ARRAY);

    VecJsValues values;

    // 采用栈来保存递归调用，避免堆栈溢出
    struct Status {
        VecBlocks::iterator         itBlock, itBlockEnd;
        DequeJsProperties::iterator itItems, itItemsEnd;
    };
    vector<Status> stack;

    auto otherArr = (JsArray *)ctx->runtime->getObject(other);

    VecBlocks::iterator itBlock = otherArr->_blocks.begin(), itBlockEnd = otherArr->_blocks.end();
    DequeJsProperties::iterator itItems, itItemsEnd;
    if (itBlock != itBlockEnd) {
        itItems = (*itBlock)->items.begin();
        itItemsEnd = (*itBlock)->items.end();
    }

    while (1) {
        while (1) {
            while (itItems != itItemsEnd) {
                auto &prop = *itItems;
                itItems++;

                JsValue value = getPropertyValue(ctx, self, prop, jsValueUndefined);

                if (depth > 0 && value.type == JDT_ARRAY) {
                    depth--;
                    stack.push_back({itBlock, itBlockEnd, itItems, itItemsEnd});

                    auto otherArr = (JsArray *)ctx->runtime->getObject(value);
                    if (!otherArr->_blocks.empty()) {
                        itBlock = otherArr->_blocks.begin(); itBlockEnd = otherArr->_blocks.end();
                        itItems = (*itBlock)->items.begin(); itItemsEnd = (*itBlock)->items.end();
                    }
                } else {
                    values.push_back(value);
                    if (values.size() > 0x1000000) {
                        ctx->throwException(JE_RANGE_ERROR, "Maximum call stack size exceeded");
                        return JE_MAX_STACK_EXCEEDED;
                    }
                }
            }

            itBlock++;
            if (itBlock == itBlockEnd) {
                break;
            }

            itItems = (*itBlock)->items.begin();
            itItemsEnd = (*itBlock)->items.end();
        }

        if (stack.empty()) {
            break;
        }

        auto &status = stack.back();
        itBlock = status.itBlock;
        itBlockEnd = status.itBlockEnd;
        itItems = status.itItems;
        itItemsEnd = status.itItemsEnd;
        stack.pop_back();
    }

    return push(ctx, values.data(), (uint32_t)values.size());
}

void JsArray::reverse(VMContext *ctx) {
    if (_length < ARRAY_BLOCK_SIZE)  {
        // 都在第一个 block 内
        auto &items = *_firstBlockItems;
        int32_t high = (int32_t)items.size();
        if (high == 0) {
            return;
        }
        high--;

        for (int low = 0; low < high; low++, high--) {
            auto &left = items[low];
            auto &right = items[high];
            JsValue vl = getPropertyValue(ctx, self, left, jsValueEmpty);
            JsValue vr = getPropertyValue(ctx, self, right, jsValueEmpty);

            setPropertyValue(ctx, &left, self, vr);
            setPropertyValue(ctx, &right, self, vl);
        }
    } else {
        uint32_t high = _length;
        if (high == 0) {
            return;
        }
        high--;

        for (int low = 0; low < high; low++, high--) {
            auto vl = getByIndex(ctx, self, low, jsValueEmpty);
            auto vr = getByIndex(ctx, self, high, jsValueEmpty);

            if (vl.isEmpty() && vr.isEmpty()) {
                continue;
            }

            setByIndex(ctx, self, low, vr);
            setByIndex(ctx, self, high, vl);
        }
    }
}

static set<uint32_t> toStringCallStack;

void JsArray::toString(VMContext *ctx, const JsValue &thiz, BinaryOutputStream &stream) {
    uint32_t lastIdx = 0;
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
            auto v = getPropertyValue(ctx, thiz, item, jsValueEmpty);

            LockedStringViewWrapper s;
            if (v.type == JDT_ARRAY) {
                auto arr = (JsArray *)ctx->runtime->getObject(v);
                arr->toString(ctx, v, stream);
            } else {
                s = ctx->runtime->toStringView(ctx, v);
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

JsArray *JsArray::cloneArrayOnly() {
    auto other = new JsArray();
    other->_length = _length;

    for (auto b : _blocks) {
        Block *t;
        if (b == _firstBlock) {
            t = other->_firstBlock;
        } else {
            t = new Block;
            other->_blocks.push_back(t);
        }

        t->index = b->index;
        t->items = b->items;
    }

    return other;
}

void JsArray::dump(VMContext *ctx, const JsValue &thiz, VecJsValues &values) {
    for (auto b : _blocks) {
        values.resize(b->index + b->items.size(), jsValueUndefined);

        int i = b->index;
        for (auto &prop : b->items) {
            values[i] = getPropertyValue(ctx, thiz, prop, jsValueEmpty);
            i++;
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
            _firstBlockItems->resize(index + 1, jsPropertyNotInitialized);
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

        block->items.resize(index - block->index + 1, jsPropertyNotInitialized);

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
        }
        prev->items.resize(index - prev->index + 1, jsPropertyNotInitialized);
        return prev;
    }

    block = *it;

    uint32_t prevDistance = index - (prev->index + (uint32_t)prev->items.size());
    uint32_t nextDistance = block->index - index;

    // 前一个满了 或者 后一个满了
    if (prevDistance <= nextDistance && index - prev->index < ARRAY_BLOCK_SIZE
            && index - (prev->index + prev->items.size()) < ARRAY_BLOCK_SIZE / 2) {
        // 距离前一个近，而且前一个还没满，且添加的距离不小于 ARRAY_BLOCK_SIZE / 2
        prev->items.resize(index - prev->index + 1, jsPropertyNotInitialized);
        return prev;
    } else if (block->index + block->items.size() - index < ARRAY_BLOCK_SIZE
            && block->index - index < ARRAY_BLOCK_SIZE / 2) {
        // 距离后一个近，而且后一个还没满，且添加的距离不小于 ARRAY_BLOCK_SIZE / 2
        auto insertCount = block->index - index;
        block->items.insert(block->items.begin(), insertCount, jsPropertyNotInitialized);
        block->index = index;
        return block;
    } else {
        // 在 block 所在的位置插入一个新的 block
        block = new Block;
        _blocks.insert(it, block);
        block->index = index;
        block->items.push_back(jsPropertyNotInitialized);
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
        b->items.resize(size, jsPropertyNotInitialized);

        index += size;
        length -= size;
    }

    _length = length;
}
