//
//  BinaryOperation.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/4.
//

#ifndef BinaryOperation_hpp
#define BinaryOperation_hpp


inline bool throwSymbolConvertException(VMContext *ctx, bool isToNumber = true) {
    ctx->throwException(PE_TYPE_ERROR,
        isToNumber ? "Cannot convert a Symbol value to a number" : "Cannot convert a Symbol value to a string");
    return false;
}

inline void throwSymbolConvertStringException(VMContext *ctx) {
    ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a string");
}

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
        if (r == 0) {
            // 特殊处理 -0 的情况
            double r = (double)a * (double)b;
            if (signbit(r)) {
                return rt->pushDoubleValue(r);
            } else {
                return JsValue(JDT_INT32, (int32_t)r);
            }
        } else if (r == (int32_t)r) {
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
        double r = (double)a / (double)b;
        if (r == (int32_t)r && !(r == 0 && signbit(r))) {
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

struct BinaryOpMod {
    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        if (b == 0) {
            return jsValueNaN;
        }

        uint32_t r = a % b;
        if (r == 0) {
            auto r = fmod((double)a, (double)b);
            if (signbit(r)) {
                return rt->pushDoubleValue(r);
            }
        }
        return JsValue(JDT_INT32, (int32_t)r);
    }

    JsValue operator()(VMRuntime *rt, double a, int32_t b) const {
        auto r = fmod(a, (double)b);
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double b) const {
        auto r = fmod((double)a, b);
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, double a, double b) const {
        auto r = fmod(a, b);
        return rt->pushDoubleValue(r);
    }
};

struct BinaryOpExp {
    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        double r = pow(a, b);
        if (r == (int32_t)r) {
            return JsValue(JDT_INT32, (int32_t)r);
        }
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, double a, int32_t b) const {
        double r = pow(a, (double)b);
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double b) const {
        if (isnan(b) || (isinf(b) && (a == 1 || a == -1))) {
            // 1 ** Infinity
            return jsValueNaN;
        }

        auto r = pow((double)a, b);
        return rt->pushDoubleValue(r);
    }

    JsValue operator()(VMRuntime *rt, double a, double b) const {
        if (isnan(b) || (isinf(b) && (a == 1 || a == -1))) {
            // 1 ** Infinity
            return jsValueNaN;
        }

        auto r = pow(a, b);
        return rt->pushDoubleValue(r);
    }
};

inline int32_t doubleToInt32(double v) {
    return isnormal(v) ? (int64_t(v) & 0xffffffff) : 0;
}

struct BinaryOpBitXor {

    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        auto r = a ^ b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, int32_t b) const {
        auto a = doubleToInt32(da);
        auto r = a ^ b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double db) const {
        auto b = doubleToInt32(db);
        auto r = a ^ b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, double db) const {
        auto a = doubleToInt32(da);
        auto b = doubleToInt32(db);
        auto r = a ^ b;
        return JsValue(JDT_INT32, r);
    }
};

struct BinaryOpBitOr {

    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        auto r = a | b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, int32_t b) const {
        auto a = doubleToInt32(da);
        auto r = a | b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double db) const {
        auto b = doubleToInt32(db);
        auto r = a | b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, double db) const {
        auto a = doubleToInt32(da);
        auto b = doubleToInt32(db);
        auto r = a | b;
        return JsValue(JDT_INT32, r);
    }
};

struct BinaryOpBitAnd {

    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        auto r = a & b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, int32_t b) const {
        auto a = doubleToInt32(da);
        auto r = a & b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double db) const {
        auto b = doubleToInt32(db);
        auto r = a & b;
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, double db) const {
        auto a = doubleToInt32(da);
        auto b = doubleToInt32(db);
        auto r = a & b;
        return JsValue(JDT_INT32, r);
    }
};

inline int32_t roundShiftSecondOp(int32_t b) {
    // 在 JavaScript 中，移位数量要转换为 unsigned，再对 32 取模.
    return (int32_t(b) & 0xffffffff) % 32;
}

struct BinaryOpLeftShift {

    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        auto r = a << roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, int32_t b) const {
        auto a = doubleToInt32(da);
        auto r = a << roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double db) const {
        auto b = doubleToInt32(db);
        auto r = a << roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, double db) const {
        auto a = doubleToInt32(da);
        auto b = doubleToInt32(db);
        auto r = a << roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }
};

