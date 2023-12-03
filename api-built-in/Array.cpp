//
//  Array.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "BuiltIn.hpp"
#include "objects/JsArray.hpp"
#include "objects/JsArguments.hpp"
#include "objects/JsPrimaryObject.hpp"
#include "strings/JsString.hpp"
#include "interpreter/BinaryOperation.hpp"


void normalizeStart(int &start, int length) {
    if (start < 0) {
        start += length;
        if (start < 0) {
            start = 0;
        }
    }
}

void normalizeEnd(int &end, int length) {
    if (end < 0) {
        end += length;
        if (end < 0) {
            end = 0;
        }
    }

    if (end > length) {
        end = length;
    }
}

StringView objectPrototypeToStringView(const JsValue &thiz);

void throwModifyPropertyException(VMContext *ctx, JsError err, JsValue thiz, int index) {
    if (err == JE_TYPE_PROP_READ_ONLY) {
        auto s = objectPrototypeToStringView(thiz);
        ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '%d' of object '%.*s'", index, s.len, s.data);
    } else if (err == JE_TYPE_NO_PROP_SETTER) {
        auto s = objectPrototypeToStringView(thiz);
        ctx->throwException(JE_TYPE_ERROR, "Cannot set property %d of %.*s which has only a getter", index, s.len, s.data);
    } else if (err == JE_MAX_STACK_EXCEEDED) {
        ctx->throwException(JE_TYPE_ERROR, "Maximum call stack size exceeded");
    } else if (err == JE_TYPE_PREVENTED_EXTENSION) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot add property 0, object is not extensible");
    } else if (err == JE_TYPE_PROP_NO_DELETABLE) {
        auto s = objectPrototypeToStringView(thiz);
        ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '%d' of %.*s", index, s.len, s.data);
    } else {
        assert(0);
    }
}

void setPropByIndexEx(VMContext *ctx, JsValue thiz, IJsObject *obj, JsValue value, int32_t index) {
    if (value.isEmpty()) {
        if (!obj->removeByIndex(ctx, index)) {
            auto s = objectPrototypeToStringView(thiz);
            ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '%d' of %.*s", index, s.len, s.data);
        }
    } else {
        auto err = obj->setByIndex(ctx, thiz, index, value);
        if (err != JE_OK) {
            throwModifyPropertyException(ctx, err, thiz, index);
        }
    }
}

template<typename T>
void arrayLikeAction(VMContext *ctx, JsValue thiz, T action) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            action(0, thiz);
        } else if (thiz.type == JDT_STRING) {
            auto &str = runtime->getStringWithRandAccess(thiz);
            auto length = str.size();
            for (uint32_t i = 0; i < length && ctx->error == JE_OK; i++) {
                action(i, makeJsValueChar(str.chartAt(i)));
            }
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    for (int i = 0; i < length && ctx->error == JE_OK; i++) {
        auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
        if (value.isValid()) {
            action(i, value);
        }
    }
}

template<bool ignoreEmptySlot=true, typename T>
void arrayLikeActionEx(VMContext *ctx, JsValue thiz, T action, int start = 0) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            if (start <= 0) {
                action(0, thiz);
            }
        } else if (thiz.type == JDT_STRING) {
            auto &str = runtime->getStringWithRandAccess(thiz);
            auto length = str.size();
            normalizeStart(start, length);

            for (uint32_t i = start; i < length; i++) {
                if (action(i, makeJsValueChar(str.chartAt(i)))) {
                    break;
                }
            }
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    normalizeStart(start, length);

    if (ignoreEmptySlot) {
        for (int i = start; i < length; i++) {
            auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            if (value.isValid()) {
                if (action(i, value)) {
                    break;
                }
            }
        }
    } else {
        for (int i = start; i < length; i++) {
            auto value = obj->getByIndex(ctx, thiz, i, jsValueUndefined);
            if (action(i, value)) {
                break;
            }
        }
    }
}

template<bool ignoreEmptySlot=true, typename T>
void arrayLikeActionExReverse(VMContext *ctx, JsValue thiz, T action, int fromIndex = -1) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            action(0, thiz);
        } else if (thiz.type == JDT_STRING) {
            auto &str = runtime->getStringWithRandAccess(thiz);
            auto length = str.size();
            if (fromIndex < 0) {
                fromIndex += length;
                if (fromIndex < 0) {
                    fromIndex = 0;
                }
            } else if (fromIndex > length) {
                fromIndex = length - 1;
            }

            for (int i = fromIndex; i >= 0; i--) {
                if (action(i, makeJsValueChar(str.chartAt(i)))) {
                    break;
                }
            }
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    if (fromIndex < 0) {
        fromIndex += length;
        if (fromIndex < 0) {
            fromIndex = 0;
        }
    } else if (fromIndex > length) {
        fromIndex = length - 1;
    }

    if (ignoreEmptySlot) {
        for (int i = fromIndex; i >= 0; i--) {
            auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            if (value.isValid()) {
                if (action(i, value)) {
                    break;
                }
            }
        }
    } else {
        for (int i = fromIndex; i >= 0; i--) {
            auto value = obj->getByIndex(ctx, thiz, i, jsValueUndefined);
            if (action(i, value)) {
                break;
            }
        }
    }
}

static void arrayConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        ctx->retValue = runtime->pushObject(new JsArray());
        return;
    } else if (args.count == 1 && (args[0].type == JDT_INT32 || args[0].type == JDT_NUMBER)) {
        auto len = args[0];
        auto n = 0;
        if (len.type == JDT_NUMBER) {
            auto v = runtime->getDouble(len);
            if (v != (uint32_t)v) {
                ctx->throwException(JE_RANGE_ERROR, "Invalid array length");
                ctx->retValue = jsValueUndefined;
                return;
            }
            n = (uint32_t)v;
        }

        auto arr = new JsArray(n);
        ctx->retValue = runtime->pushObject(arr);
        return;
    }

    auto arr = new JsArray();
    arr->push(ctx, args.data, args.count);
    ctx->retValue = runtime->pushObject(arr);
}

