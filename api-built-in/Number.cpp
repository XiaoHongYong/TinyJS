﻿//
//  Number.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"
#include "utils/StringEx.h"


void numberConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue value;
    if (args.count == 0) {
        value = makeJsValueInt32(0);
    } else {
        value = args[0];
        if (value.type != JDT_NUMBER && value.type == JDT_INT32) {
            auto runtime = ctx->runtime;
            auto n = runtime->toNumber(ctx, value);
            if (n == (int32_t)n) {
                value = makeJsValueInt32(n);
            } else {
                value = runtime->pushDouble(n);
            }
        }
    }

    if (thiz.isEmpty()) {
        // New
        auto obj = new JsNumberObject(value);
        ctx->retValue = ctx->runtime->pushObject(obj);
    } else {
        ctx->retValue = value;
    }
}


void numberIsFinite(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto v = args.getAt(0);
    if (v.type == JDT_NUMBER) {
        auto d = ctx->runtime->getDouble(v);
        ctx->retValue = makeJsValueBool(!isnan(d) && !isinf(d));
    } else if (v.type == JDT_INT32) {
        ctx->retValue = jsValueTrue;
    } else {
        ctx->retValue = jsValueFalse;
    }
}

void numberIsInteger(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto v = args.getAt(0);
    if (v.type == JDT_NUMBER) {
        auto d = ctx->runtime->getDouble(v);
        ctx->retValue = makeJsValueBool(!isinf(d) && (d == (int64_t)d || abs(d) >= 9007199254740992L)); // 2 ** 53
    } else if (v.type == JDT_INT32) {
        ctx->retValue = jsValueTrue;
    } else {
        ctx->retValue = jsValueFalse;
    }
}

void numberIsSafeInteger(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto v = args.getAt(0);
    if (v.type == JDT_NUMBER) {
        auto d = ctx->runtime->getDouble(v);
        ctx->retValue = makeJsValueBool(d == (int64_t)d && d < 9007199254740992L); // 2 ** 53
    } else if (v.type == JDT_INT32) {
        ctx->retValue = jsValueTrue;
    } else {
        ctx->retValue = jsValueFalse;
    }
}

void numberParseFloat(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto v = args.getAt(0);
    while (true) {
        if (v.type == JDT_CHAR) {
            if (isDigit(v.value.n32)) {
                ctx->retValue = makeJsValueInt32(v.value.n32 - '0');
            } else {
                ctx->retValue = jsValueNaN;
            }
        } else if (v.type == JDT_STRING) {
            auto str = ctx->runtime->getString(v).utf8Str().trimStart(stringViewBlanks);
            double d = NAN;
            bool negative = false;

            if (str.len > 0) {
                if (str.data[0] == '-') {
                    negative = true;
                    str.data++; str.len--;
                } else if (str.data[0] == '+') {
                    str.data++; str.len--;
                }
            }

            if (str.startsWith(SS_INFINITY)) {
                d = INFINITY;
            } else if (str.startsWith(SS_INFINITY)) {
                d = NAN;
            } else {
                for (int i = 0; i < str.len; i++) {
                    auto c = str.data[i];
                    if (!isDigit(c) && c != 'e' && c != 'E' && c != '.'  && c != '-' && c != '+') {
                        // parseNumber 不支持 0xff, 0o123 等的解析，所以提前截断需要解析的字符串
                        str.len = i;
                        break;
                    }
                }

                auto p = parseNumber(str, d);
                if (p == str.data) {
                    d = NAN;
                }
            }

            if (negative) {
                d = -d;
            }

            int32_t n = (int32_t)d;
            if (n == d) {
                ctx->retValue = makeJsValueInt32(n);
            } else if (isnan(d)) {
                ctx->retValue = jsValueNaN;
            } else {
                ctx->retValue = ctx->runtime->pushDouble(d);
            }
        } else if (v.isNumber()) {
            ctx->retValue = v;
        } else if (v.type == JDT_SYMBOL) {
            // 抛出异常
            ctx->runtime->toStringViewStrictly(ctx, v);
        } else if (v.type >= JDT_OBJECT) {
            v = ctx->runtime->toString(ctx, v);
            continue;
        } else {
            ctx->retValue = jsValueNaN;
        }
        return;
    }
}

