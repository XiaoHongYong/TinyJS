//
//  JsRegExp.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#ifndef JsRegExp_hpp
#define JsRegExp_hpp

#include "IJsObject.hpp"
#include <regex>


class JsRegExp : public JsObject {
public:
    JsRegExp(const SizedString &re);
    ~JsRegExp();

    const string &toString() const { return _strRe; }

protected:
    string                      _strRe;
    std::regex                  _re;

};

#endif /* JsRegExp_hpp */
