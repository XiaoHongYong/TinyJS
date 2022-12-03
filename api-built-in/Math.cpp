//
//  Math.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/11/19.
//

#include <math.h>
#include "BuiltIn.hpp"
#include "objects/JsObjectFunction.hpp"
#include "objects/JsArray.hpp"
#include "objects/JsArguments.hpp"


// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math

void math_abs(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto value = args.getAt(0);
    auto d = runtime->toNumber(ctx, value);
    if (value.type == JDT_NUMBER || value.type == JDT_INT32) {
        if (d >= 0) {
            ctx->retValue = value;
        } else if (value.type == JDT_INT32) {
            ctx->retValue = JsValue(JDT_INT32, -value.value.n32);
        } else {
            // double
            ctx->retValue = runtime->pushDoubleValue(-d);
        }
    } else {
        if (isnan(d)) {
            ctx->retValue = jsValueNaN;
        } else {
            if (d < 0) {
                d = -d;
            }

            if (d == (int32_t)d) {
                ctx->retValue = JsValue(JDT_INT32, (int32_t)d);
            } else {
                ctx->retValue = runtime->pushDoubleValue(d);
            }
        }
    }
}

void math_acos(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_acosh(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_asin(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_asinh(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_atan(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_atan2(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_atanh(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_cbrt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_ceil(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto value = args.getAt(0);
    if (value.type == JDT_INT32) {
        ctx->retValue = value;
        return;
    }

    auto d = runtime->toNumber(ctx, value);
    if (isnan(d)) {
        ctx->retValue = jsValueNaN;
    } else if (isinf(d)) {
        if (signbit(d)) {
            ctx->retValue = jsValueNegInf;
        } else {
            ctx->retValue = jsValueInf;
        }
    } else {
        auto n = (int32_t)d;
        if (n == (int64_t)d) {
            ctx->retValue = JsValue(JDT_INT32, n);
        } else {
            auto n = (int64_t)d;
            ctx->retValue = runtime->pushDoubleValue(n);
        }
    }
}

void math_clz32(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_cos(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_cosh(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_exp(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_expm1(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_floor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto value = args.getAt(0);
    if (value.type == JDT_INT32) {
        ctx->retValue = value;
        return;
    }

    auto d = floor(runtime->toNumber(ctx, value));
    if (isnan(d)) {
        ctx->retValue = jsValueNaN;
    } else if (isinf(d)) {
        if (signbit(d)) {
            ctx->retValue = jsValueNegInf;
        } else {
            ctx->retValue = jsValueInf;
        }
    } else {
        auto n = (int32_t)d;
        if (n == (int64_t)d) {
            ctx->retValue = JsValue(JDT_INT32, n);
        } else {
            ctx->retValue = runtime->pushDoubleValue(d);
        }
    }
}

void math_fround(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_hypot(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_imul(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_log(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_log10(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_log1p(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_log2(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_max(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto v1 = args.getAt(0);
    auto v2 = args.getAt(1);
    if (v1.type == JDT_INT32 && v2.type == JDT_INT32) {
        ctx->retValue = JsValue(JDT_INT32, max(v1.value.n32, v2.value.n32));
        return;
    }

    auto d1 = runtime->toNumber(ctx, v1);
    auto d2 = runtime->toNumber(ctx, v2);
    if (d1 >= d2) {
        if (v1.isNumber()) {
            ctx->retValue = v1;
        } else {
            ctx->retValue = runtime->pushDoubleValue(d1);
        }
    } else {
        if (v2.isNumber()) {
            ctx->retValue = v2;
        } else {
            ctx->retValue = runtime->pushDoubleValue(d2);
        }
    }
}

void math_min(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto v1 = args.getAt(0);
    auto v2 = args.getAt(1);
    if (v1.type == JDT_INT32 && v2.type == JDT_INT32) {
        ctx->retValue = JsValue(JDT_INT32, min(v1.value.n32, v2.value.n32));
        return;
    }

    auto d1 = runtime->toNumber(ctx, v1);
    auto d2 = runtime->toNumber(ctx, v2);
    if (d1 <= d2) {
        if (v1.isNumber()) {
            ctx->retValue = v1;
        } else {
            ctx->retValue = runtime->pushDoubleValue(d1);
        }
    } else {
        if (v2.isNumber()) {
            ctx->retValue = v2;
        } else {
            ctx->retValue = runtime->pushDoubleValue(d2);
        }
    }
}

void math_pow(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto v1 = args.getAt(0);
    auto v2 = args.getAt(1);
    auto d1 = runtime->toNumber(ctx, v1);
    auto d2 = runtime->toNumber(ctx, v2);

    auto r = pow(d1, d2);
    if (r == (int32_t)r) {
        ctx->retValue = JsValue(JDT_INT32, r);
    } else {
        ctx->retValue = runtime->pushDoubleValue(r);
    }
}

void math_random(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    double r = rand() / (double)RAND_MAX;
    ctx->retValue = ctx->runtime->pushDoubleValue(r);
}

void math_round(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto value = args.getAt(0);
    if (value.type == JDT_INT32) {
        ctx->retValue = value;
        return;
    }

    auto d = round(runtime->toNumber(ctx, value));
    if (isnan(d)) {
        ctx->retValue = jsValueNaN;
    } else if (isinf(d)) {
        if (signbit(d)) {
            ctx->retValue = jsValueNegInf;
        } else {
            ctx->retValue = jsValueInf;
        }
    } else {
        auto n = (int32_t)d;
        if (n == (int64_t)d) {
            ctx->retValue = JsValue(JDT_INT32, n);
        } else {
            ctx->retValue = runtime->pushDoubleValue(d);
        }
    }
}

void math_sign(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto value = args.getAt(0);
    auto d = runtime->toNumber(ctx, value);
    if (isnan(d)) {
        ctx->retValue = jsValueNaN;
    } else if (d == 0) {
        ctx->retValue = JsValue(JDT_INT32, 0);
    } else {
        ctx->retValue = JsValue(JDT_INT32, signbit(d) ? -1 : 1);
    }
}

void math_sin(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_sinh(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_sqrt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto value = args.getAt(0);
    auto d = sqrt(runtime->toNumber(ctx, value));
    if (isnan(d)) {
        ctx->retValue = jsValueNaN;
    } else if (isinf(d)) {
        if (signbit(d)) {
            ctx->retValue = jsValueNegInf;
        } else {
            ctx->retValue = jsValueInf;
        }
    } else {
        ctx->retValue = runtime->pushDoubleValue(d);
    }
}

void math_tan(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_tanh(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void math_trunc(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    assert(0);
    ctx->retValue = jsValueUndefined;
}


static JsLibProperty mathFunctions[] = {
    { "name", nullptr, "Math" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "abs", math_abs, },
    { "acos", math_acos, },
    { "acosh", math_acosh, },
    { "asin", math_asin, },
    { "asinh", math_asinh, },
    { "atan", math_atan, },
    { "atan2", math_atan2, },
    { "atanh", math_atanh, },
    { "cbrt", math_cbrt, },
    { "ceil", math_ceil, },
    { "clz32", math_clz32, },
    { "cos", math_cos, },
    { "cosh", math_cosh, },
    { "exp", math_exp, },
    { "expm1", math_expm1, },
    { "floor", math_floor, },
    { "fround", math_fround, },
    { "hypot", math_hypot, },
    { "imul", math_imul, },
    { "log", math_log, },
    { "log10", math_log10, },
    { "log1p", math_log1p, },
    { "log2", math_log2, },
    { "max", math_max, },
    { "min", math_min, },
    { "pow", math_pow, },
    { "random", math_random, },
    { "round", math_round, },
    { "sign", math_sign, },
    { "sin", math_sin, },
    { "sinh", math_sinh, },
    { "sqrt", math_sqrt, },
    { "tan", math_tan, },
    { "tanh", math_tanh, },
    { "trunc", math_trunc, },
};

void registerObjMath(VMRuntimeCommon *rt) {
    setGlobalLibObject("Math", rt, mathFunctions, CountOf(mathFunctions));
}
