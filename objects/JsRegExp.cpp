//
//  JsRegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#include "JsRegExp.hpp"


bool parseRegexpFlags(const StringView &flags, uint32_t &flagsOut) {
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

JsRegExp::JsRegExp(const StringView &str, const std::regex &re, uint32_t flags) : JsObjectLazy(_props, CountOf(_props), jsValuePrototypeRegExp, JDT_REGEX),  _strRe((cstr_t)str.data, str.len), _flags(flags), _re(re)
{
    // isGSetter, isConfigurable, isEnumerable, isWritable
    JsLazyProperty props[] = {
        // 添加缺省的 lastIndex 属性.
        { SS_LASTINDEX, makeJsValueInt32(0).asProperty(JP_WRITABLE), false, },
        { SS_DOTALL, makeJsValueBool(flags &RF_DOT_ALL).asProperty(0), false, },
        { SS_FLAGS, makeJsValueBool(false).asProperty(0), false, },
        { SS_GLOBAL, makeJsValueBool(flags & RF_GLOBAL_SEARCH).asProperty(0), false, },
        { SS_HASINDICES, makeJsValueBool(flags & RF_INDEX).asProperty(0), false, },
        { SS_IGNORECASE, makeJsValueBool(flags & RF_CASE_INSENSITIVE).asProperty(0), false, },
        { SS_MULTILINE, makeJsValueBool(flags & RF_MULTILINE).asProperty(0), false, },
        { SS_SOURCE, makeJsValueBool(false).asProperty(0), false, },
        { SS_STICKY, makeJsValueBool(flags & RF_STICKY).asProperty(0), false, },
        { SS_UNICODE, makeJsValueBool(flags & RF_UNICODE).asProperty(0), false, },
    };

    static_assert(sizeof(props) == sizeof(props));
    memcpy(_props, props, sizeof(props));
}

JsRegExp::~JsRegExp() {
}

void JsRegExp::setLastIndex(int index) {
    assert(_props[0].name.equal(SS_LASTINDEX));
    _props[0].prop.setValue(makeJsValueInt32(index));
}

IJsObject *JsRegExp::clone() {
    return new JsRegExp(_strRe, _re, _flags);
}