struct BinaryOpRightShift {

    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        auto r = a >> roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, int32_t b) const {
        auto a = doubleToInt32(da);
        auto r = a >> roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double db) const {
        auto b = doubleToInt32(db);
        auto r = a >> roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }

    JsValue operator()(VMRuntime *rt, double da, double db) const {
        auto a = doubleToInt32(da);
        auto b = doubleToInt32(db);
        auto r = a >> roundShiftSecondOp(b);
        return JsValue(JDT_INT32, r);
    }
};

struct BinaryOpUnsignedRightShift {

    inline JsValue rightShift(VMRuntime *rt, uint32_t a, uint32_t b) const {
        int64_t r = a >> roundShiftSecondOp(b);
        if (r == (int32_t)r) {
            return JsValue(JDT_INT32, (int32_t)r);
        } else {
            return rt->pushDoubleValue(r);
        }
    }

    JsValue operator()(VMRuntime *rt, int32_t a, int32_t b) const {
        return rightShift(rt, a, b);
    }

    JsValue operator()(VMRuntime *rt, double da, int32_t b) const {
        auto a = doubleToInt32(da);
        return rightShift(rt, uint32_t(a), b);
    }

    JsValue operator()(VMRuntime *rt, int32_t a, double db) const {
        auto b = doubleToInt32(db);
        return rightShift(rt, a, uint32_t(b));
    }

    JsValue operator()(VMRuntime *rt, double da, double db) const {
        auto a = doubleToInt32(da);
        auto b = doubleToInt32(db);
        return rightShift(rt, uint32_t(a), uint32_t(b));
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
            throwSymbolConvertException(ctx);
            return jsValueNaN;
        case JDT_CHAR: {
            SizedStringWrapper str;
            if (leftStr.isValid()) {
                str.append(rt->getUtf8String(leftStr));
            } else {
                str.append(left);
            }

            str.append(right);
            return rt->pushString(str.str());
        }
        case JDT_STRING: {
            if (leftStr.isValid()) {
                return rt->plusString(leftStr, right);
            } else {
                SizedStringWrapper s(left);
                return rt->plusString(s.str(), right);
            }
        }
        default: {
            return plusOperate(ctx, rt, left, leftStr, rt->tryCallJsObjectValueOf(ctx, right));
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
                    throwSymbolConvertException(ctx);
                    return jsValueNaN;
                case JDT_CHAR: {
                    SizedStringWrapper str(SS_UNDEFINED);
                    str.append(right);
                    return rt->pushString(str.str());
                }
                case JDT_STRING:
                    return rt->plusString(jsStringValueUndefined, right);
                default: {
                    return plusOperate(ctx, rt, left, rt->tryCallJsObjectValueOf(ctx, right));
                }
            }
            break;
        case JDT_NULL:
            return plusOperate(ctx, rt, 0, jsStringValueNull, right);
        case JDT_BOOL:
            return plusOperate(ctx, rt, left.value.n32, left.value.n32 ? jsStringValueTrue : jsStringValueFalse, right);
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
                    throwSymbolConvertException(ctx);
                    return jsValueNaN;
                case JDT_CHAR:
                case JDT_STRING: {
                    SizedStringWrapper s(n);
                    return rt->plusString(s, right);
                }
                default:
                    return plusOperate(ctx, rt, left, rt->tryCallJsObjectValueOf(ctx, right));
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
                    return rt->plusString(left, right);
                }
                case JDT_NUMBER: {
                    SizedStringWrapper str(left);
                    str.append(rt->getDouble(right));
                    return rt->pushString(str.str());
                }
                case JDT_SYMBOL:
                    throwSymbolConvertStringException(ctx);
                    return jsValueNaN;
                default:
                    return plusOperate(ctx, rt, left, rt->tryCallJsObjectValueOf(ctx, right));
            }
            break;
        }
        case JDT_STRING: {
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                    return rt->plusString(left, jsStringValueUndefined);
                case JDT_NULL:
                    return rt->plusString(left, jsStringValueNull);
                case JDT_BOOL:
                    return rt->plusString(left, right.value.n32 ? jsStringValueTrue : jsStringValueFalse);
                case JDT_INT32: {
                    SizedStringWrapper str(right.value.n32);
                    return rt->plusString(left, str);
                }
                case JDT_CHAR: {
                    return rt->plusString(left, right);
                }
                case JDT_STRING: {
                    return rt->plusString(left, right);
                }
                case JDT_NUMBER: {
                    SizedStringWrapper str(rt->getDouble(right));
                    return rt->plusString(left, str);
                }
                case JDT_SYMBOL:
                    throwSymbolConvertStringException(ctx);
                    return jsValueNaN;
                default:
                    return plusOperate(ctx, rt, left, rt->tryCallJsObjectValueOf(ctx, right));
            }
            break;
        }
        case JDT_SYMBOL: {
            throwSymbolConvertException(ctx, left.isNumber());
            return jsValueNaN;
        }
        default:
            return plusOperate(ctx, rt, rt->tryCallJsObjectValueOf(ctx, left), right);
    }
}

