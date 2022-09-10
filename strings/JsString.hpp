//
//  JsString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/8.
//

#ifndef JsString_hpp
#define JsString_hpp

#include "IJsObject.hpp"


IJsIterator *newJsStringIterator(VMRuntime *runtime, const JsValue &str);

JsValue getStringMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &name);

#endif /* JsString_hpp */
