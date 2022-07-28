//
//  String.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "../VirtualMachineTypes.hpp"


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

        auto v = item;
        if (item.type >= JDT_OBJECT) {
            Arguments noArgs;
            runtime->vm->callMember(ctx, v, "toString", noArgs);
            if (ctx->error == PE_OK) {
                v = ctx->stack.back();
                ctx->stack.pop_back();
            }
        }
        
        switch (v.type) {
            case JDT_NOT_INITIALIZED:
                ctx->throwException(PE_REFERECNE_ERROR, "Cannot access variable before initialization");
                break;
            case JDT_UNDEFINED:
            case JDT_NULL:
                str.push_back('\0');
                break;
            case JDT_INT32:
            case JDT_BOOL:
                if (v.value.n32 >= 0 && v.value.n32 <= 0xFFFF) {
                    ucs2ToUtf8(v.value.n32, str);
                }
                break;
            case JDT_STRING: {
                auto s = runtime->getString(v);
                s.trim();
                double v;
                auto p = parseNumber(s, v);
                if (p == s.data + s.len) {
                    ucs2ToUtf8((int)v, str);
                } else {
                    str.push_back('\0');
                }
                break;
            }
            case JDT_NUMBER: {
                int n = (int)runtime->getDouble(v);
                if (n >= 0 && n <= 0xFFFF) {
                    ucs2ToUtf8(n, str);
                }
                break;
            }
            default:
                break;
        }
    }

    auto poolStr = runtime->allocString((uint32_t)str.size());
    memcpy(poolStr.value.data, str.c_str(), str.size());
    ctx->stack.push_back(JsValue(JDT_STRING, runtime->pushString(JsString(poolStr))));
}

void stringFromCodePoint(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    
}

static JsLibProperty stringFunctions[] = {
    { "fromCharCode", stringFromCharCode },
    { "fromCodePoint", stringFromCodePoint },
    { "name", nullptr, "String" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void registerString(VMRuntimeCommon *rt) {
    rt->setGlobalObject("String",
        new JsLibObject(rt, stringFunctions, CountOf(stringFunctions)));
}
