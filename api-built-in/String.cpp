//
//  String.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"


static void stringConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    JsValue str = args.count > 0 ? runtime->toString(ctx, args[0]) : jsStringValueEmpty;

    if (thiz.type != JDT_NOT_INITIALIZED) {
        ctx->retValue = str;
        return;
    }

    // New
    auto obj = new JsStringObject(str);
    ctx->retValue = runtime->pushObjectValue(obj);
}

void stringFromCharCode(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    string str;
    for (uint32_t i = 0; i < args.count; i++) {
        auto &item = args.data[i];
        int32_t code = item.type == JDT_INT32 ? item.value.n32 : (int32_t)runtime->toNumber(ctx, item);

        // 按照数据类型都完全正确的情况处理
        if (code <= 0xFFFF) {
            utf32CodeToUtf8((uint16_t)code, str);
        }
    }

    auto tmp = runtime->allocString((uint32_t)str.size());
    memcpy(tmp.data, str.c_str(), str.size());
    ctx->retValue = runtime->pushString(JsString(tmp));
}

void stringFromCodePoint(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    string str;
    for (uint32_t i = 0; i < args.count; i++) {
        auto &item = args.data[i];
        double code = item.type == JDT_INT32 ? item.value.n32 : runtime->toNumber(ctx, item);

        // 按照数据类型都完全正确的情况处理
        if (code == (uint32_t)code && code <= 0x10FFFF) {
            utf32CodeToUtf8((uint32_t)code, str);
        } else {
            SizedStringWrapper str(code);
            ctx->throwException(PE_RANGE_ERROR, "Invalid code point %.*s", str.len, str.data);
            return;
        }
    }

    auto tmp = runtime->allocString((uint32_t)str.size());
    memcpy(tmp.data, str.c_str(), str.size());
    ctx->retValue = runtime->pushString(JsString(tmp));
}

static JsLibProperty stringFunctions[] = {
    { "fromCharCode", stringFromCharCode },
    { "fromCodePoint", stringFromCodePoint },
    { "name", nullptr, "String" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


inline JsValue convertStringToJsValue(VMContext *ctx, const JsValue &thiz, const char *funcName) {
    if (thiz.type == JDT_STRING || thiz.type == JDT_CHAR) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_STRING) {
        auto obj = (JsStringObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(PE_TYPE_ERROR, "String.prototype.%s requires that 'this' be a String", funcName);
    return jsValueNotInitialized;
}

void stringPrototypeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "at");
    if (!strValue.isValid()) {
        return;
    }

    int32_t index = (int32_t)runtime->toNumber(ctx, args.getAt(0, JsValue(JDT_INT32, 0)));

    if (strValue.type == JDT_CHAR) {
        if (index == 0 || index == -1) {
            ctx->retValue = strValue;
        } else {
            ctx->retValue = jsValueUndefined;
        }
        return;
    }

    auto &str = runtime->getStringWithRandAccess(strValue);

    if (index < 0) {
        index = str.size() + index;
        if (index < 0) {
            ctx->retValue = jsValueUndefined;
            return;
        }
    }

    if (index < str.size()) {
        ctx->retValue = JsValue(JDT_CHAR, str.chartAt(index));
    } else {
        ctx->retValue = jsValueUndefined;
    }
}

int getStringCharCodeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args, cstr_t apiName) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, apiName);
    if (!strValue.isValid()) {
        return -1;
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (strValue.type == JDT_STRING) {
        auto &str = runtime->getStringWithRandAccess(strValue);
        if (index < 0 || index >= str.size()) {
            return -1;
        }
        return str.chartAt(index);
    } else {
        if (index != 0) {
            return -1;
        }
        return strValue.value.n32;
    }
}

void stringPrototypeCharAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int code = getStringCharCodeAt(ctx, thiz, args, "charAt");
    if (code == -1) {
        ctx->retValue = jsStringValueEmpty;
        return;
    }

    ctx->retValue = JsValue(JDT_CHAR, code);
}

void stringPrototypeCharCodeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int code = getStringCharCodeAt(ctx, thiz, args, "charCodeAt");
    if (code == -1) {
        ctx->retValue = jsValueNaN;
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, code);
}

void stringPrototypeCodePointAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "codePointAt");
    if (!strValue.isValid()) {
        return;
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (strValue.type == JDT_STRING) {
        auto &str = runtime->getStringWithRandAccess(strValue);
        if (index < 0 || index >= str.size()) {
            ctx->retValue = jsValueUndefined;
            return;
        }
        ctx->retValue = JsValue(JDT_INT32, str.codePointAt(index));
    } else {
        if (index != 0) {
            ctx->retValue = jsValueUndefined;
            return;
        }
        ctx->retValue = JsValue(JDT_INT32, strValue.value.n32);
    }
}

void stringPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertStringToJsValue(ctx, thiz, "toString");
    if (!value.isValid()) {
        return;
    }

    ctx->retValue = value;
}

void stringPrototypeLength(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "length");
    if (!strValue.isValid()) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, runtime->getStringLength(strValue));
}

static JsLibProperty stringPrototypeFunctions[] = {
    // @@iterator
    // * anchor
    { "at", stringPrototypeAt },
    // * big
    // * blink
    // * bold
    { "charAt", stringPrototypeCharAt },
    { "charCodeAt", stringPrototypeCharCodeAt },
    { "codePointAt", stringPrototypeCodePointAt },
    { "toString", stringPrototypeToString },
    makeJsLibPropertyGetter("length", stringPrototypeLength),
};

class StringPrototypeObject : public JsLibObject {
public:
    JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
        if (name.len > 0 && isDigit(name.data[0])) {
            bool successful = false;
            auto n = name.atoi(successful);
            if (successful) {
                return getRawByIndex(ctx, (uint32_t)n, includeProtoProp);
            }
        }

        return nullptr;
    }

    JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
        static JsProperty propIndex;
        return &propIndex;
    }

};

void registerString(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, stringPrototypeFunctions, CountOf(stringPrototypeFunctions));
    rt->objPrototypeString = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeString, prototypeObj);

    SET_PROTOTYPE(stringFunctions, jsValuePrototypeString);

    rt->setGlobalObject("String",
        new JsLibObject(rt, stringFunctions, CountOf(stringFunctions), stringConstructor));
}
