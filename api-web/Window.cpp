//
//  Console.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "WebAPI.hpp"
#include "interpreter/VirtualMachineTypes.hpp"


// https://developer.mozilla.org/en-US/docs/Web/API/Window
static void windowConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->throwException(JE_TYPE_ERROR, "Illegal constructor");
}

static JsLibProperty windowFunctions[] = {
    { "name", nullptr, "Window" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

static void windowPrototypeAlert(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
}

void window_setInterval(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->throwException(JE_TYPE_ERROR, "Failed to execute 'setInterval' on 'Window': 1 argument required, but only 0 present.");
        return;
    }

    auto callback = args.getAt(0);
    auto duration = args.getIntAt(ctx, 1, 0);

    auto id = ctx->runtime->registerTimer(ctx, callback, duration, true);
    ctx->retValue = makeJsValueInt32(id);
}

void window_setTimeout(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->throwException(JE_TYPE_ERROR, "Failed to execute 'setTimeout' on 'Window': 1 argument required, but only 0 present.");
        return;
    }

    auto callback = args.getAt(0);
    auto duration = args.getIntAt(ctx, 1, 0);

    auto id = ctx->runtime->registerTimer(ctx, callback, duration, false);
    ctx->retValue = makeJsValueInt32(id);
}

void window_clearTimer(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        return;
    }

    auto id = args.getIntAt(ctx, 0, 0);
    ctx->runtime->unregisterTimer(id);
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty globalFunctions[] = {
    { "alert", windowPrototypeAlert },
    { "setInterval", window_setInterval },
    { "setTimeout", window_setTimeout },
    { "clearInterval", window_clearTimer },
    { "clearTimeout", window_clearTimer },
};

static JsLibProperty windowPrototypeFunctions[] = { };

void registerWindow(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, windowPrototypeFunctions, CountOf(windowPrototypeFunctions));
    rt->objPrototypeWindow = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeWindow, prototypeObj);

    SET_PROTOTYPE(windowFunctions, jsValuePrototypeWindow);

    setGlobalLibObject("Window", rt, windowFunctions, CountOf(windowFunctions), windowConstructor, jsValuePrototypeFunction);

    // 注册全局函数
    for (auto &item : globalFunctions) {
        item.name.setStable();
        auto f = rt->pushNativeFunction(item.function, item.name);
        rt->setGlobalValue((char *)item.name.data, f);
    }
}
