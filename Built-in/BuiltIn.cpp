//
//  BuiltIn.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#include "BuiltIn.hpp"


void registerString(VMRuntimeCommon *rt);
void registerObject(VMRuntimeCommon *rt);
void registerSymbol(VMRuntimeCommon *rt);
void registerErrorAPIs(VMRuntimeCommon *rt);

void registerBuiltIns(VMRuntimeCommon *rt) {
    registerString(rt);
    registerObject(rt);
    registerSymbol(rt);
    registerErrorAPIs(rt);
}