void arrayIsArray(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto arr = args.getAt(0);

    if (arr == jsValuePrototypeArray) {
        ctx->retValue = jsValueTrue;
    } else {
        ctx->retValue = makeJsValueBool(arr.type == JDT_ARRAY);
    }
}

void arrayFrom(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto src = args.getAt(0);
    auto callback = args.getAt(1);
    auto thisArg = args.getAt(2, jsValueGlobalThis);

    if (callback.type <= JDT_UNDEFINED) {
        auto arrObj = new JsArray();
        auto arr = ctx->runtime->pushObject(arrObj);

        auto vm = ctx->vm;
        arrayLikeAction(ctx, src, [ctx, vm, arrObj](int index, JsValue item) {
            arrObj->push(ctx, item);
        });

        ctx->retValue = arr;
    } else if (!callback.isFunction()) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "%.*s is not a function", callback);
        return;
    } else {
        auto arrObj = new JsArray();
        auto arr = ctx->runtime->pushObject(arrObj);

        auto vm = ctx->vm;
        arrayLikeActionEx<false>(ctx, src, [ctx, vm, arrObj, callback, thisArg](int index, JsValue item) {
            ArgumentsX args(item, makeJsValueInt32(index));
            vm->callMember(ctx, thisArg, callback, args);
            arrObj->push(ctx, ctx->retValue);
            return false;
        });

        ctx->retValue = arr;
    }
}

void arrayOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto arrObj = new JsArray();
    auto arr = ctx->runtime->pushObject(arrObj);

    arrObj->push(ctx, args.data, args.count);

    ctx->retValue = arr;
}

static JsLibProperty arrayFunctions[] = {
    { "name", nullptr, "Array" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "isArray", arrayIsArray },
    { "from", arrayFrom },
    { "of", arrayOf },
    { "prototype", nullptr, nullptr, jsValuePropertyWritable },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args);

void arrayPrototypeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto index = args.getIntAt(ctx, 0, 0);
    int32_t length = 0;
    IJsObject *obj = nullptr;

    if (thiz.type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    ctx->retValue = jsValueUndefined;

    switch (thiz.type) {
        case JDT_CHAR:
            if (index == -1 || index == 0) {
                ctx->retValue = thiz;
            }
            return;
        case JDT_STRING: {
            auto &str = ctx->runtime->getStringWithRandAccess(thiz);
            if (index < 0) {
                // 负数倒着访问
                index += str.size();
            }

            if (index >= 0 && index < str.size()) {
                ctx->retValue = makeJsValueChar(str.chartAt(index));
            }
            return;
        }
        case JDT_ARRAY: {
            obj = runtime->getObject(thiz);
            auto jsthiz = (JsArray *)obj;
            length = jsthiz->length();
            break;
        }
        case JDT_ARGUMENTS: {
            obj = runtime->getObject(thiz);
            auto jsthiz = (JsArguments *)obj;
            length = jsthiz->length();
            break;
        }
        default: {
            if (thiz.type >= JDT_OBJECT) {
                obj = runtime->getObject(thiz);
                auto n = runtime->toNumber(ctx, obj->getByName(ctx, thiz, SS_LENGTH));
                if (!isnan(n)) {
                    // Array like.
                    length = (int32_t)n;
                }
            }
        }
    }

    if (index < 0) {
        // 负数倒着访问
        index += length;
    }

    if (index >= 0 && index < length) {
        ctx->retValue = obj->getByIndex(ctx, thiz, index);
    }
}

#define validateThiz(apiName) if (thiz.type <= JDT_NULL) { ctx->throwException(JE_TYPE_ERROR, "Array.prototype." apiName " called on null or undefined"); return; }

JsValue newPrimaryObject(VMRuntime *runtime, const JsValue &thiz) {
    IJsObject *obj = nullptr;
    switch (thiz.type) {
        case JDT_INT32:
        case JDT_NUMBER: obj = new JsNumberObject(thiz); break;
        case JDT_BOOL: obj = new JsBooleanObject(thiz); break;
        case JDT_SYMBOL: obj = new JsSymbolObject(thiz); break;
        case JDT_CHAR:
        case JDT_STRING: obj = new JsStringObject(thiz); break;
        default:
            assert(0);
            break;
    }

    return runtime->pushObject(obj);
}

void arrayPrototypeConcat(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    validateThiz("concat");

    auto runtime = ctx->runtime;

    JsArray *arrObj = nullptr;
    JsValue arr;
    if (thiz.type == JDT_ARRAY) {
        arrObj = ((JsArray *)runtime->getObject(thiz))->cloneArrayOnly();
    } else {
        JsValue a1 = thiz;
        if (thiz.type < JDT_OBJECT) {
            a1 = newPrimaryObject(runtime, thiz);
        }

        arrObj = new JsArray();
        arrObj->push(ctx, a1);
    }

    arr = runtime->pushObject(arrObj);

    for (int i = 0; i < args.count; i++) {
        if (args[i].type == JDT_ARRAY) {
            arrObj->extend(ctx, (JsArray *)runtime->getObject(args[i]));
        } else {
            arrObj->push(ctx, args[i]);
        }
    }

    ctx->retValue = arr;
}

