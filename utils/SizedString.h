//
//  Created by HongyongXiao on 2021/11/12.
//

#ifndef SizedString_hpp
#define SizedString_hpp

#pragma once

#include <cstddef>
#include <stdint.h>
#include <memory.h>
#include <string>
#include <vector>
#include "Hash.h"


using namespace std;

using utf16_t = uint16_t;
using utf32_t = uint32_t;

class SizedString {
public:
    SizedString() { auto p = (uint64_t *)this; p[0] = 0; p[1] = 0; }
    SizedString(const SizedString &other) { *this = other; }
    SizedString(const char *data);
    SizedString(const string &s) : data((uint8_t *)s.c_str()), len((uint32_t)s.size()), _isStable(false) { }
    SizedString(const void *data, size_t len) : data((uint8_t *)data), len((uint32_t)len), _isStable(false) { }
    SizedString(size_t len, const void *data) : data((uint8_t *)data), len((uint32_t)len), _isStable(false) { }
    SizedString(const uint8_t *data, size_t len) : data((uint8_t *)data), len((uint32_t)len), _isStable(false) { }
    SizedString(const void *data, size_t len, bool isStable) : data((uint8_t *)data), len((uint32_t)len), _isStable(isStable) { }

    inline bool empty() const { return len == 0; }

    inline void setStable() { _isStable = true; }
    inline bool isStable() const { return _isStable; }

    bool isNumeric() const;

    uint8_t *strlchr(uint8_t c) const;
    uint8_t *strrchr(uint8_t c) const;

    int strstr(const SizedString &find, int32_t start = 0) const;
    int stristr(const SizedString &find) const;
    int strrstr(const SizedString &find, int32_t start = 0x7FFFFFFF) const;

    int cmp(const SizedString &other) const;
    int iCmp(const SizedString &other) const;

    inline bool equal(const char *other) const {
        return cmp(SizedString(other)) == 0;
    }

    inline bool equal(const SizedString &other) const {
        return cmp(other) == 0;
    }

    inline bool iEqual(const SizedString &other) const {
        return iCmp(other) == 0;
    }

    bool startsWith(const SizedString &with) const;
    bool iStartsWith(const SizedString &with) const;

    inline bool endsWith(const SizedString &with) const {
        if (len >= with.len) {
            return memcmp(data + len - with.len, with.data, with.len) == 0;
        }
        return false;
    }

    void trim(uint8_t charToTrim);
    void trim(const SizedString &toTrim);
    void trim();

    void shrink(int startShrinkSize, int endShrinkSize = 0);

    SizedString subStr(size_t offset, size_t size) const;
    SizedString subStr(const uint8_t *start, const uint8_t *end) const {
        assert(start <= end);
        assert(start >= data);
        assert(end <= data + len);
        return SizedString(start, (uint32_t)(end - start), _isStable);
    }
    SizedString subStr(const char *start, const char *end) const {
        return subStr((const uint8_t *)start, (const uint8_t *)end);
    }

    long atoi(bool &successful) const;
    SizedString itoa(long num, char *str) const;

    void toLowerCase();

    template <class _Container>
    void split(char chSeparator, _Container &container, int count = -1) {
        if (len == 0) {
            return;
        }

        const uint8_t *p, *start, *end;

        start = p = data;
        end = data + len;
        while (p < end && container.size() + 1 < (unsigned int)count) {
            while (p < end && *p != chSeparator) {
                p++;
            }

            if (p < end) {
                typename _Container::value_type tmp((const char *)start, size_t(p - start));
                container.push_back(tmp);
                p++;
            } else {
                typename _Container::value_type tmp((const char *)start, size_t(end - start));
                container.push_back(tmp);
                return;
            }

            start = p;
        }

        typename _Container::value_type tmp((const char *)start, size_t(end - start));
        container.push_back(tmp);
    }

    bool split(char chSeparator, SizedString &left, SizedString &right);
    bool split(const char *separator, SizedString &left, SizedString &right);

