//
//  Console.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "WebAPI.hpp"
#include "interpreter/VirtualMachineTypes.hpp"
#include "objects/JsArray.hpp"


using SetUInts = set<uint32_t>;

void dumpObject(VMContext *ctx, const JsValue &obj, string &out, SetUInts &historyObjs) {
    assert(obj.type >= JDT_OBJECT);
    bool isArray = obj.type == JDT_ARRAY;

    // 避免同一个 object 被多次输出
    if (historyObjs.find(obj.value.index) != historyObjs.end()) {
        out.append(isArray ? "[...]" : "{...}");
        return;
    }
    historyObjs.insert(obj.value.index);

    auto runtime = ctx->runtime;

    IJsObject *pobj = runtime->getObject(obj);
    std::shared_ptr<IJsIterator> it(obj.type == JDT_ARRAY ? ((JsArray *)pobj)->getIteratorObjectAll(ctx) : pobj->getIteratorObject(ctx, false));

    SizedString key;
    JsValue value;
    vector<string> vs;
    vector<string> kvs;

    while (it->next(&key, nullptr, &value)) {
        auto s = runtime->toSizedString(ctx, value);
        if (value.type == JDT_OBJ_BOOL) { s = SizedString("Boolean"); }
        else if (value.type == JDT_OBJ_NUMBER) { s = SizedString("Number"); }
        else if (value.type == JDT_OBJ_STRING) { s = SizedString("String"); }
        else if (value.type == JDT_OBJ_SYMBOL) { s = SizedString("Symbol"); }
        else if (value.type == JDT_OBJECT) { s = SizedString("Object"); }
        else if (value.type == JDT_ARRAY) {
            auto arr = (JsArray *)runtime->getObject(value);
            s = LockedSizedStringWrapper(stringPrintf("Array(%d)", arr->length()));
        }

        if (isArray && key.isNumeric()) {
            vs.push_back(string((cstr_t)s.data, s.len));
        } else {
            string kv;
            kv.append((cstr_t)key.data, key.len);
            kv.append(": ");
            kv.append((cstr_t)s.data, s.len);
            kvs.push_back(kv);
        }
    }

    std::sort(kvs.begin(), kvs.end());
    vs.insert(vs.end(), kvs.begin(), kvs.end());

    out.append(isArray ? "[" : "{");
    out.append(strJoin(vs.begin(), vs.end(), ", "));
    out.append(isArray ? "]" : "}");
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
        if (v.type == JDT_OBJECT || v.type == JDT_ARRAY) {
            SetUInts historyObjs;
            dumpObject(ctx, v, out, historyObjs);
            continue;
        } else if (v.type >= JDT_OBJECT) {
            Arguments noArgs;
            runtime->vm->callMember(ctx, v, SS_TOSTRING, noArgs);
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
                utf32CodeToUtf8(v.value.n32, out);
                break;
            case JDT_STRING: {
                auto &s = runtime->getUtf8String(v);
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
