//
//  Console.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "Console.hpp"
#include "../VirtualMachineTypes.hpp"


void consoleLog(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    for (uint32_t i = 0; i < args.count; i++) {
        auto &v = args.data[i];
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
            case JDT_STRING: {
                auto s = runtime->getString(v);
                printf("%.*s ", (int)s.len, s.data);
                break;
            }
            case JDT_REGEX:
            case JDT_ARRAY:
            case JDT_OBJECT:
            case JDT_FUNCTION:
            case JDT_NATIVE_FUNCTION:
            {
                Arguments noArgs;
                runtime->vm->callMember(ctx, v, "toString", noArgs);
                if (ctx->error == PE_OK) {
                    JsValue r = ctx->stack.back();
                    ctx->stack.pop_back();
                    assert(r.type == JDT_STRING);
                    if (r.type == JDT_STRING) {
                        auto s = runtime->getString(v);
                        printf("%.*s ", (int)s.len, s.data);
                    }
                }
                break;
            }
            default:
                assert(0);
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

static JsLibFunction consoleFunctions[] = {
    { "log", consoleLog },
    { "trace", consoleTrace },
};

void registerConsole(VMContext *ctx, VMScope *globalScope) {
    registerGlobalObject(ctx, globalScope, "console",
        new JsLibObject(ctx, consoleFunctions, CountOf(consoleFunctions)));
}
