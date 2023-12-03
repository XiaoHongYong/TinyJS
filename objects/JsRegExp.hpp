//
//  JsRegExp.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#ifndef JsRegExp_hpp
#define JsRegExp_hpp

#include "JsObjectLazy.hpp"
#include <regex>


enum RegexpFlags {
    RF_CASE_INSENSITIVE = std::regex_constants::icase,      // i    Case-insensitive search.    ignoreCase
#ifdef _WIN32
    RF_MULTILINE        = 1 << 15,  // m    Allows ^ and $ to match newline characters.    multiline
#else
    RF_MULTILINE        = std::regex_constants::multiline,  // m    Allows ^ and $ to match newline characters.    multiline
#endif
    RF_DOT_ALL          = 1 << 16, // s    Allows . to match newline characters.    dotAll
    RF_UNICODE          = 1 << 17, // u    "Unicode"; treat a pattern as a sequence of Unicode code points.    unicode
    RF_STICKY           = 1 << 18, // y    Perform a "sticky" search that matches starting at the current
                                   //      position in the target string.    sticky
    RF_GLOBAL_SEARCH    = 1 << 19, // g    Global search.    global
    RF_INDEX            = 1 << 20, // d    Generate indices for substring matches.    hasIndices
};

bool parseRegexpFlags(const StringView &flags, uint32_t &flagsOut);

class JsRegExp : public JsObjectLazy {
public:
    JsRegExp(const StringView &str, const std::regex &re, uint32_t flags);
    ~JsRegExp();

    const string &toString() const { return _strRe; }

    std::regex &getRegexp() { return _re; }
    uint32_t flags() const { return _flags; }

    void setLastIndex(int index);

    virtual IJsObject *clone() override;

protected:
    JsLazyProperty              _props[10];

    string                      _strRe;
    std::regex                  _re;
    uint32_t                    _flags;

};

#endif /* JsRegExp_hpp */
