//
//  CommonString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#ifndef CommonString_hpp
#define CommonString_hpp

#include "../Utils/SizedString.h"


struct JsValue;

const int MAX_INT_TO_CONST_STR = 999;
SizedString intToSizedString(uint32_t n);

SizedString makeCommonString(const char *str);


class SizedStringWrapper : public SizedString {
public:
    SizedStringWrapper(int32_t n);
    SizedStringWrapper(uint32_t n) : SizedStringWrapper(int32_t(n)) { }
    SizedStringWrapper(double n);
    SizedStringWrapper(const JsValue &v);

    bool append(const JsValue &v);
    bool append(const SizedString &s);

    const SizedString &str() const { return *this; }

    enum {
        MAX_SIZE = 128,
    };

protected:
    uint8_t             _buf[MAX_SIZE];

};

#endif /* CommonString_hpp */