// <
struct RelationalOpLessThan {
    bool undefinedVsNullUndefined() const { return false; }
    bool nullVsUndefined() const { return false; }
    bool nullVsNull() const { return false; }
    bool nullVsNumber(double b) const { return 0 < b; }
    bool nullVsSymbol(VMContext *ctx) const { return throwSymbolConvertException(ctx); }
    bool numberVsNull(double a) const { return a < 0; }

    bool symbolVsSymbol(VMContext *ctx, int32_t a, int32_t b) const { return throwSymbolConvertException(ctx); }
    bool symbolVsOthers(VMContext *ctx) const { return throwSymbolConvertException(ctx); }

    bool operator()(double a, double b) const {
        return a < b;
    }

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) < 0;
    }
};

// <=
struct RelationalOpLessEqThan {
    bool undefinedVsNullUndefined() const { return false; }
    bool nullVsUndefined() const { return false; }
    bool nullVsNull() const { return true; }
    bool nullVsNumber(double b) const { return 0 <= b; }
    bool nullVsSymbol(VMContext *ctx) const { return throwSymbolConvertException(ctx); }
    bool numberVsNull(double a) const { return a <= 0; }

    bool symbolVsSymbol(VMContext *ctx, int32_t a, int32_t b) const { return throwSymbolConvertException(ctx); }
    bool symbolVsOthers(VMContext *ctx) const { return throwSymbolConvertException(ctx); }

    bool operator()(double a, double b) const {
        return a <= b;
    }

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) <= 0;
    }
};

// ==
struct RelationalOpEq {
    bool undefinedVsNullUndefined() const { return true; }
    bool nullVsUndefined() const { return true; }
    bool nullVsNull() const { return true; }
    bool nullVsNumber(double b) const { return false; }
    bool nullVsSymbol(VMContext *) const { return false; }
    bool numberVsNull(double a) const { return false; }

    bool symbolVsSymbol(VMContext *ctx, int32_t a, int32_t b) const { return false; }
    bool symbolVsOthers(VMContext *ctx) const { return false; }

    bool operator()(double a, double b) const {
        return a == b;
    }

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) == 0;
    }
};

// >
struct RelationalOpGreaterThan {
    bool undefinedVsNullUndefined() const { return false; }
    bool nullVsUndefined() const { return false; }
    bool nullVsNull() const { return false; }
    bool nullVsNumber(double b) const { return 0 > b; }
    bool nullVsSymbol(VMContext *ctx) const { return throwSymbolConvertException(ctx); }
    bool numberVsNull(double a) const { return a > 0; }

    bool symbolVsSymbol(VMContext *ctx, int32_t a, int32_t b) const { return throwSymbolConvertException(ctx); }
    bool symbolVsOthers(VMContext *ctx) const { return throwSymbolConvertException(ctx); }

    bool operator()(double a, double b) const {
        return a > b;
    }

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) > 0;
    }
};

// >=
struct RelationalOpGreaterEqThan {
    bool undefinedVsNullUndefined() const { return false; }
    bool nullVsUndefined() const { return false; }
    bool nullVsNull() const { return true; }
    bool nullVsNumber(double b) const { return 0 >= b; }
    bool nullVsSymbol(VMContext *ctx) const { return throwSymbolConvertException(ctx); }
    bool numberVsNull(double a) const { return a >= 0; }