void arrayPrototypeCopyWithin(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto target = args.getIntAt(ctx, 0, 0);
    auto start = args.getIntAt(ctx, 1, 0);
    auto end = args.getIntAt(ctx, 2, MAX_INT32);

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.isString()) {
            auto length = thiz.type == JDT_CHAR ? 1 : runtime->getStringLength(thiz);
            normalizeStart(target, length);
            normalizeStart(start, length);
            normalizeEnd(end, length);

            if (start >= length || target >= length || start >= end) {
                ctx->retValue = newPrimaryObject(runtime, thiz);
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '%d' of object '[object String]'", target);
            }
        } else {
            ctx->retValue = newPrimaryObject(runtime, thiz);
        }
        return;
    }

    ctx->retValue = thiz;

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length) || length <= 0) {
        return;
    }

    normalizeStart(target, length);
    normalizeStart(start, length);
    normalizeEnd(end, length);

    if (target >= length || start >= end || target == start) {
        return;
    }

    if (target < start) {
        // 往左复制
        for (int i = start; i < end && ctx->error == JE_OK; i++) {
            auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            setPropByIndexEx(ctx, thiz, obj, value, target++);
        }
    } else {
        // 从右往左复制
        target += end - start - 1;
        if (target >= length) {
            end -= target + 1 - length;
            target = length - 1;
        }
        for (int i = end - 1; i >= start && ctx->error == JE_OK; i--) {
            auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            setPropByIndexEx(ctx, thiz, obj, value, target--);
        }
    }

    ctx->retValue = thiz;
}

class EntriesArrayLikeObjIterator: public IJsIterator {
public:
    EntriesArrayLikeObjIterator(VMContext *ctx, IJsObject *obj, uint32_t length) : IJsIterator(false, false), _ctx(ctx), _obj(obj), _length(length)
    {
        _index = 0;
        _isOfIterable = true;
    }
    ~EntriesArrayLikeObjIterator() {
    }

    virtual bool nextOf(JsValue &valueOut) override {
        assert(_obj);

        if (_index >= _length) {
            return false;
        }

        auto arrObj = new JsArray();
        auto arr = _ctx->runtime->pushObject(arrObj);

        arrObj->push(_ctx, makeJsValueInt32(_index));
        arrObj->push(_ctx, _obj->getByIndex(_ctx, _obj->self, _index));

        valueOut = _curValue = arr;

        _index++;
        return true;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        IJsIterator::markReferIdx(rt);

        rt->markReferIdx(_obj->self);
    }

protected:
    uint32_t                _index, _length;
    VMContext               *_ctx;
    IJsObject               *_obj;

};


class EntriesIterator : public IJsIterator {
public:
    EntriesIterator(VMContext *ctx, IJsIterator *other) : IJsIterator(false, false), _ctx(ctx), _other(other) {
        _index = 0;
        _isOfIterable = true;
    }
    ~EntriesIterator() {
        delete _other;
    }

    virtual bool nextOf(JsValue &valueOut) override {
        assert(_other);

        if (!_other->nextOf(valueOut)) {
            _curValue = jsValueUndefined;
            return false;
        }

        auto arrObj = new JsArray();
        auto arr = _ctx->runtime->pushObject(arrObj);

        arrObj->push(_ctx, makeJsValueInt32(_index));
        arrObj->push(_ctx, valueOut);

        valueOut = _curValue = arr;

        _index++;
        return true;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        IJsIterator::markReferIdx(rt);

        rt->markReferIdx(_other->self);
    }

protected:
    uint32_t                _index;
    VMContext               *_ctx;
    IJsIterator             *_other;

};

void arrayPrototypeEntries(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    IJsObject *obj = nullptr;

    if (thiz.type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    IJsIterator *it = nullptr;

    switch (thiz.type) {
        case JDT_CHAR:
        case JDT_STRING:
            it = newJsStringIterator(ctx, thiz, false);
            break;
        case JDT_ARRAY:
        case JDT_ARGUMENTS:
            obj = runtime->getObject(thiz);
            it = obj->getIteratorObject(ctx);
            break;
        default: {
            if (thiz.type >= JDT_OBJECT) {
                obj = runtime->getObject(thiz);
                auto n = runtime->toNumber(ctx, obj->getByName(ctx, thiz, SS_LENGTH));
                if (!isnan(n)) {
                    // Array like.
                    auto length = (int32_t)n;
                    if (length > 0) {
                        it = new EntriesArrayLikeObjIterator(ctx, obj, (uint32_t)length);
                        ctx->retValue = runtime->pushObject(it);
                        return;
                    }
                }
            }
            break;
        }
    }

    if (it == nullptr) {
        it = new EmptyJsIterator();
    } else {
        it = new EntriesIterator(ctx, it);
    }

    ctx->retValue = runtime->pushObject(it);
}

void arrayPrototypeEvery(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool result = true;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeActionEx(ctx, thiz, [ctx, vm, &result, callback, thisArg, thiz](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            return false;
        }

        result = false;
        return true;
    });

    ctx->retValue = makeJsValueBool(result);
}

void arrayPrototypeFill(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    IJsObject *obj = nullptr;
    JsValue retValue = thiz;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
            return;
        } else {
            retValue = newPrimaryObject(runtime, thiz);
            obj = runtime->getObject(retValue);
        }
    } else {
        obj = runtime->getObject(thiz);
    }

    int32_t length = 0;
    if (!obj->getLength(ctx, length) || length <= 0) {
        ctx->retValue = retValue;
        return;
    }

    auto value = args.getAt(0);
    auto start = args.getIntAt(ctx, 1, 0);
    auto end = args.getIntAt(ctx, 2, MAX_INT32);

    normalizeStart(start, length);
    normalizeEnd(end, length);

    if (start < end) {
        for (int i = start; i < end && ctx->error == JE_OK; i++) {
            auto ret = obj->setByIndex(ctx, retValue, i, value);
            if (ret != JE_OK) {
                throwModifyPropertyException(ctx, ret, retValue, i);
                break;
            }
        }
    }

    ctx->retValue = retValue;
}

