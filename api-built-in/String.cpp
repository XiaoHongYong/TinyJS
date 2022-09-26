//
//  String.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"


void ucs2ToUtf8(uint16_t code, string &out) {
    if (code < 0x80) {
        out.push_back((uint8_t)code);
    } else if (code < 0x0800) {
        out.push_back((uint8_t)((code >> 6) | 0xC0));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    } else {
        out.push_back((uint8_t)((code >> 12) | 0xE0));
        out.push_back((uint8_t)((code >> 6 & 0x3F) | 0x80));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    }
}

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
        if (item.type == JDT_INT32) {
            // 按照数据类型都完全正确的情况处理
            if (item.value.n32 <= 0xFFFF) {
                ucs2ToUtf8(item.value.n32, str);
            }
            continue;
        }

        auto f = runtime->toNumber(ctx, item);
        ucs2ToUtf8((uint16_t)f, str);
    }

    auto tmp = runtime->allocString((uint32_t)str.size());
    memcpy(tmp.data, str.c_str(), str.size());
    ctx->retValue = runtime->pushString(JsString(tmp));
}

void stringFromCodePoint(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = jsValueUndefined;
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

    SizedString str;
    uint8_t buf[32];
    if (strValue.type == JDT_STRING) {
        str = runtime->getString(strValue);
    } else {
        buf[0] = strValue.value.n32;
        str = SizedString(buf, 1);
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (index < 0) {
        index = str.len + index;
        if (index < 0) {
            ctx->retValue = jsValueUndefined;
            return;
        }
    }

    if (index < str.len) {
        ctx->retValue = JsValue(JDT_CHAR, str.data[index]);
    } else {
        ctx->retValue = jsValueUndefined;
    }
}

void stringPrototypeCharAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "charAt");
    if (!strValue.isValid()) {
        return;
    }

    SizedString str;
    uint8_t buf[32];
    if (strValue.type == JDT_STRING) {
        str = runtime->getString(strValue);
    } else {
        buf[0] = strValue.value.n32;
        str = SizedString(buf, 1);
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (index < 0 || index >= str.len) {
        ctx->retValue = jsStringValueEmpty;
        return;
    }

    ctx->retValue = JsValue(JDT_CHAR, str.data[index]);
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
    { "at", stringPrototypeAt },
    { "charAt", stringPrototypeCharAt },
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
