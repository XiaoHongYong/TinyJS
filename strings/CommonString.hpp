//
//  CommonString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#ifndef CommonString_hpp
#define CommonString_hpp

#include "utils/SizedString.h"


struct JsValue;

const int MAX_INT_TO_CONST_STR = 999;

SizedString intToSizedString(uint32_t n);
SizedString makeCommonString(const char *str);


class NumberToSizedString : public SizedString {
public:
    NumberToSizedString(uint32_t n);

    void set(uint32_t n);
    const SizedString &str() const { return *this; }

protected:
    uint8_t             _buf[32];

};


class SizedStringWrapper : public SizedString {
public:
    SizedStringWrapper() : SizedString(_buf, 0) { };
    SizedStringWrapper(int32_t n);
    SizedStringWrapper(uint32_t n) : SizedStringWrapper(int32_t(n)) { }
    SizedStringWrapper(double n);
    SizedStringWrapper(const JsValue &v);
    SizedStringWrapper(const SizedString &s);
    SizedStringWrapper(const string &v);

    SizedStringWrapper(const SizedStringWrapper &other);
    SizedStringWrapper &operator =(const SizedStringWrapper &other);

    void clear();

    bool append(const JsValue &v);
    bool append(const SizedString &s);
    bool append(double v);
    bool append(uint32_t n);
    bool append(int32_t n);

    const SizedString &str() const { return *this; }

    enum {
        MAX_SIZE = 128,
    };

protected:
    uint8_t             _buf[MAX_SIZE];

};

class LockedSizedStringWrapper : public SizedString {
public:
    LockedSizedStringWrapper() { }
    LockedSizedStringWrapper(int32_t n);
    LockedSizedStringWrapper(uint32_t n) : LockedSizedStringWrapper(int32_t(n)) { }
    LockedSizedStringWrapper(double n);
    LockedSizedStringWrapper(const JsValue &v);
    LockedSizedStringWrapper(const SizedString &s) { *(SizedString *)this = s; }
    LockedSizedStringWrapper(const string &v);

    LockedSizedStringWrapper(const LockedSizedStringWrapper &other);
    LockedSizedStringWrapper &operator =(const LockedSizedStringWrapper &other);

    void clear();

    void reset(const SizedString &s);

protected:
    enum {
        MAX_SIZE = 128,
    };

    uint8_t             _buf[MAX_SIZE];

};

#endif /* CommonString_hpp */
