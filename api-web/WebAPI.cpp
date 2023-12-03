//
//  WebAPI.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/12.
//

#include "WebAPI.hpp"


void registerConsole(VMRuntimeCommon *rt);
void registerWindow(VMRuntimeCommon *rt);

void registerWebAPIs(VMRuntimeCommon *rt) {
    registerWindow(rt);
    registerConsole(rt);
}
