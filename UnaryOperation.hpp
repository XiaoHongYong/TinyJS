//
//  UnaryOperation.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/9.
//

#ifndef UnaryOperation_hpp
#define UnaryOperation_hpp


JsValue increaseJsValue(VMContext *ctx, JsValue &v, int inc, bool isPost) {
    JsValue org = jsValueNaN;
    auto runtime = ctx->runtime;

    switch (v.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED: v = jsValueNaN; break;;
        case JDT_NULL: org = JsValue(JDT_INT32, 0); v = JsValue(JDT_INT32, inc); break;;
        case JDT_BOOL: org = JsValue(JDT_INT32, v.value.n32); v = JsValue(JDT_INT32, v.value.n32 + inc); break;;
        case JDT_INT32: {
            org = v;
            int64_t n = v.value.n32;
            n += inc;
            if (n == (int32_t)n) {
                v = JsValue(JDT_INT32, (int32_t)n);
            } else {
                v = runtime->pushDoubleValue(n);
            }
            break;
        }
        case JDT_NUMBER: {
            org = v;
            v = runtime->pushDoubleValue(runtime->getDouble(v) + inc);
            break;
        }
        case JDT_SYMBOL: {
            ctx->throwException(PE_TYPE_ERROR, " Cannot convert a Symbol value to a number");
            break;;
        }
        case JDT_CHAR: {
            if (isdigit(v.value.n32)) {
                org = JsValue(JDT_INT32, v.value.n32 - '0');
                v = JsValue(JDT_INT32, v.value.n32 - '0' + inc);
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
                    org = JsValue(JDT_INT32, (int32_t)n - inc);
                    v = JsValue(JDT_INT32, n);
                } else {
                    org = runtime->pushDoubleValue(n - inc);
                    v = runtime->pushDoubleValue(n);
                }
            } else {
                v = jsValueNaN;
            }
            break;
        }
    }

    return isPost ? org : v;
}

inline JsValue increaseMemberDot(VMContext *ctx, const JsValue &obj, SizedString &name, int inc, bool isPost) {
    auto runtime = ctx->runtime;

    switch (obj.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", (int)name.len, name.data);
            return jsValueNaN;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", (int)name.len, name.data);
            return jsValueNaN;
        case JDT_BOOL:
        case JDT_INT32:
        case JDT_NUMBER:
        case JDT_SYMBOL:
            return jsValueNaN;
        case JDT_CHAR:
        case JDT_STRING:
            return runtime->objPrototypeString->increaseByName(ctx, obj, name, inc, isPost);
        default: {
            auto pobj = runtime->getObject(obj);
            assert(pobj);
            return pobj->increaseByName(ctx, obj, name, inc, isPost);
        }
    }

    return jsValueUndefined;
}

inline void throwReadPropertyException(VMContext *ctx, cstr_t format, const JsValue &index) {
    string buf;
    auto name = ctx->runtime->toSizedString(ctx, index, buf);
    ctx->throwException(PE_TYPE_ERROR, format, (int)name.len, name.data);
}

inline JsValue increaseMemberIndex(VMContext *ctx, const JsValue &obj, JsValue &index, int inc, bool isPost) {
    auto runtime = ctx->runtime;

    switch (obj.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            throwReadPropertyException(ctx, "Cannot read properties of undefined (reading '%.*s')", index);
            return jsValueNaN;
        case JDT_NULL:
            throwReadPropertyException(ctx, "Cannot read properties of null (reading '%.*s')", index);
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

#endif /* UnaryOperation_hpp */