    string toString() { return string((const char *)data, len); }

    uint32_t offsetOf(const void *p) const { return (uint32_t)((uint8_t *)p - data); }

public:
    uint8_t                         *data;
    uint32_t                        len;

protected:
    friend class SizedStringUtf16;

    // 为 true 表示此字符串的 data 是稳定的 buffer，不会被释放.
    uint32_t                        _isStable : 1;

    // 给 SizedStringUtf16 的成员变量
    uint32_t                        _isAnsi : 1;
    uint32_t                        _unused1 : 1;
    uint32_t                        _lenUtf16 : 29;

};

static_assert(sizeof(SizedString) == 16);

/**
 * SizedString 保存的是 utf-8 编码, 而 dataUtf16 和 lenUtf16 保存的是 utf-16 编码
 */
class SizedStringUtf16 {
public:
    SizedStringUtf16() {
        _dataUtf16 = nullptr;
        setAnsi(true);
    }

    SizedStringUtf16(const utf16_t *dataUtf16, uint32_t lenUtf16) : _dataUtf16((uint16_t *)dataUtf16) {
        setUtf16Size(lenUtf16);
    }

    SizedStringUtf16(const SizedString &s) : _utf8Str(s) {
        onSetUtf8String();
    }

    void set(const SizedString &other) {
        _utf8Str = other;
        onSetUtf8String();
    }

    void setUtf16(const utf16_t *dataUtf16, uint32_t lenUtf16) {
        _dataUtf16 = (utf16_t *)dataUtf16;
        assert(lenUtf16 == size()); // _lenUtf16 = lenUtf16;
        setAnsi(false);
    }

    const SizedString &utf8Str() const { return _utf8Str; }

    inline uint32_t size() const { return _utf8Str._lenUtf16; }
    inline utf16_t *utf16Data() const { return _dataUtf16; }

    inline utf16_t chartAt(uint32_t index) const {
        assert(index < size());
        assert(canRandomAccess());
        return isAnsi() ? _utf8Str.data[index] : _dataUtf16[index];
    }

    utf32_t codePointAt(uint32_t index) const;

    inline bool canRandomAccess() const { return isAnsi() || _dataUtf16 != nullptr; }
    inline bool isAnsi() const { return _utf8Str._isAnsi; }
    inline bool isUtf16Valid() const { return _dataUtf16 != nullptr; }

    bool equal(uint32_t code) const;

    int indexOf(const SizedString &find, int32_t start = 0) const;
    int lastIndexOf(const SizedString &find, int32_t start = 0x7FFFFFFF) const;

protected:
    void onSetUtf8String();
    inline void setAnsi(bool isAnsi) { _utf8Str._isAnsi = isAnsi; }
    inline void setUtf16Size(uint32_t len) { _utf8Str._lenUtf16 = len; }

protected:
    SizedString                     _utf8Str;

    // _lenUtf16 是一定有效的，_dataUtf16 不一定有效
    utf16_t                         *_dataUtf16;

};


using VecSizedStrings = std::vector<SizedString>;
using VecSizedStringUtf16s = std::vector<SizedStringUtf16>;

#define MAKE_STABLE_STR(s)       SizedString(s, sizeof(s) - 1, true)

inline SizedString makeStableStr(const char *str) { return SizedString(str, (uint32_t)strlen(str), true); }

const SizedString sizedStringNull(nullptr, 0, true);

struct SizedStrCmpLess {

    bool operator()(const SizedString &a, const SizedString &b) const {
        return a.cmp(b) < 0;
    }

};

struct SizedStrCmpEqual {
    bool operator()(const SizedString &first, const SizedString &other) const {
        return first.equal(other);
    }
};


class SizedStringHash {
public:
    uint64_t operator()(const SizedString& s) const {
        return hashBytes(s.data, s.len);
    }
};

bool strIsInList(SizedString &str, SizedString *arr, size_t count);
bool IStrIsInList(SizedString &str, SizedString *arr, size_t count);

#endif /* SizedString_hpp */
