//
//  Console.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "WebAPI.hpp"
#include "interpreter/VirtualMachineTypes.hpp"


using SetUInts = set<uint32_t>;

void dumpObject(VMContext *ctx, const JsValue &obj,string &out, SetUInts &historyObjs) {
    assert(obj.type >= JDT_OBJECT);

    // 避免同一个 object 被多次输出
    if (historyObjs.find(obj.value.index) != historyObjs.end()) {
        out.append("{...}");
        return;
    }
    historyObjs.insert(obj.value.index);

    auto runtime = ctx->runtime;

    IJsObject *pobj = runtime->getObject(obj);
    std::shared_ptr<IJsIterator> it(pobj->getIteratorObject(ctx, false));

    out.append("{");

    SizedString key;
    JsValue value;
    bool first = true;
    string buf;

    while (it->next(&key, nullptr, &value)) {
        if (first) first = false; else out.append(",");

        out.append((cstr_t)key.data, key.len);
        out.append(": ");
        auto s = runtime->toSizedString(ctx, value, buf);
        out.append((cstr_t)s.data, s.len);
    }

    out.append("}");
}

void consoleLog(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    string out;
    char buf[64];

    for (uint32_t i = 0; i < args.count; i++) {
        if (!out.empty()) {
            out.append(" ");
        }

        auto v = args.data[i];
        if (v.type == JDT_OBJECT) {
            SetUInts historyObjs;
            dumpObject(ctx, v, out, historyObjs);
            continue;
        } else if (v.type >= JDT_OBJECT) {
            Arguments noArgs;
            runtime->vm->callMember(ctx, v, "toString", noArgs);
            if (ctx->error == PE_OK) {
                v = ctx->retValue;
            } else {
                return;
            }
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
            case JDT_NUMBER: {
                auto d = runtime->getDouble(v);
                auto len = floatToString(d, buf);
                out.append(buf, len);
                break;
            }
            case JDT_CHAR:
                out.append(1, (char)v.value.n32);
                break;
            case JDT_STRING: {
                auto s = runtime->getString(v);
                out.append((const char *)s.data, s.len);
                break;
            }
            case JDT_SYMBOL: {
                auto s = runtime->getSymbol(v);
                out.append(s.toString());
                break;
            }
            default:
                out.append("[Object]");
                break;
        }
    }

    runtime->console->log(SizedString(out));
    ctx->retValue = jsValueUndefined;
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

    ctx->retValue = jsValueUndefined;
}

static JsLibProperty consoleFunctions[] = {
    { "log", consoleLog },
    { "trace", consoleTrace },
};

void registerConsole(VMRuntimeCommon *rt) {
    rt->setGlobalObject("console",
        new JsLibObject(rt, consoleFunctions, CountOf(consoleFunctions)));
}
