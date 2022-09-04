//
//  BinaryOperation.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/4.
//

#ifndef BinaryOperation_hpp
#define BinaryOperation_hpp


struct BinaryOpSub {
    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        int64_t r = (int64_t)a - (int64_t)b;
        if (r == (int32_t)r) {
            return JsValue(JDT_INT32, (int32_t)r);
        } else {
            return rt->pushDoubleValue(r);
        }
    }

    JsValue operator()(VMRuntime *rt, double a, int32_t b) const {
        auto r = a - b;
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double b) const {
        auto r = a - b;
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, double a, double b) const {
        auto r = a - b;
        return rt->pushDoubleValue(r);
    }
};


struct BinaryOpMul {
    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        int64_t r = (int64_t)a * (int64_t)b;
        if (r == (int32_t)r) {
            return JsValue(JDT_INT32, (int32_t)r);
        } else {
            return rt->pushDoubleValue(r);
        }
    }

    JsValue operator()(VMRuntime *rt, double a, int32_t b) const {
        auto r = a * b;
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double b) const {
        auto r = a * b;
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, double a, double b) const {
        auto r = a * b;
        return rt->pushDoubleValue(r);
    }
};

struct BinaryOpDiv {
    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        if (b == 0) {
            return a == 0 ? jsValueNaN : jsValueInf;
        }

        int64_t r = (int64_t)a / (int64_t)b;
        if (r == (int32_t)r) {
            return JsValue(JDT_INT32, (int32_t)r);
        } else {
            return rt->pushDoubleValue(r);
        }
    }

    JsValue operator()(VMRuntime *rt, double a, int32_t b) const {
        auto r = a / b;
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double b) const {
        auto r = a / b;
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, double a, double b) const {
        auto r = a / b;
        return rt->pushDoubleValue(r);
    }
};


// 除了加法之外的其他数学运算
template<typename Operator>
inline JsValue arithmeticBinaryOperation(VMContext *ctx, VMRuntime *rt, const JsValue &left, const JsValue &right, const Operator &op) {
    // 算术运算一般都是 number 类型的，快速针对这些类型进行判断
    if (left.type == JDT_INT32) {
        if (right.type == JDT_INT32) {
            return op(rt, left.value.n32, right.value.n32);
        } else if (right.type == JDT_NUMBER) {
            return op(rt, left.value.n32, rt->getDouble(right));
        } else {
            return op(rt, left.value.n32, rt->toNumber(ctx, right));
        }
    } else if (left.type == JDT_NUMBER) {
        if (right.type == JDT_INT32) {
            return op(rt, rt->getDouble(left), right.value.n32);
        } else if (right.type == JDT_NUMBER) {
            return op(rt, rt->getDouble(left), rt->getDouble(right));
        } else {
            return op(rt, rt->getDouble(left), rt->toNumber(ctx, right));
        }
    } else {
        return op(rt, rt->toNumber(ctx, left), rt->toNumber(ctx, right));
    }
}

inline JsValue plusOperate(VMContext *ctx, VMRuntime *rt, int32_t left, const JsValue &leftStr, const JsValue &right) {
    switch (right.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            return jsValueNaN;
        case JDT_NULL:
            return JsValue(JDT_INT32, left);
        case JDT_BOOL:
        case JDT_INT32: {
            auto r = (int64_t)left + right.value.n32;
            if (r == (int32_t)r) {
                return JsValue(JDT_INT32, (int32_t)r);
            }
            return rt->pushDoubleValue(r);
        }
        case JDT_NUMBER: {
            auto r = left + rt->getDouble(right);
            return rt->pushDoubleValue(r);
        }
        case JDT_SYMBOL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
            return jsValueNaN;
        case JDT_CHAR: {
            char buf[64];
            uint32_t len = 0;
            if (leftStr.isValid()) {
                SizedString str = rt->getString(leftStr);
                assert(len < CountOf(buf));
                memcpy(buf, str.data, str.len);
                len = str.len;
            } else {
                len = (uint32_t)::itoa(left, buf);
            }
            buf[len++] = (char )right.value.index;
            return rt->pushString(SizedString(buf, len));
        }
        default: {
            JsValue a = leftStr;
            if (!leftStr.isValid()) {
                SizedStringWrapper s(left);
                a = rt->toString(ctx, right);
            }

            JsValue r = rt->toString(ctx, right);
            return rt->addString(a, r);
        }
    }
}

