//
//  CommonString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#ifndef CommonString_hpp
#define CommonString_hpp

#include "utils/StringView.h"


struct JsValue;

const int MAX_INT_TO_CONST_STR = 999;

StringView intToStringView(uint32_t n);
StringView makeCommonString(const char *str);


class NumberToStringView : public StringView {
public:
    NumberToStringView(uint32_t n);

    void set(uint32_t n);
    const StringView &str() const { return *this; }

protected:
    uint8_t             _buf[32];

};


class StringViewWrapper : public StringView {
public:
    StringViewWrapper() : StringView(_buf, 0) { };
    StringViewWrapper(int32_t n);
    StringViewWrapper(uint32_t n) : StringViewWrapper(int32_t(n)) { }
    StringViewWrapper(double n);
    StringViewWrapper(const JsValue &v);
    StringViewWrapper(const StringView &s);
    StringViewWrapper(const string &v);

    StringViewWrapper(const StringViewWrapper &other);
    StringViewWrapper &operator =(const StringViewWrapper &other);

    void clear();

    bool append(const JsValue &v);
    bool append(const StringView &s);
    bool append(double v);
    bool append(uint32_t n);
    bool append(int32_t n);

    const StringView &str() const { return *this; }

    enum {
        MAX_SIZE = 128,
    };

protected:
    uint8_t             _buf[MAX_SIZE];

};

class LockedStringViewWrapper : public StringView {
public:
    LockedStringViewWrapper() { }
    LockedStringViewWrapper(int32_t n);
    LockedStringViewWrapper(uint32_t n) : LockedStringViewWrapper(int32_t(n)) { }
    LockedStringViewWrapper(double n);
    LockedStringViewWrapper(const JsValue &v);
    LockedStringViewWrapper(const StringView &s) { *(StringView *)this = s; }
    LockedStringViewWrapper(const string &v);

    LockedStringViewWrapper(const LockedStringViewWrapper &other);
    LockedStringViewWrapper &operator =(const LockedStringViewWrapper &other);

    void clear();

    void reset(const StringView &s);

protected:
    enum {
        MAX_SIZE = 128,
    };

    uint8_t             _buf[MAX_SIZE];

};

#endif /* CommonString_hpp */
