//
//  JsRegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#include "JsRegExp.hpp"


JsRegExp::JsRegExp(const SizedString &re) : JsObject(jsValuePrototypeRegExp),  _strRe((cstr_t)re.data, re.len), _re(_strRe.c_str()) {
    type = JDT_REGEX;

    // 添加缺省的 lastIndex 属性.
    _props[SS_LASTINDEX] = JsValue(JDT_INT32, 0);
}

JsRegExp::~JsRegExp() {
}
