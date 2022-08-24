//
//  BuiltIn.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#include "BuiltIn.hpp"


/**
 * 所有文档参考：https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects
 *
 * 除了 Object 外，内置对象的 prototye 的 __proto__ 属性都是 Object.prototype
 *   Object.is(String.prototype.__proto__, Object.prototype) : true
 *   Object.is(Symbol.prototype.__proto__, Object.prototype) : true
 *   Object.is(Number.prototype.__proto__, Object.prototype) : true
 *   Object.is(Object.prototype.__proto__, Object.prototype) : false
 * Object.prototype 是 null
 */

void registerBoolean(VMRuntimeCommon *rt);
void registerNumber(VMRuntimeCommon *rt);
void registerRegExp(VMRuntimeCommon *rt);
void registerString(VMRuntimeCommon *rt);
void registerObject(VMRuntimeCommon *rt);
void registerArray(VMRuntimeCommon *rt);
void registerSymbol(VMRuntimeCommon *rt);
void registerObjFunction(VMRuntimeCommon *rt);
void registerErrorAPIs(VMRuntimeCommon *rt);
void registerEval(VMRuntimeCommon *rt);


void registerBuiltIns(VMRuntimeCommon *rt) {
    registerBoolean(rt);
    registerNumber(rt);
    registerRegExp(rt);
    registerString(rt);
    registerObject(rt);
    registerArray(rt);
    registerSymbol(rt);
    registerObjFunction(rt);
    registerErrorAPIs(rt);
    registerEval(rt);
}