    bool symbolVsSymbol(VMContext *ctx, int32_t a, int32_t b) const { return throwSymbolConvertException(ctx); }
    bool symbolVsOthers(VMContext *ctx) const { return throwSymbolConvertException(ctx); }

    bool operator()(double a, double b) const {
        return a >= b;
    }

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) >= 0;
    }
};

// double <, <=, ==, >, >= 运算
template<typename Operator>
inline bool relationalNumberCmp(VMContext *ctx, VMRuntime *rt, double left, const JsValue &right, const Operator &op) {
    switch (right.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            return false;
        case JDT_NULL:
            return op.numberVsNull(left);
        case JDT_BOOL:
        case JDT_INT32:
            return op(left, right.value.n32);
        case JDT_NUMBER:
            return op(left, rt->getDouble(right));
        case JDT_SYMBOL:
            return op.symbolVsOthers(ctx);
        default:
            // 转换为 number 进行比较
            double n;
            if (rt->toNumber(ctx, right, n)) {
                return op(left, n);
            }
            return false;
    }
}

// string <, <=, ==, >, >= 运算
template<typename Operator>
inline bool relationalStringCmp(VMContext *ctx, VMRuntime *rt, const SizedString &left, const JsValue &right, const Operator &op) {
    switch (right.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            return false;
        case JDT_NULL: {
            // 转换为 number 进行比较
            double n;
            if (jsStringToNumber(left, n)) {
                return op.numberVsNull(n);
            }
            return false;
        }
        case JDT_BOOL:
        case JDT_INT32: {
            double n;
            if (jsStringToNumber(left, n)) {
                return op(n, right.value.n32);
            }
            return false;
        }
        case JDT_NUMBER: {
            double n;
            if (jsStringToNumber(left, n)) {
                return op(n, rt->getDouble(right));
            }
            return false;
        }
        case JDT_SYMBOL:
            return op.symbolVsOthers(ctx);
        case JDT_CHAR: {
            SizedStringWrapper tmp(right);
            return op(left, tmp.str());
        }
        case JDT_STRING: {
            return op(left, rt->getUtf8String(right));
        }
        default: {
            // Object 类型需要再次转换
            return relationalStringCmp(ctx, rt, left, rt->tryCallJsObjectValueOf(ctx, right), op);
        }
    }
}

// <, <=, ==, >, >= 运算
template<typename Operator>
inline bool relationalOperate(VMContext *ctx, VMRuntime *rt, const JsValue &left, const JsValue &right, const Operator &op) {
    switch (left.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            if (right.type <= JDT_NULL) return op.undefinedVsNullUndefined();
            if (right.type == JDT_SYMBOL) return op.nullVsSymbol(ctx);
            return false;
        case JDT_NULL:
            switch (right.type) {
                case JDT_NOT_INITIALIZED:
                case JDT_UNDEFINED:
                    return op.nullVsUndefined();
                case JDT_NULL:
                    return op.nullVsNull();
                case JDT_BOOL:
                case JDT_INT32:
                    return op.nullVsNumber(right.value.n32);
                case JDT_NUMBER:
                    return op.nullVsNumber(rt->getDouble(right));
                case JDT_SYMBOL:
                    return op.nullVsSymbol(ctx);
                default:
                    // 转换为 number 进行比较
                    double n;
                    if (rt->toNumber(ctx, right, n)) {
                        return op.nullVsNumber(n);
                    }
                    return false;
            }
            return false;
        case JDT_BOOL:
        case JDT_INT32:
            return relationalNumberCmp(ctx, rt, left.value.n32, right, op);
        case JDT_NUMBER:
            return relationalNumberCmp(ctx, rt, rt->getDouble(left), right, op);
        case JDT_CHAR: {
            SizedStringWrapper str(left);
            return relationalStringCmp(ctx, rt, str, right, op);
        }
        case JDT_STRING: {
            auto &str = rt->getUtf8String(left);
            return relationalStringCmp(ctx, rt, str, right, op);
        }
        case JDT_SYMBOL: {
            if (right.type == JDT_SYMBOL) {
                return op.symbolVsSymbol(ctx, left.value.n32, right.value.n32);
            }

            return op.symbolVsOthers(ctx);
        }
        default: {
            // Object 类型需要再次转换
            return relationalOperate(ctx, rt, rt->tryCallJsObjectValueOf(ctx, left), right, op);
        }
    }
}

