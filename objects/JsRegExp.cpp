//
//  JsRegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#include "JsRegExp.hpp"


bool parseRegexpFlags(const SizedString &flags, uint32_t &flagsOut) {
    flagsOut = 0;
    auto p = flags.data, end = flags.data + flags.len;

    while (p < end) {
        auto c = *p;
        if (c == 'i') {
            flagsOut |= RF_CASE_INSENSITIVE;
        } else if (c == 'g') {
            flagsOut |= RF_GLOBAL_SEARCH;
        } else if (c == 'm') {
            flagsOut |= RF_MULTILINE;
        } else if (c == 's') {
            flagsOut |= RF_DOT_ALL;
        } else if (c == 'u') {
            flagsOut |= RF_UNICODE;
        } else if (c == 'y') {
            flagsOut |= RF_STICKY;
        } else if (c == 'd') {
            flagsOut |= RF_INDEX;
        } else {
            return false;
        }
        p++;
    }

    return true;
}

JsRegExp::JsRegExp(const SizedString &str, const std::regex &re, uint32_t flags) : JsObjectLazy(_props, CountOf(_props), jsValuePrototypeRegExp),  _strRe((cstr_t)str.data, str.len), _flags(flags), _re(re) {
    type = JDT_REGEX;

    // isGSetter, isConfigurable, isEnumerable, isWritable
    JsLazyProperty props[] = {
        // 添加缺省的 lastIndex 属性.
        { SS_LASTINDEX, JsProperty(JsValue(JDT_INT32, 0), false, false, false, true), false, },
        { SS_DOTALL, JsProperty(JsValue(JDT_BOOL, flags &RF_DOT_ALL), false, false, false, false), false, },
        { SS_FLAGS, JsProperty(JsValue(JDT_BOOL, false), flags, false, false, false), false, },
        { SS_GLOBAL, JsProperty(JsValue(JDT_BOOL, flags &RF_GLOBAL_SEARCH), false, false, false, false), false, },
        { SS_HASINDICES, JsProperty(JsValue(JDT_BOOL, flags &RF_INDEX), false, false, false, false), false, },
        { SS_IGNORECASE, JsProperty(JsValue(JDT_BOOL, flags &RF_CASE_INSENSITIVE), false, false, false, false), false, },
        { SS_MULTILINE, JsProperty(JsValue(JDT_BOOL, flags &RF_MULTILINE), false, false, false, false), false, },
        { SS_SOURCE, JsProperty(JsValue(JDT_BOOL, false), false, false, false, false), false, },
        { SS_STICKY, JsProperty(JsValue(JDT_BOOL, flags &RF_STICKY), false, false, false, false), false, },
        { SS_UNICODE, JsProperty(JsValue(JDT_BOOL, flags &RF_UNICODE), false, false, false, false), false, },
    };

    static_assert(sizeof(props) == sizeof(props));
    memcpy(_props, props, sizeof(props));
}

JsRegExp::~JsRegExp() {
}

void JsRegExp::setLastIndex(int index) {
    assert(_props[0].name.equal(SS_LASTINDEX));
    _props[0].prop.value.value.n32 = index;
}

IJsObject *JsRegExp::clone() {
    return new JsRegExp(_strRe, _re, _flags);
}
