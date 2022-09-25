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
}

static JsLibProperty windowFunctions[] = {
    { "name", nullptr, "Window" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static void windowPrototypeAlert(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
}

static JsLibProperty windowPrototypeFunctions[] = {
    { "alert", windowPrototypeAlert },
};

void registerWindow(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, windowPrototypeFunctions, CountOf(windowPrototypeFunctions));
    rt->objPrototypeWindow = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeWindow, prototypeObj);

    SET_PROTOTYPE(windowFunctions, jsValuePrototypeWindow);

    rt->setGlobalObject("Window",
        new JsLibObject(rt, windowFunctions, CountOf(windowFunctions), windowConstructor));
}
