//
//  CommonString.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#include "CommonString.hpp"
#include "VirtualMachine.hpp"


SizedString makeCommonString(const char *str) {
    SizedString s(str);
    s.unused = COMMON_STRINGS;
    return s;
}

const int INT_STR_POS_10 = 10;
const int INT_STR_POS_100 = INT_STR_POS_10 + 2 * (100 - 10);
const int INT_STR_SIZE = INT_STR_POS_100 + 3 * (1000 - 100);

uint8_t *initializeInt2StrBuf() {
    static uint8_t buf[INT_STR_SIZE];

    auto p = buf;
    for (int i = 0; i <= 9; i++) {
        *p++ = '0' + i;
    }
    assert(INT_STR_POS_10 == p - buf);

    for (int i = 1; i <= 9; i++) {
        for (int k = 0; k <= 9; k++) {
            *p++ = '0' + i;
            *p++ = '0' + k;
        }
    }
    assert(INT_STR_POS_100 == p - buf);

    for (int n = 1; n <= 9; n++) {
        for (int i = 0; i <= 9; i++) {
            for (int k = 0; k <= 9; k++) {
                *p++ = '0' + n;
                *p++ = '0' + i;
                *p++ = '0' + k;
            }
        }
    }
    assert(INT_STR_SIZE == p - buf);

    return buf;
}

const uint8_t *INT_STR_BUF = initializeInt2StrBuf();

SizedString intToSizedString(uint32_t n) {
    if (n < 10) {
        return SizedString(INT_STR_BUF + n, 1, COMMON_STRINGS);
    } else if (n < 100) {
        return SizedString(INT_STR_BUF + INT_STR_POS_10 + (n - 10) * 2, 2, COMMON_STRINGS);
    } else if (n < 1000) {
        return SizedString(INT_STR_BUF + INT_STR_POS_100 + (n - 100) * 3, 3, COMMON_STRINGS);
    } else {
        return SizedString(nullptr, 0, 0);
    }
}

NumberToSizedString::NumberToSizedString(uint32_t n) {
    auto ss = intToSizedString(n);
    if (ss.len == 0) {
        ss.len = (uint32_t)::itoa(n, (char *)_buf);
        ss.data = _buf;
    }

    len = ss.len;
    data = ss.data;
    unused = ss.unused;
}

SizedStringWrapper::SizedStringWrapper(int32_t n) {
    len = (uint32_t)::itoa(n, (char *)_buf);
    data = _buf;
}

SizedStringWrapper::SizedStringWrapper(double n) {
    len = floatToString(n, (char *)_buf);
    data = _buf;
}

SizedStringWrapper::SizedStringWrapper(const JsValue &v) {
    data = _buf;

    if (v.type == JDT_CHAR) {
        if (len + 1 < CountOf(_buf)) {
            _buf[len] = (char)v.value.n32;
            len++;
        } else {
        }
    } else if (v.type == JDT_UNDEFINED) {
        append(JsStringValueUndefined);
    } else if (v.type == JDT_NULL) {
        append(JsStringValueNull);
    } else if (v.type == JDT_BOOL) {
        append(v.value.n32 ? JsStringValueTrue : JsStringValueFalse);
    } else if (v.type == JDT_INT32) {
        SizedStringWrapper s2(v.value.n32);
        append(s2.str());
    }
}

bool SizedStringWrapper::append(const JsValue &v) {
    if (v.type == JDT_CHAR) {
        if (len + 1 < CountOf(_buf)) {
            _buf[len] = (char)v.value.n32;
            len++;
            return true;
        } else {
            return false;
        }
    } else if (v.type == JDT_UNDEFINED) {
        return append(JsStringValueUndefined);
    } else if (v.type == JDT_NULL) {
        return append(JsStringValueNull);
    } else if (v.type == JDT_BOOL) {
        return append(v.value.n32 ? JsStringValueTrue : JsStringValueFalse);
    } else if (v.type == JDT_INT32) {
        SizedStringWrapper s2(v.value.n32);
        return append(s2.str());
    }

    return false;
}

bool SizedStringWrapper::append(const SizedString &s) {
    if (len + s.len < CountOf(_buf)) {
        memcpy(_buf + len, s.data, s.len);
        len += s.len;
        return true;
    } else {
        return false;
    }
}