void arrayPrototypeFilter(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);
    auto vm = ctx->vm;
    auto arrObj = new JsArray();
    auto arr = runtime->pushObject(arrObj);

    arrayLikeAction(ctx, thiz, [ctx, vm, callback, thisArg, thiz, arrObj](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            arrObj->push(ctx, item);
        }
    });

    ctx->retValue = arr;
}

void arrayPrototypeFind(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue result = jsValueUndefined;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeActionEx<false>(ctx, thiz, [ctx, vm, &result, callback, thisArg, thiz](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            result = item;
            return true;
        }
        return false;
    });

    ctx->retValue = result;
}

void arrayPrototypeFindIndex(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int32_t result = -1;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeActionEx<false>(ctx, thiz, [ctx, vm, &result, callback, thisArg, thiz](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            result = index;
            return true;
        }
        return false;
    });

    ctx->retValue = makeJsValueInt32(result);
}

void arrayPrototypeFindLast(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue result = jsValueUndefined;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeActionExReverse<false>(ctx, thiz, [ctx, vm, &result, callback, thisArg, thiz](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            result = item;
            return true;
        }
        return false;
    });

    ctx->retValue = result;
}

void arrayPrototypeFindLastIndex(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int32_t result = -1;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeActionExReverse<false>(ctx, thiz, [ctx, vm, &result, callback, thisArg, thiz](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            result = index;
            return true;
        }
        return false;
    });

    ctx->retValue = makeJsValueInt32(result);
}

void arrayPrototypeFlat(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto depth = args.getIntAt(ctx, 0);
    if (ctx->error != JE_OK) {
        return;
    }

    auto arrObj = new JsArray();
    auto arr = ctx->runtime->pushObject(arrObj);

    auto vm = ctx->vm;
    arrayLikeAction(ctx, thiz, [ctx, vm, arrObj, thiz, depth](int index, JsValue item) {
        if (item.type == JDT_ARRAY && depth > 0) {
            arrObj->extend(ctx, item, depth - 1);
        } else {
            arrObj->push(ctx, item);
        }
    });

    ctx->retValue = arr;
}

void arrayPrototypeFlatMap(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto callback = args.getAt(0);
    if (!callback.isFunction()) {
        ctx->throwException(JE_TYPE_ERROR, "flatMap mapper function is not callable");
        return;
    }

    auto thisArg = args.getAt(1, jsValueGlobalThis);
    auto arrObj = new JsArray();
    auto arr = ctx->runtime->pushObject(arrObj);

    auto vm = ctx->vm;
    arrayLikeAction(ctx, thiz, [ctx, vm, arrObj, thiz, callback, thisArg](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->retValue.type == JDT_ARRAY) {
            arrObj->extend(ctx, ctx->retValue, 0);
        } else {
            arrObj->push(ctx, ctx->retValue);
        }
    });

    ctx->retValue = arr;
}

void arrayPrototypeForEach(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeAction(ctx, thiz, [ctx, vm, thiz, callback, thisArg](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
    });

    ctx->retValue = jsValueUndefined;
}

/*
void arrayPrototypeGroup(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto index = args.getIntAt(ctx, 0, 0);

    ctx->retValue = jsValueUndefined;
}

void arrayPrototypeGroupToMap(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto index = args.getIntAt(ctx, 0, 0);

    ctx->retValue = jsValueUndefined;
}*/

void arrayPrototypeIncludes(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool result = false;
    auto expected = args.getAt(0);
    auto fromIndex = args.getIntAt(ctx, 1, 0);
    auto runtime = ctx->runtime;
    bool expectedNan = runtime->isNan(expected);

    arrayLikeActionEx<false>(ctx, thiz, [expectedNan, runtime, &result, expected](int index, JsValue item) {
        if (relationalStrictEqual(runtime, item, expected) || (runtime->isNan(item) && expectedNan)) {
            result = true;
            return true;
        }
        return false;
    }, fromIndex);

    ctx->retValue = makeJsValueBool(result);
}

void arrayPrototypeIndexOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int result = -1;
    auto expected = args.getAt(0);
    auto fromIndex = args.getIntAt(ctx, 1, 0);
    auto runtime = ctx->runtime;

    arrayLikeActionEx<true>(ctx, thiz, [runtime, &result, expected](int index, JsValue item) {
        if (relationalStrictEqual(runtime, item, expected)) {
            result = index;
            return true;
        }
        return false;
    }, fromIndex);

    ctx->retValue = makeJsValueInt32(result);
}

void arrayPrototypeJoin(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto sep = args.getAt(0);
    auto sepStr = runtime->toStringViewStrictly(ctx, sep);
    if (ctx->error != JE_OK) {
        return;
    }

    if (sep.type == JDT_UNDEFINED) {
        sepStr = StringView(",");
    }

    BinaryOutputStream stream;

    arrayLikeActionEx<false>(ctx, thiz, [ctx, runtime, &stream, &sepStr](int index, JsValue item) {
        auto s = runtime->toStringViewStrictly(ctx, item);
        if (ctx->error != JE_OK) {
            return true;
        }

        stream.write(s);
        return false;
    });

    ctx->retValue = runtime->pushString(stream.toStringView());
}


class IndexIterator: public IJsIterator {
public:
    IndexIterator(uint32_t length) : IJsIterator(false, false) {
        _index = 0;
        _length = length;
        _isOfIterable = true;
    }

    virtual bool nextOf(JsValue &valueOut) override {
        valueOut = makeJsValueInt32(_index);
        _index++;
        return _index <= _length;
    }

protected:
    uint32_t                _index, _length;

};