inline JsValue plusOperate(VMContext *ctx, VMRuntime *rt, const JsValue &left, const JsValue &right) {
    switch (left.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                case JDT_NULL:
                case JDT_BOOL:
                case JDT_INT32:
                case JDT_NUMBER:
                    // 转换为 number，算 NaN
                    return jsValueNaN;
                case JDT_SYMBOL:
                    ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
                    return jsValueNaN;
                case JDT_CHAR: {
                    char buf[32] = "undefined";
                    buf[SS_UNDEFINED.len] = (char)right.value.index;
                    return rt->pushString(SizedString(buf, SS_UNDEFINED.len + 1));
                }
                case JDT_STRING:
                    return rt->addString(JsStringValueUndefined, right);
                default: {
                    auto r = rt->toString(ctx, right);
                    return rt->addString(JsStringValueUndefined, r);
                }
            }
            break;
        case JDT_NULL:
            return plusOperate(ctx, rt, 0, JsStringValueNull, right);
        case JDT_BOOL:
            return plusOperate(ctx, rt, left.value.n32, left.value.n32 ? JsStringValueTrue : JsStringValueFalse, right);
        case JDT_INT32:
            return plusOperate(ctx, rt, left.value.n32, jsValueNotInitialized, right);
        case JDT_NUMBER: {
            auto n = rt->getDouble(left);
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                    return jsValueNaN;
                case JDT_NULL:
                    return left;
                case JDT_BOOL:
                case JDT_INT32: {
                    auto r = n + right.value.n32;
                    if (r == (int32_t)r) {
                        return JsValue(JDT_INT32, (int32_t)r);
                    }
                    return rt->pushDoubleValue(r);
                }
                case JDT_NUMBER: {
                    auto r = n + rt->getDouble(right);
                    return rt->pushDoubleValue(r);
                }
                case JDT_SYMBOL:
                    ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
                    return jsValueNaN;
                default: {
                    auto s = rt->toString(ctx, left);
                    auto r = rt->toString(ctx, right);
                    return rt->addString(s, r);
                }
            }
            break;
        }
        case JDT_CHAR: {
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                case JDT_NULL:
                case JDT_BOOL:
                case JDT_INT32:
                case JDT_CHAR: {
                    SizedStringWrapper str(left);
                    str.append(right);
                    return rt->pushString(str.str());
                }
                case JDT_STRING: {
                    SizedStringWrapper str1(left);
                    return rt->addString(str1, right);
                }
                case JDT_NUMBER: {
                    SizedStringWrapper str(left);
                    str.append(right);
                    return rt->pushString(str.str());
                }
                case JDT_SYMBOL:
                    ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
                    return jsValueNaN;
                default: {
                    SizedStringWrapper str1(left);
                    auto r = rt->toString(ctx, right);
                    return rt->addString(str1.str(), r);
                }
            }
            break;
        }
        case JDT_STRING: {
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                    return rt->addString(left, jsValueUndefined);
                case JDT_NULL:
                    return rt->addString(left, jsValueNull);
                case JDT_BOOL:
                    return rt->addString(left, right.value.n32 ? JsStringValueTrue : JsStringValueFalse);
                case JDT_INT32: {
                    SizedStringWrapper str(right);
                    return rt->addString(left, str);
                }
                case JDT_CHAR: {
                    SizedStringWrapper str(right);
                    return rt->addString(left, str);
                }
                case JDT_STRING: {
                    return rt->addString(left, right);
                }
                case JDT_NUMBER: {
                    SizedStringWrapper str(rt->getDouble(right));
                    return rt->addString(left, str);
                }
                case JDT_SYMBOL:
                    ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
                    return jsValueNaN;
                default: {
                    auto r = rt->toString(ctx, right);
                    return rt->addString(left, r);
                }
            }
            break;
        }
        case JDT_SYMBOL: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
            return jsValueNaN;
        }
        default: {
            auto str1 = rt->toString(ctx, left);
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                    return rt->addString(str1, jsValueUndefined);
                case JDT_NULL:
                    return rt->addString(str1, jsValueNull);
                case JDT_BOOL:
                    return rt->addString(str1, right.value.n32 ? JsStringValueTrue : JsStringValueFalse);
                case JDT_INT32: {
                    SizedStringWrapper str(right);
                    return rt->addString(str1, str);
                }
                case JDT_CHAR: {
                    SizedStringWrapper str(right);
                    return rt->addString(str1, str);
                }
                case JDT_STRING: {
                    return rt->addString(str1, right);
                }
                case JDT_NUMBER: {
                    SizedStringWrapper str(rt->getDouble(right));
                    return rt->addString(str1, str);
                }
                case JDT_SYMBOL:
                    ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
                    return jsValueNaN;
                default: {
                    auto r = rt->toString(ctx, right);
                    return rt->addString(str1, r);
                }
            }
            break;
        }
    }
}

#endif /* BinaryOperation_hpp */
