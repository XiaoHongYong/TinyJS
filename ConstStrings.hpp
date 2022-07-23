//
//  ConstStrings.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#ifndef ConstStrings_hpp
#define ConstStrings_hpp

#include "../Utils/SizedString.h"


extern SizedString SS_PROTOTYPE;
extern SizedString SS_UNDEFINED;
extern SizedString SS_NULL;
extern SizedString SS_TRUE;
extern SizedString SS_FALSE;

extern SizedString SS_THIS;
extern SizedString SS_ARGUMENTS;
extern SizedString SS_LENGTH;


const int MAX_INT_TO_CONST_STR = 999;
SizedString intToSizedString(uint32_t n);


#endif /* ConstStrings_hpp */