void arrayPrototypeKeys(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    int32_t length = 0;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
            return;
        } else if (thiz.type == JDT_CHAR) {
            length = 1;
        } else if (thiz.type == JDT_STRING) {
            length = runtime->getStringLength(thiz);
        }
    } else {
        IJsObject *obj = runtime->getObject(thiz);
        length = 0;
        if (!obj->getLength(ctx, length)) {
            length = 0;
        }
    }

    auto it = new IndexIterator(length);
    ctx->retValue = runtime->pushObject(it);
}

void arrayPrototypeLastIndexOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int result = -1;
    auto expected = args.getAt(0);
    auto fromIndex = args.getIntAt(ctx, 1, MAX_INT32);
    auto runtime = ctx->runtime;

    arrayLikeActionExReverse(ctx, thiz, [runtime, &result, expected](int index, JsValue item) {
        if (relationalStrictEqual(runtime, item, expected)) {
            result = index;
            return true;
        }
        return false;
    }, fromIndex);

    ctx->retValue = makeJsValueInt32(result);
}

void arrayPrototypeMap(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);
    auto arrObj = new JsArray();
    auto arr = ctx->runtime->pushObject(arrObj);
    auto vm = ctx->vm;
    auto runtime = ctx->runtime;

    auto action = [ctx, vm, arrObj, thiz, callback, thisArg](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        arrObj->push(ctx, ctx->retValue);
    };

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            action(0, thiz);
        } else if (thiz.type == JDT_STRING) {
            auto &str = runtime->getStringWithRandAccess(thiz);
            auto length = str.size();
            for (uint32_t i = 0; i < length && ctx->error == JE_OK; i++) {
                action(i, makeJsValueChar(str.chartAt(i)));
            }
        }
    } else {
        IJsObject *obj = runtime->getObject(thiz);
        int32_t length = 0;
        if (obj->getLength(ctx, length)) {
            for (int i = 0; i < length && ctx->error == JE_OK; i++) {
                auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
                if (value.isEmpty()) {
                    arrObj->push(ctx, jsValueUndefined);
                } else {
                    action(i, value);
                }
            }
        }
    }

    ctx->retValue = arr;
}

void arrayPrototypePop(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '0' of [object String]");
        } else if (thiz.type == JDT_STRING) {
            auto len = runtime->getStringLength(thiz);
            if (len == 0) {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property 'length' of object '[object String]'");
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '%d' of [object String]", len - 1);
            }
        } else {
            ctx->retValue = jsValueUndefined;
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    if (length <= 0) {
        obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(0));
        ctx->retValue = jsValueUndefined;
    } else {
        auto index = length - 1;
        auto v = obj->getByIndex(ctx, thiz, index);
        if (!obj->removeByIndex(ctx, index)) {
            auto s = objectPrototypeToStringView(thiz);
            ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '%d' of %.*s", index, s.len, s.data);
        } else {
            ctx->retValue = v;
            obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(index));
        }
    }
}

void arrayPrototypePush(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR || thiz.type == JDT_STRING) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property 'length' of object '[object String]'");
        } else {
            ctx->retValue = makeJsValueInt32(1);
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    obj->getLength(ctx, length);

    if (thiz.type == JDT_ARRAY) {
        auto *arr = (JsArray *)obj;
        auto err = arr->push(ctx, args.data, args.count);
        if (err != JE_OK) {
            throwModifyPropertyException(ctx, err, thiz, arr->length());
        }
        length += args.count;
    } else {
        for (int i = 0; i < args.count; i++) {
            auto err = obj->setByIndex(ctx, thiz, length++, args[i]);
            if (err != JE_OK) {
                throwModifyPropertyException(ctx, err, thiz, length - 1);
                break;
            }
        }

        if (ctx->error == JE_OK) {
            auto err = obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(length));
            if (err != JE_OK) {
                throwModifyPropertyException(ctx, err, thiz, length - 1);
            }
        }
    }

    ctx->retValue = makeJsValueInt32(length);
}

void arrayPrototypeReduce(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto vm = ctx->vm;
    auto callback = args.getAt(0);
    if (!callback.isFunction()) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "%.*s is not a function", callback);
        return;
    }

    auto accumulator = args.getAt(1, jsValueEmpty);

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            if (args.count == 1) {
                ctx->retValue = thiz;
            } else {
                ArgumentsX args(accumulator, thiz, makeJsValueInt32(0), thiz);
                vm->callMember(ctx, jsValueGlobalThis, callback, args);
            }
        } else if (thiz.type == JDT_STRING) {
            auto &str = runtime->getStringWithRandAccess(thiz);
            auto length = str.size();

            uint32_t i = 0;
            if (args.count == 1) {
                if (length == 0) {
                    ctx->throwException(JE_TYPE_ERROR, "Reduce of empty array with no initial value");
                    return;
                }

                accumulator = makeJsValueChar(str.chartAt(0));
                i = 1;
            }

            for (; i < length && ctx->error == JE_OK; i++) {
                ArgumentsX args(accumulator, makeJsValueChar(str.chartAt(i)), makeJsValueInt32(i), thiz);
                vm->callMember(ctx, jsValueGlobalThis, callback, args);
                accumulator = ctx->retValue;
            }
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    uint32_t i = 0;
    if (args.count == 1) {
        while (i < length && accumulator.isEmpty()) {
            accumulator = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            i++;
        }
    }

    if (accumulator.isEmpty()) {
        ctx->throwException(JE_TYPE_ERROR, "Reduce of empty array with no initial value");
        return;
    }

    for (; i < length && ctx->error == JE_OK; i++) {
        auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
        if (value.isValid()) {
            ArgumentsX args(accumulator, value, makeJsValueInt32(i), thiz);
            vm->callMember(ctx, jsValueGlobalThis, callback, args);
            accumulator = ctx->retValue;
        }
    }

    ctx->retValue = accumulator;
}

