//
//  UnaryOperation.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/9.
//

#ifndef UnaryOperation_hpp
#define UnaryOperation_hpp


inline JsValue increaseJsValue(VMContext *ctx, JsValue &v, int inc, bool isPost) {
    JsValue org = jsValueNaN;
    auto runtime = ctx->runtime;

    switch (v.type) {
        case JDT_UNDEFINED: v = jsValueNaN; break;
        case JDT_NULL: org = makeJsValueInt32(0); v = makeJsValueInt32(inc); break;
        case JDT_BOOL: org = makeJsValueInt32(v.value.n32); v = makeJsValueInt32(v.value.n32 + inc); break;
        case JDT_INT32: {
            org = v;
            int64_t n = v.value.n32;
            n += inc;
            if (n == (int32_t)n) {
                v = makeJsValueInt32((int32_t)n);
            } else {
                v = runtime->pushDouble(n);
            }
            break;
        }
        case JDT_NUMBER: {
            org = v;
            v = runtime->pushDouble(runtime->getDouble(v) + inc);
            break;
        }
        case JDT_SYMBOL: {
            ctx->throwException(JE_TYPE_ERROR, " Cannot convert a Symbol value to a number");
            break;
        }
        case JDT_CHAR: {
            if (isdigit(v.value.n32)) {
                org = makeJsValueInt32(v.value.n32 - '0');
                v = makeJsValueInt32(v.value.n32 - '0' + inc);
            } else {
                v = jsValueNaN;
            }
            break;
        }
        default: {
            double n;
            if (runtime->toNumber(ctx, v, n)) {
                n += inc;
                if (n == (int32_t)n) {
                    org = makeJsValueInt32((int32_t)n - inc);
                    v = makeJsValueInt32((int32_t)n);
                } else {
                    org = runtime->pushDouble(n - inc);
                    v = runtime->pushDouble(n);
                }
            } else {
                v = jsValueNaN;
            }
            break;
        }
    }

    return isPost ? org : v;
}

inline JsValue increaseMemberDot(VMContext *ctx, const JsValue &obj, StringView &name, int inc, bool isPost) {
    auto runtime = ctx->runtime;

    switch (obj.type) {
        case JDT_UNDEFINED:
            ctx->throwException(JE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", (int)name.len, name.data);
            return jsValueNaN;
        case JDT_NULL:
            ctx->throwException(JE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", (int)name.len, name.data);
            return jsValueNaN;
        case JDT_BOOL:
        case JDT_INT32:
        case JDT_NUMBER:
        case JDT_SYMBOL:
            return jsValueNaN;
        case JDT_CHAR:
        case JDT_STRING:
            return runtime->objPrototypeString()->increaseByName(ctx, obj, name, inc, isPost);
        default: {
            auto pobj = runtime->getObject(obj);
            assert(pobj);
            return pobj->increaseByName(ctx, obj, name, inc, isPost);
        }
    }

    return jsValueUndefined;
}

inline JsValue increaseMemberIndex(VMContext *ctx, const JsValue &obj, JsValue &index, int inc, bool isPost) {
    auto runtime = ctx->runtime;

    switch (obj.type) {
        case JDT_UNDEFINED:
            ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", index);
            return jsValueNaN;
        case JDT_NULL:
            ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", index);
            return jsValueNaN;
        case JDT_BOOL:
        case JDT_INT32:
        case JDT_NUMBER:
        case JDT_SYMBOL:
            return jsValueNaN;
        case JDT_CHAR:
        case JDT_STRING: {
            auto value = ctx->vm->getMemberIndex(ctx, obj, index);
            return increaseJsValue(ctx, value, inc, isPost);
        }
        default: {
            auto pobj = runtime->getObject(obj);
            assert(pobj);
            return pobj->increase(ctx, obj, index, inc, isPost);
        }
    }

    return jsValueUndefined;
}

inline JsValue bitNotOperation(VMContext *ctx, JsValue &v) {
    if (v.type == JDT_INT32) {
        return makeJsValueInt32(~v.value.n32);
    }

    auto runtime = ctx->runtime;
    int32_t n = 0;

    switch (v.type) {
        case JDT_UNDEFINED:
        case JDT_NULL:
            break;
        case JDT_BOOL: n = v.value.n32; break;
            break;
        case JDT_NUMBER: n = doubleToInt32(runtime->getDouble(v)); break;
        case JDT_SYMBOL:
            ctx->throwException(JE_TYPE_ERROR, " Cannot convert a Symbol value to a number");
            break;
        default: {
            double d;
            if (runtime->toNumber(ctx, v, d)) {
                n = doubleToInt32(d);
            } else {
                n = 0;
            }
            break;
        }
    }

    return makeJsValueInt32(~n);
}


#endif /* UnaryOperation_hpp */
