//
//  CommonString.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/23.
//

#include "CommonString.hpp"
#include "VirtualMachine.hpp"


SizedString makeCommonString(const char *str) {
    SizedString s(str, strlen(str), true);
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
    set(n);
}

void NumberToSizedString::set(uint32_t n) {
    auto ss = intToSizedString(n);
    if (ss.len == 0) {
        len = (uint32_t)::itoa(n, (char *)_buf);
        data = _buf;
    } else {
        *(SizedString *)this = ss;
    }
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
        len = utf32CodeToUtf8(v.value.n32, data);
    } else if (v.type == JDT_UNDEFINED) {
        append(SS_UNDEFINED);
    } else if (v.type == JDT_NULL) {
        append(SS_NULL);
    } else if (v.type == JDT_BOOL) {
        append(v.value.n32 ? SS_TRUE : SS_FALSE);
    } else if (v.type == JDT_INT32) {
        SizedStringWrapper s2(v.value.n32);
        append(s2.str());
    } else {
        assert(0);
    }
}

SizedStringWrapper::SizedStringWrapper(const SizedString &s) {
    data = _buf;
    len = 0;

    append(s);
}

SizedStringWrapper::SizedStringWrapper(const string &v) {
    data = _buf;
    len = min((int)v.size(), (int)CountOf(_buf));
    memcpy(_buf, v.c_str(), len);
}

SizedStringWrapper::SizedStringWrapper(const SizedStringWrapper &other) {
    data = _buf;
    len = other.len;
    memcpy(_buf, other._buf, other.len);
}

SizedStringWrapper &SizedStringWrapper::operator =(const SizedStringWrapper &other) {
    data = _buf;
    len = other.len;
    memcpy(_buf, other._buf, other.len);

    return *this;
}

void SizedStringWrapper::clear() {
    len = 0;
    data = _buf;
}

bool SizedStringWrapper::append(const JsValue &v) {
    if (v.type == JDT_CHAR) {
        if (len + utf32CodeToUtf8Length(v.value.n32) < CountOf(_buf)) {
            len += utf32CodeToUtf8(v.value.n32, _buf + len);
            return true;
        } else {
            assert(0);
            return false;
        }
    } else if (v.type == JDT_UNDEFINED) {
        return append(SS_UNDEFINED);
    } else if (v.type == JDT_NULL) {
        return append(SS_NULL);
    } else if (v.type == JDT_BOOL) {
        return append(v.value.n32 ? SS_TRUE : SS_FALSE);
    } else if (v.type == JDT_INT32) {
        SizedStringWrapper s2(v.value.n32);
        return append(s2.str());
    } else {
        assert(0);
    }

    return false;
}

bool SizedStringWrapper::append(const SizedString &s) {
    if (len + s.len < CountOf(_buf)) {
        memcpy(_buf + len, s.data, s.len);
        len += s.len;
        return true;
    } else {
        assert(0);
        return false;
    }
}

bool SizedStringWrapper::append(double v) {
    if (len + 32 < CountOf(_buf)) {
        len += floatToString(v, (char *)_buf + len);
        return true;
    } else {
        assert(0);
        return false;
    }
}

bool SizedStringWrapper::append(uint32_t n) {
    if (len + 12 < CountOf(_buf)) {
        len += (uint32_t)::itoa(n, (char *)_buf + len);
        return true;
    }

    assert(0);
    return false;
}

bool SizedStringWrapper::append(int32_t n) {
    if (len + 12 < CountOf(_buf)) {
        len += (uint32_t)::itoa(n, (char *)_buf + len);
        return true;
    }

    assert(0);
    return false;
}


LockedSizedStringWrapper::LockedSizedStringWrapper(int32_t n) {
    auto ss = intToSizedString(n);
    if (ss.len == 0) {
        len = (uint32_t)::itoa(n, (char *)_buf);
        data = _buf;
    } else {
        *(SizedString *)this = ss;
    }
}

LockedSizedStringWrapper::LockedSizedStringWrapper(double n) {
    len = floatToString(n, (char *)_buf);
    data = _buf;
}

LockedSizedStringWrapper::LockedSizedStringWrapper(const JsValue &v) {
    data = _buf;

    if (v.type == JDT_CHAR) {
        len = utf32CodeToUtf8(v.value.n32, data);
    } else if (v.type == JDT_UNDEFINED) {
        *(SizedString *)this = SS_UNDEFINED;
    } else if (v.type == JDT_NULL) {
        *(SizedString *)this = SS_NULL;
    } else if (v.type == JDT_BOOL) {
        *(SizedString *)this = v.value.n32 ? SS_TRUE : SS_FALSE;
    } else if (v.type == JDT_INT32) {
        auto n = v.value.index;
        auto ss = intToSizedString(n);
        if (ss.len == 0) {
            len = (uint32_t)::itoa(n, (char *)_buf);
            data = _buf;
        } else {
            *(SizedString *)this = ss;
        }
    } else {
        assert(0);
    }
}

LockedSizedStringWrapper::LockedSizedStringWrapper(const string &v) {
    data = _buf;
    len = min((int)v.size(), (int)CountOf(_buf));
    memcpy(_buf, v.c_str(), len);
}

void LockedSizedStringWrapper::clear() {
    len = 0;
    data = _buf;
}

void LockedSizedStringWrapper::reset(const SizedString &s) {
    *(SizedString *)this = s;
}

LockedSizedStringWrapper::LockedSizedStringWrapper(const LockedSizedStringWrapper &other) {
    len = other.len;
    if (other.data == other._buf) {
        memcpy(_buf, other._buf, other.len);
        data = _buf;
    } else {
        data = other.data;
        if (other.isStable()) {
            setStable();
        }
    }
}

LockedSizedStringWrapper &LockedSizedStringWrapper::operator =(const LockedSizedStringWrapper &other) {
    len = other.len;
    if (other.data == other._buf) {
        memcpy(_buf, other._buf, other.len);
        data = _buf;
    } else {
        data = other.data;
        if (other.isStable()) {
            setStable();
        }
    }

    return *this;
}