void arrayPrototypeReduceRight(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto vm = ctx->vm;
    auto callback = args.getAt(0);
    if (!callback.isFunction()) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "%.*s is not a function", callback);
        return;
    }

    auto accumulator = args.getAt(1, jsValueEmpty);

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            if (args.count == 1) {
                ctx->retValue = thiz;
            } else {
                ArgumentsX args(accumulator, thiz, makeJsValueInt32(0), thiz);
                vm->callMember(ctx, jsValueGlobalThis, callback, args);
            }
        } else if (thiz.type == JDT_STRING) {
            auto &str = runtime->getStringWithRandAccess(thiz);
            auto length = str.size();

            int32_t i = length - 1;
            if (args.count == 1) {
                if (length == 0) {
                    ctx->throwException(JE_TYPE_ERROR, "Reduce of empty array with no initial value");
                    return;
                }

                accumulator = makeJsValueChar(str.chartAt(i--));
            }

            for (; i >= 0 && ctx->error == JE_OK; i--) {
                ArgumentsX args(accumulator, makeJsValueChar(str.chartAt(i)), makeJsValueInt32(i), thiz);
                vm->callMember(ctx, jsValueGlobalThis, callback, args);
                accumulator = ctx->retValue;
            }
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    int32_t i = length - 1;
    if (args.count == 1) {
        while (i >= 0 && accumulator.isEmpty()) {
            accumulator = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            i--;
        }
    }

    if (accumulator.isEmpty()) {
        ctx->throwException(JE_TYPE_ERROR, "Reduce of empty array with no initial value");
        return;
    }

    for (; i >= 0 && ctx->error == JE_OK; i--) {
        auto value = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
        if (value.isValid()) {
            ArgumentsX args(accumulator, value, makeJsValueInt32(i), thiz);
            vm->callMember(ctx, jsValueGlobalThis, callback, args);
            accumulator = ctx->retValue;
        }
    }

    ctx->retValue = accumulator;
}

void arrayPrototypeReverse(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '0' of object '[object String]'");
        } else if (thiz.type == JDT_STRING) {
            auto len = runtime->getStringLength(thiz);
            if (len == 0) {
                ctx->retValue = newPrimaryObject(runtime, thiz);
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '0' of object '[object String]'");
            }
        } else {
            ctx->retValue = newPrimaryObject(runtime, thiz);
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    if (thiz.type == JDT_ARRAY) {
        ((JsArray *)obj)->reverse(ctx);
        ctx->retValue = thiz;
        return;
    }

    int32_t length = 0;
    if (!obj->getLength(ctx, length) && length > 0) {
        ctx->retValue = thiz;
        return;
    }

    length--;
    for (uint32_t i = 0; i < length && ctx->error == JE_OK; i++, length--) {
        auto left = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
        auto right = obj->getByIndex(ctx, thiz, length, jsValueEmpty);
        setPropByIndexEx(ctx, thiz, obj, right, i);
        setPropByIndexEx(ctx, thiz, obj, left, length);
    }

    ctx->retValue = thiz;
}

void arrayPrototypeShift(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '0' of [object String]");
        } else if (thiz.type == JDT_STRING) {
            auto len = runtime->getStringLength(thiz);
            if (len == 0) {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property 'length' of object '[object String]'");
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '0' of object '[object String]'");
            }
        } else {
            ctx->retValue = jsValueUndefined;
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    if (thiz.type == JDT_ARRAY) {
        auto ret = ((JsArray *)obj)->popFront(ctx);
        if (std::get<1>(ret) != JE_OK) {
            throwModifyPropertyException(ctx, std::get<1>(ret), thiz, std::get<2>(ret));
        } else {
            ctx->retValue = std::get<0>(ret);
        }
        return;
    }

    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(0));
        ctx->retValue = jsValueUndefined;
        return;
    }

    if (length <= 0) {
        obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(0));
        ctx->retValue = jsValueUndefined;
    } else {
        auto ret = obj->getByIndex(ctx, thiz, 0);
        for (int32_t i = 1; i < length && ctx->error == JE_OK; i++) {
            auto v = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
            setPropByIndexEx(ctx, thiz, obj, v, i - 1);
        }
        if (ctx->error == JE_OK) {
            if (obj->removeByIndex(ctx, length - 1)) {
                obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(length - 1));
            } else {
                throwModifyPropertyException(ctx, JE_TYPE_PROP_NO_DELETABLE, thiz, length - 1);
            }
        }
        ctx->retValue = ret;
    }
}

void arrayPrototypeSlice(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto start = args.getIntAt(ctx, 0, 0);
    auto end = args.getIntAt(ctx, 1, MAX_INT32);

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
            return;
        }

        auto *arrObj = new JsArray();
        auto arr = runtime->pushObject(arrObj);

        if (thiz.type == JDT_CHAR) {
            if (start <= 0 && end >= 1) {
                arrObj->push(ctx, thiz);
            }
        } else if (thiz.type == JDT_STRING) {
            auto str = runtime->getStringWithRandAccess(thiz);
            normalizeStart(start, str.size());
            normalizeEnd(end, str.size());

            for (auto i = start; i < end; i++) {
                arrObj->push(ctx, makeJsValueChar(str.chartAt(i)));
            }
        }

        ctx->retValue = arr;
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    normalizeStart(start, length);
    normalizeEnd(end, length);

//    if (thiz.type == JDT_ARRAY) {
//        ((JsArray *)obj)->slice(ctx, start, end);
//        return;
//    }

    auto *arrObj = new JsArray();
    auto arr = runtime->pushObject(arrObj);

    for (int32_t i = start; i < end && ctx->error == JE_OK; i++) {
        auto v = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
        arrObj->push(ctx, v);
    }

    ctx->retValue = arr;
}

