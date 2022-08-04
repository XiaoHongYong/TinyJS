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

    for (uint32_t i = 0; i < args.count; i++) {
        auto v = args.data[i];
        if (v.type >= JDT_OBJECT) {
            Arguments noArgs;
            runtime->vm->callMember(ctx, v, "toString", noArgs);
            if (ctx->error == PE_OK) {
                v = ctx->stack.back();
                ctx->stack.pop_back();
            } else {
                return;
            }
        }

        switch (v.type) {
            case JDT_NOT_INITIALIZED:
                ctx->throwException(PE_REFERECNE_ERROR, "Cannot access variable before initialization");
                break;
            case JDT_UNDEFINED:
                printf("undefined ");
                break;
            case JDT_NULL:
                printf("null ");
                break;
            case JDT_INT32:
                printf("%d ", v.value.n32);
                break;
            case JDT_BOOL:
                printf("%s ", v.value.n32 ? "true" : "false");
                break;
            case JDT_NUMBER:
                printf("%lf ", runtime->getDouble(v));
                break;
            case JDT_CHAR:
                printf("%c ", v.value.n32);
                break;
            case JDT_STRING: {
                auto s = runtime->getString(v);
                printf("%.*s ", (int)s.len, s.data);
                break;
            }
            case JDT_SYMBOL: {
                auto s = runtime->getSymbolName(v);
                printf("%.*s ", (int)s.len, s.data);
                break;
            }
            default:
                printf("[Object] ");
                break;
        }
    }

    printf("\n");
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
}

static JsLibProperty consoleFunctions[] = {
    { "log", consoleLog },
    { "trace", consoleTrace },
};

void registerConsole(VMRuntimeCommon *rt) {
    rt->setGlobalObject("console",
        new JsLibObject(rt, consoleFunctions, CountOf(consoleFunctions)));
}