inline bool relationalEqual(VMContext *ctx, VMRuntime *rt, const JsValue &left, const JsValue &right) {
    if (left.equal(right)) {
        // TODO: 需要检查 JsValue 在 runtime->doubleValues 的值是否为 NaN
        return !left.equal(jsValueNaN);
    } else {
        // 需要特别判断 Object 的情况
        if (left.type >= JDT_OBJECT && right.type >= JDT_OBJECT) {
            return false;
        }

        return relationalOperate(ctx, rt, left, right, RelationalOpEq());
    }

}

inline bool relationalStrictEqual(VMRuntime *rt, const JsValue &left, const JsValue &right) {
    if (left.equal(right) && !left.equal(jsValueNaN)) {
        return true;
    }

    if (left.type == JDT_CHAR) {
        if (right.type == JDT_STRING && rt->getStringLength(right) == 1) {
            auto &s = rt->getString(right);
            return s.equal(left.value.index);
        }
    } else if (left.type == JDT_STRING) {
        auto len1 = rt->getStringLength(left);
        if (right.type == JDT_CHAR) {
            if (len1 == 1) {
                auto &s1 = rt->getString(left);
                return s1.equal(right.value.n32);
            }
        } else if (right.type == JDT_STRING) {
            auto len2 = rt->getStringLength(right);
            if (len1 == len2) {
                auto &s1 = rt->getUtf8String(left);
                auto &s2 = rt->getUtf8String(right);
                return s1.equal(s2);
            }
        }
    } else if (left.type == JDT_INT32) {
        if (right.type == JDT_NUMBER) {
            auto d = rt->getDouble(right);
            return left.value.n32 == d;
        }
    } else if (left.type == JDT_NUMBER) {
        if (right.type == JDT_INT32) {
            auto d1 = rt->getDouble(left);
            return d1 == right.value.n32;
        } else if (right.type == JDT_NUMBER) {
            auto d1 = rt->getDouble(left);
            auto d2 = rt->getDouble(right);
            return d1 == d2;
        }
    }

    return false;
}

inline void assignMemberIndexOperation(VMContext *ctx, VMRuntime *runtime, const JsValue &obj, JsValue index, const JsValue &value) {
    if (obj.type < JDT_OBJECT) {
        // Primitive 类型都不能设置属性
        return;
    }

    if (index.type >= JDT_OBJECT) {
        ctx->vm->callMember(ctx, obj, SS_TOSTRING, Arguments());
        if (ctx->error != PE_OK) {
            return;
        }
        index = ctx->retValue;
    }

    auto pobj = runtime->getObject(obj);
    pobj->set(ctx, obj, index, value);
}

inline bool instanceOf(VMContext *ctx, VMRuntime *rt, const JsValue &left, const JsValue &right) {
    if (right.type < JDT_OBJECT) {
        ctx->throwException(PE_TYPE_ERROR, "Right-hand side of 'instanceof' is not an object");
        return false;
    }

    if (right.type < JDT_FUNCTION) {
        ctx->throwException(PE_TYPE_ERROR, "Right-hand side of 'instanceof' is not callable");
        return false;
    }

    if (left.type < JDT_OBJECT) {
        return false;
    }

    auto prototype = rt->getObject(right)->getByName(ctx, right, SS_PROTOTYPE);
    if (prototype.type < JDT_OBJECT) {
        return false;
    }

    auto proto = left;

    do {
        auto obj = rt->getObject(proto);
        proto = obj->getByName(ctx, proto, SS___PROTO__);
        if (proto == prototype) {
            return true;
        }
    } while (proto.type >= JDT_OBJECT);

    return false;
}

inline bool convertToStringLessCmp(VMContext *ctx, JsValue left, JsValue right) {
    auto rt = ctx->runtime;
    auto s1 = rt->toSizedString(ctx, left);
    auto s2 = rt->toSizedString(ctx, right);

    return s1.cmp(s2) < 0;
}

#endif /* BinaryOperation_hpp */
