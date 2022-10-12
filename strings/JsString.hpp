//
//  JsString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/8.
//

#ifndef JsString_hpp
#define JsString_hpp

#include "JsObject.hpp"


IJsIterator *newJsStringIterator(VMContext *ctx, const JsValue &str, bool includeProtoProps);

JsValue getStringMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &name);

#endif /* JsString_hpp */
