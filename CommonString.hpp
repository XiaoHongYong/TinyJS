//
//  CommonString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#ifndef CommonString_hpp
#define CommonString_hpp

#include "../Utils/SizedString.h"


const int MAX_INT_TO_CONST_STR = 999;
SizedString intToSizedString(uint32_t n);

SizedString makeCommonString(const char *str);

class NumberToSizedString : public SizedString {
public:
    NumberToSizedString(uint32_t n);

    const SizedString &str() const { return *this; }
    
protected:
    uint8_t             _buf[32];

};

#endif /* CommonString_hpp */
