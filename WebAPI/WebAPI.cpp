//
//  WebAPI.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/12.
//

#include "WebAPI.hpp"


void registerConsole(VMContext *ctx, VMScope *globalScope);

void registerWebAPIs(VMContext *ctx, VMScope *globalScope) {
    registerConsole(ctx, globalScope);
}
