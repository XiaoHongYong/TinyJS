//
//  Console.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "WebAPI.hpp"
#include "../VirtualMachineTypes.hpp"


void consoleLog(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    string out;
    char buf[64];

    for (uint32_t i = 0; i < args.count; i++) {
        auto v = args.data[i];
        if (v.type >= JDT_OBJECT) {
            Arguments noArgs;
            runtime->vm->callMember(ctx, v, "toString", noArgs);
            if (ctx->error == PE_OK) {
                v = ctx->retValue;
            } else {
                return;
            }
        }

        if (!out.empty()) {
            out.append(" ");
        }

        switch (v.type) {
            case JDT_NOT_INITIALIZED:
                ctx->throwException(PE_REFERECNE_ERROR, "Cannot access variable before initialization");
                break;
            case JDT_UNDEFINED:
                out.append("undefined");
                break;
            case JDT_NULL:
                out.append("null");
                break;
            case JDT_INT32:
                sprintf(buf, "%d", v.value.n32);
                out.append(buf);
                break;
            case JDT_BOOL:
                out.append(v.value.n32 ? "true" : "false");
                break;
            case JDT_NUMBER:
                sprintf(buf, "%lf", runtime->getDouble(v));
                out.append(buf);
                break;
            case JDT_CHAR:
                out.append(1, (char)v.value.n32);
                break;
            case JDT_STRING: {
                auto s = runtime->getString(v);
                out.append((const char *)s.data, s.len);
                break;
            }
            case JDT_SYMBOL: {
                auto s = runtime->getSymbolName(v);
                out.append((const char *)s.data, s.len);
                break;
            }
            default:
                out.append("[Object]");
                break;
        }
    }

    runtime->console->log(SizedString(out));
    ctx->retValue = JsUndefinedValue;
}

void consoleTrace(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    for (auto it = ctx->stackFrames.rbegin(); it != ctx->stackFrames.rend(); ++it) {
        auto f = (*it)->function;
        auto &name = f->name;
        if (name.len) {
            printf("%-14.*s %d:%d\n", (int)name.len, name.data, f->line, f->col);
        } else {
            printf("(anonymous)    %d:%d\n", f->line, f->col);
        }
    }

    ctx->retValue = JsUndefinedValue;
}

static JsLibProperty consoleFunctions[] = {
    { "log", consoleLog },
    { "trace", consoleTrace },
};

void registerConsole(VMRuntimeCommon *rt) {
    rt->setGlobalObject("console",
        new JsLibObject(rt, consoleFunctions, CountOf(consoleFunctions)));
}