double parseInt(char *start, char *end, int base = -1) {
    if (start >= end || base > 36) {
        return NAN;
    }

    int sign = 1;
    if (*start == '-') {
        sign = -1;
        start++;
    } else if (*start == '+') {
        start++;
    }

    auto orgStart = start;
    if (*start == '0' && base == -1) {
        start++;
        auto ch = *start;
        if ((ch == 'x' || ch == 'X') && start + 1 < end) {
            // Read hex numbers
            start++;
            if (base != -1 && base != 16) {
                return 0;
            }
            base = 16;
        }
    }

    if (base == -1) {
        base = 10;
    }

    uint64_t n = 0, exp = 0, maxN = ((uint64_t)-1) / base - base;
    while (start < end) {
        auto ch = *start;
        int x = toDigit(ch);
        if (x >= base) {
            break;
        }

        start++;
        uint64_t t = n * base + x;
        if (t > maxN || exp > 0) {
            // 溢出了
            exp++;
        } else {
            n = t;
        }
    }

    if (orgStart == start) {
        return NAN;
    }

    double value = n;
    if (exp > 0) {
        // 溢出了
        value *= pow(base, exp);
    }

    return value * sign;
}

void numberParseInt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto v = args.getAt(0);
    auto base = args.getIntAt(ctx, 1, -1);

    auto str = runtime->toStringViewStrictly(ctx, v).trimStart(stringViewBlanks);

    double d = parseInt(str.data, str.data + str.len, base);
    int32_t n = (int32_t)d;
    if (n == d) {
        ctx->retValue = makeJsValueInt32(n);
    } else if (isnan(d)) {
        ctx->retValue = jsValueNaN;
    } else {
        ctx->retValue = ctx->runtime->pushDouble(d);
    }
}

void numberIsNaN(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto v = args.getAt(0);
    if (v.type == JDT_NUMBER) {
        auto d = ctx->runtime->getDouble(v);
        ctx->retValue = makeJsValueBool(isnan(d));
    } else {
        ctx->retValue = jsValueFalse;
    }
}