void arrayPrototypeSome(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool result = false;
    auto callback = args.getAt(0);
    auto thisArg = args.getAt(1, jsValueGlobalThis);

    auto vm = ctx->vm;
    arrayLikeActionEx(ctx, thiz, [ctx, vm, &result, callback, thisArg, thiz](int index, JsValue item) {
        ArgumentsX args(item, makeJsValueInt32(index), thiz);
        vm->callMember(ctx, thisArg, callback, args);
        if (ctx->error == JE_OK && ctx->runtime->testTrue(ctx->retValue)) {
            result = true;
            return true;
        }
        return false;
    });

    ctx->retValue = makeJsValueBool(result);
}

void arrayPrototypeSort(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        } else if (thiz.type == JDT_CHAR) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '0' of object '[object String]'");
        } else if (thiz.type == JDT_STRING) {
            auto len = runtime->getStringLength(thiz);
            if (len == 0) {
                ctx->retValue = newPrimaryObject(runtime, thiz);
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '0' of object '[object String]'");
            }
        } else {
            ctx->retValue = newPrimaryObject(runtime, thiz);
        }
        return;
    }

    auto callback = args.getAt(0);

    IJsObject *obj = runtime->getObject(thiz);
    if (thiz.type == JDT_ARRAY) {
        ((JsArray *)obj)->sort(ctx, callback);
        ctx->retValue = thiz;
        return;
    }

    int32_t length = 0;
    if (!obj->getLength(ctx, length) && length > 0) {
        ctx->retValue = thiz;
        return;
    }

    VecJsValues values;
    int countEmpty = 0, countUndefined = 0;
    for (uint32_t i = 0; i < length && ctx->error == JE_OK; i++) {
        auto v = obj->getByIndex(ctx, thiz, i, jsValueEmpty);
        if (v.isEmpty()) {
            countEmpty++;
        } else if (v.type == JDT_UNDEFINED) {
            countUndefined++;
        } else {
            values.push_back(v);
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
    for (; i < (int)values.size() && ctx->error == JE_OK; i++) {
        obj->setByIndex(ctx, thiz, i, values[i]);
    }

    // undefined
    countUndefined += i;
    for (; i < countUndefined && ctx->error == JE_OK; i++) {
        obj->setByIndex(ctx, thiz, i, jsValueUndefined);
    }

    // empty
    countEmpty += i;
    for (; i < countEmpty && ctx->error == JE_OK; i++) {
        if (!obj->removeByIndex(ctx, i)) {
            throwModifyPropertyException(ctx, JE_TYPE_PROP_NO_DELETABLE, thiz, i);
            break;
        }
    }

    ctx->retValue = thiz;

}

void arrayPrototypeSplice(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto start = args.getIntAt(ctx, 0, 0);
    auto deleteCount = args.getIntAt(ctx, 1, MAX_INT32);
    if (deleteCount < 0) {
        deleteCount = 0;
    }

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
            return;
        }

        if (thiz.isString()) {
            int length = thiz.type == JDT_CHAR ? 1 : runtime->getStringLength(thiz);
            normalizeStart(start, length);

            if (start >= length) {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property 'length' of object '[object String]'");
            } else if (deleteCount >= length || start + deleteCount >= length) {
                ctx->throwException(JE_TYPE_ERROR, "Cannot delete property '%d' of [object String]", length - 1);
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '%d' of object '[object String]'",
                                    deleteCount >= args.count - 2 ? start : length - 1);
            }
        }

        if (ctx->error == JE_OK) {
            auto *arrObj = new JsArray();
            auto arr = runtime->pushObject(arrObj);
            ctx->retValue = arr;
        }
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    if (!obj->getLength(ctx, length)) {
        return;
    }

    normalizeStart(start, length);
    if (start + (int64_t)deleteCount > (int64_t)length) {
        deleteCount = length - start;
    }

//    if (thiz.type == JDT_ARRAY) {
//        ((JsArray *)obj)->slice(ctx, start, end);
//        return;
//    }

    auto *arrObj = new JsArray();
    auto arr = runtime->pushObject(arrObj);

    // 将要删除的元素保存下来
    for (int i = 0; i < deleteCount && ctx->error == JE_OK; i++) {
        auto v = obj->getByIndex(ctx, thiz, i + start);
        arrObj->push(ctx, v);
    }

    // 移动
    int insertCount = args.count > 2 ? args.count - 2 : 0;
    if (insertCount > deleteCount) {
        // 往右移动
        int src = length - 1, dst = length - 1 + insertCount - deleteCount;
        int srcBegin = start + deleteCount;
        for (; src >= srcBegin && ctx->error == JE_OK; src--, dst--) {
            auto v = obj->getByIndex(ctx, thiz, src, jsValueEmpty);
            setPropByIndexEx(ctx, thiz, obj, v, dst);
        }
    } else if (insertCount == deleteCount) {
        // 不移动
    } else {
        // 往左移动
        int src = start + deleteCount;
        int dst = start + insertCount;
        for (; src < length && ctx->error == JE_OK; src++, dst++) {
            auto v = obj->getByIndex(ctx, thiz, src, jsValueEmpty);
            setPropByIndexEx(ctx, thiz, obj, v, dst);
        }

        // 删除最后的元素
        for (; dst < length && ctx->error == JE_OK; dst++) {
            if (!obj->removeByIndex(ctx, dst)) {
                throwModifyPropertyException(ctx, JE_TYPE_PROP_NO_DELETABLE, thiz, dst);
            }
        }
    }

    // 插入
    for (int32_t i = 0; i < insertCount && ctx->error == JE_OK; i++) {
        auto err = obj->setByIndex(ctx, thiz, start + i, args[i + 2]);
        if (err != JE_OK) {
            throwModifyPropertyException(ctx, err, thiz, start + i);
        }
    }

    if (ctx->error == JE_OK) {
        length += insertCount - deleteCount;
        auto err = obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(length));
        if (err != JE_OK) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property 'length' of object");
        }
    }

    ctx->retValue = arr;
}

void arrayPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type != JDT_ARRAY) {
        objectPrototypeToString(ctx, thiz, args);
        return;
    }

    JsArray *arr = (JsArray *)runtime->getObject(thiz);

    BinaryOutputStream stream;
    arr->toString(ctx, thiz, stream);
    ctx->retValue = runtime->pushString(stream.toStringView());
}

void arrayPrototypeUnshift(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
            return;
        }

        if (thiz.isString()) {
            if (args.count == 0) {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property 'length' of object '[object String]'");
            } else {
                ctx->throwException(JE_TYPE_ERROR, "Cannot assign to read only property '%d' of object '[object String]'", thiz.type == JDT_CHAR ? 1 : runtime->getStringLength(thiz) - 1);
            }
        }

        ctx->retValue = makeJsValueInt32(1);
        return;
    }

    IJsObject *obj = runtime->getObject(thiz);
    int32_t length = 0;
    obj->getLength(ctx, length);

    if (args.count > 0) {
        // 移动
        // 往右移动
        int src = length - 1, dst = length - 1 + args.count;
        for (; src >= 0 && ctx->error == JE_OK; src--, dst--) {
            auto v = obj->getByIndex(ctx, thiz, src, jsValueEmpty);
            setPropByIndexEx(ctx, thiz, obj, v, dst);
        }

        // 插入
        for (int32_t i = 0; i < args.count && ctx->error == JE_OK; i++) {
            obj->setByIndex(ctx, thiz, i, args[i]);
        }
    }

    length += args.count;
    obj->setByName(ctx, thiz, SS_LENGTH, makeJsValueInt32(length));

    ctx->retValue = makeJsValueInt32(length);
}

class ValuesArrayLikeObjIterator : public IJsIterator {
public:
    ValuesArrayLikeObjIterator(VMContext *ctx, IJsObject *obj, uint32_t length) : IJsIterator(false, false), _ctx(ctx), _obj(obj), _length(length) {
        _index = 0;
        _isOfIterable = true;
    }

    virtual bool nextOf(JsValue &valueOut) override {
        assert(_obj);

        if (_index >= _length) {
            return false;
        }

        valueOut = _curValue = _obj->getByIndex(_ctx, _obj->self, _index);

        _index++;
        return true;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        IJsIterator::markReferIdx(rt);

        rt->markReferIdx(_obj->self);
    }

protected:
    uint32_t                _index, _length;
    VMContext               *_ctx;
    IJsObject               *_obj;

};

void arrayPrototypeValues(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    IJsIterator *it = nullptr;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type <= JDT_NULL) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
            return;
        }

        if (thiz.isString()) {
            it = newJsStringIterator(ctx, thiz, false);
        } else {
            it = new EmptyJsIterator();
        }
    } else {
        IJsObject *obj = runtime->getObject(thiz);
        if (thiz.type == JDT_ARRAY) {
            it = ((JsArray *)obj)->getIteratorObject(ctx);
        } else {
            int32_t length = 0;
            if (obj->getLength(ctx, length)) {
                it = new ValuesArrayLikeObjIterator(ctx, obj, length);
            } else {
                it = new EmptyJsIterator();
            }
        }
    }

    assert(it != nullptr);
    ctx->retValue = runtime->pushObject(it);
}

static JsLibProperty arrayPrototypeFunctions[] = {
    { "at", arrayPrototypeAt },
    { "concat", arrayPrototypeConcat },
    { "copyWithin", arrayPrototypeCopyWithin },
    { "entries", arrayPrototypeEntries },
    { "every", arrayPrototypeEvery },
    { "fill", arrayPrototypeFill },
    { "filter", arrayPrototypeFilter },
    { "find", arrayPrototypeFind },
    { "findIndex", arrayPrototypeFindIndex },
    { "findLast", arrayPrototypeFindLast },
    { "findLastIndex", arrayPrototypeFindLastIndex },
    { "flat", arrayPrototypeFlat },
    { "flatMap", arrayPrototypeFlatMap },
    { "forEach", arrayPrototypeForEach },
    // { "group", arrayPrototypeGroup },
    // { "groupToMap", arrayPrototypeGroupToMap },
    { "includes", arrayPrototypeIncludes },
    { "indexOf", arrayPrototypeIndexOf },
    { "join", arrayPrototypeJoin },
    { "keys", arrayPrototypeKeys },
    { "lastIndexOf", arrayPrototypeLastIndexOf },
    { "map", arrayPrototypeMap },
    { "pop", arrayPrototypePop },
    { "push", arrayPrototypePush },
    { "reduce", arrayPrototypeReduce },
    { "reduceRight", arrayPrototypeReduceRight },
    { "reverse", arrayPrototypeReverse },
    { "shift", arrayPrototypeShift },
    { "slice", arrayPrototypeSlice },
    { "some", arrayPrototypeSome },
    { "sort", arrayPrototypeSort },
    { "splice", arrayPrototypeSplice },
    { "toLocaleString", arrayPrototypeToString },
    { "toString", arrayPrototypeToString },
    { "unshift", arrayPrototypeUnshift },
    { "values", arrayPrototypeValues },
};

void registerArray(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, arrayPrototypeFunctions, CountOf(arrayPrototypeFunctions));
    prototypeObj->setOfIteratorTrue();

    rt->objPrototypeArray = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeArray, prototypeObj);

    SET_PROTOTYPE(arrayFunctions, jsValuePrototypeArray);

    setGlobalLibObject("Array", rt, arrayFunctions, CountOf(arrayFunctions), arrayConstructor, jsValuePrototypeFunction);
}