static JsLibProperty numberFunctions[] = {
    { "NaN", nullptr, nullptr, jsValueNaN },
    { "EPSILON", nullptr, nullptr, jsValueEpsilonNumber },
    { "MAX_SAFE_INTEGER", nullptr, nullptr, jsValueMaxSafeInt },
    { "MAX_VALUE", nullptr, nullptr, jsValueMaxNumber },
    { "MIN_SAFE_INTEGER", nullptr, nullptr, jsValueMinSafeInt },
    { "MIN_VALUE", nullptr, nullptr, jsValueMinNumber },
    { "NEGATIVE_INFINITY", nullptr, nullptr, jsValueNegInf },
    { "POSITIVE_INFINITY", nullptr, nullptr, jsValueInf },

    { "isFinite", numberIsFinite },
    { "isInteger", numberIsInteger },
    { "isNaN", numberIsNaN },
    { "isSafeInteger", numberIsSafeInteger, },
    { "parseFloat", numberParseFloat, },
    { "parseInt", numberParseInt, },
    { "name", nullptr, "Number" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};


inline JsValue convertNumberToJsValue(VMContext *ctx, const JsValue &thiz, const char *funcName) {
    if (thiz.type == JDT_NUMBER || thiz.type == JDT_INT32) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_NUMBER) {
        auto obj = (JsNumberObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(JE_TYPE_ERROR, "Number.prototype.%s requires that 'this' be a Number", funcName);
    return jsValueEmpty;
}

void numberPrototypeToExponential(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toExponential");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto d = runtime->toNumber(ctx, value);
    uint32_t size = 0;
    char buf[256];

    if (args.getAt(0).type == JDT_UNDEFINED) {
        size = floatToStringEx(d, buf, sizeof(buf), -1, F_TRIM_TAILING_ZERO | F_EXPONENTIAL_NOTATION);
    } else {
        auto fractionDigits = args.getIntAt(ctx, 0);
        if (fractionDigits < 0 || fractionDigits > 100) {
            ctx->throwException(JE_RANGE_ERROR, "toExponential() argument must be between 0 and 100");
            return;
        }

        size = floatToStringEx(d, buf, sizeof(buf), fractionDigits + 1, F_EXPONENTIAL_NOTATION);
    }

    ctx->retValue = runtime->pushString(StringView(buf, size));
}

void numberPrototypeToFixed(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toFixed");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto d = runtime->toNumber(ctx, value);
    uint32_t size = 0;
    char buf[4096];

    auto precision = args.getIntAt(ctx, 0, 0);
    if (precision < 0 || precision > 100) {
        ctx->throwException(JE_RANGE_ERROR, "toFixed() digits argument must be between 0 and 100");
        return;
    }

    if (abs(d) < 1e21) {
        size = floatToStringEx(d, buf, sizeof(buf), precision + 1, F_FIXED_DIGITS);
    } else {
        size = floatToString(d, buf);
    }

    ctx->retValue = runtime->pushString(StringView(buf, size));
}

void numberPrototypeToPrecision(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toPrecision");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto d = runtime->toNumber(ctx, value);

    uint32_t size = 0;
    char buf[256];
    if (args.getAt(0).type == JDT_UNDEFINED) {
        size = floatToStringEx(d, buf, sizeof(buf), -1, F_TRIM_TAILING_ZERO);
    } else {
        auto precision = args.getIntAt(ctx, 0);
        if (precision <= 0 || precision > 100) {
            ctx->throwException(JE_RANGE_ERROR, "toPrecision() argument must be between 1 and 100");
            return;
        }

        size = floatToStringEx(d, buf, sizeof(buf), precision, F_FIXED_PRECISION);
    }

    ctx->retValue = runtime->pushString(StringView(buf, size));
}

void numberPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toString");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;

    int radix = 10;
    if (args.count > 0) {
        radix = (int)runtime->toNumber(ctx, args[0]);
        if (radix < 2 || radix > 36) {
            ctx->throwException(JE_RANGE_ERROR, "toString() radix argument must be between 2 and 36");
            return;
        }
    }

    char buf[2048];
    size_t size;
    if (value.type == JDT_INT32) {
        size = itoa_ex(value.value.n32, buf, radix);
    } else {
        auto v = runtime->getDouble(value);
        size = floatToStringWithRadix(v, buf, sizeof(buf), radix);
    }

    ctx->retValue = runtime->pushString(StringView(buf, size));
}

void numberPrototypeValueOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = convertNumberToJsValue(ctx, thiz, "valueOf");
}

static JsLibProperty numberPrototypeFunctions[] = {
    { "toExponential", numberPrototypeToExponential },
    { "toFixed", numberPrototypeToFixed },
    { "toLocaleString()", numberPrototypeToString },
    { "toPrecision", numberPrototypeToPrecision },
    { "toString", numberPrototypeToString },
    { "valueOf", numberPrototypeValueOf },
};

void registerNumber(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, numberPrototypeFunctions, CountOf(numberPrototypeFunctions));
    rt->objPrototypeNumber = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeNumber, prototypeObj);

    SET_PROTOTYPE(numberFunctions, jsValuePrototypeNumber);

    auto obj = setGlobalLibObject("Number", rt, numberFunctions, CountOf(numberFunctions), numberConstructor, jsValuePrototypeFunction);

    rt->setGlobalValue("isFinite", obj->getByName(nullptr, obj->self, StringView("isFinite")));
    rt->setGlobalValue("isNaN", obj->getByName(nullptr, obj->self, StringView("isNaN")));
    rt->setGlobalValue("parseInt", obj->getByName(nullptr, obj->self, StringView("parseInt")));
    rt->setGlobalValue("parseFloat", obj->getByName(nullptr, obj->self, StringView("parseFloat")));
}
