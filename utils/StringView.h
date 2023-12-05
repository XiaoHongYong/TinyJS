//
//  Created by HongyongXiao on 2021/11/12.
//

#ifndef StringView_hpp
#define StringView_hpp

#pragma once

#include <cstddef>
#include <stdint.h>
#include <memory.h>
#include <assert.h>
#include <string>
#include <vector>
#include "Hash.h"

#ifdef _WIN32
using utf16_t = wchar_t;
#else // #ifdef _WIN32
using utf16_t = char16_t;
#endif

using utf32_t = char32_t;
using string = std::string;

class StringView {
public:
    StringView() { auto p = (uint64_t *)this; p[0] = 0; p[1] = 0; }
    StringView(const StringView &other) { *this = other; }
    StringView(const char *data);
    StringView(const string &s) : data((char *)s.c_str()), len((uint32_t)s.size()), _isStable(false) { }
    StringView(const void *data, size_t len) : data((char *)data), len((uint32_t)len), _isStable(false) { }
    StringView(size_t len, const void *data) : data((char *)data), len((uint32_t)len), _isStable(false) { }
    StringView(const uint8_t *data, size_t len) : data((char *)data), len((uint32_t)len), _isStable(false) { }
    StringView(const void *data, size_t len, bool isStable) : data((char *)data), len((uint32_t)len), _isStable(isStable) { }

    void assign(const void *data, size_t len, bool isStable = false) {
        this->data = (char *)data;
        this->len = (uint32_t)len;
        this->_isStable = isStable;
    }

    inline bool empty() const { return len == 0; }

    inline void setStable() { _isStable = true; }
    inline bool isStable() const { return _isStable; }

    bool isNumeric() const;
    bool isAnsi() const;

    int strchr(uint8_t c, int start = 0) const;
    int strrchr(uint8_t c, int start) const;
    int strrchr(uint8_t c) const { return strrchr(c, len - 1); }

    int strstr(const StringView &find, int32_t start = 0) const;
    int stristr(const StringView &find, int32_t start = 0) const;
    int strrstr(const StringView &find, int32_t start = 0x7FFFFFFF) const;

    int cmp(const StringView &other) const;
    int iCmp(const StringView &other) const;

    inline bool equal(const char *other) const {
        return cmp(StringView(other)) == 0;
    }

    inline bool equal(const StringView &other) const {
        return cmp(other) == 0;
    }

    inline bool iEqual(const StringView &other) const {
        return iCmp(other) == 0;
    }

    bool startsWith(const StringView &with) const;
    bool iStartsWith(const StringView &with) const;

    inline bool endsWith(const StringView &with) const {
        if (len >= with.len) {
            return memcmp(data + len - with.len, with.data, with.len) == 0;
        }
        return false;
    }

    StringView trim(uint8_t charToTrim) const;
    StringView trim(const StringView &toTrim) const;
    StringView trim() const;
    StringView trimStart(const StringView &toTrim) const;
    StringView trimEnd(const StringView &toTrim) const;

    void shrink(int startShrinkSize, int endShrinkSize = 0);

    StringView substr(uint32_t offset) const {
        if (offset < len) {
            return StringView(data + offset, len - offset, _isStable);
        }
        return StringView();
    }

    StringView substr(uint32_t offset, uint32_t size) const;
    StringView substr(const char *start, const char *end) const {
        assert(start <= end);
        assert(start >= data);
        assert(end <= data + len);
        return StringView(start, (uint32_t)(end - start), _isStable);
    }
    StringView substr(const uint8_t *start, const uint8_t *end) const {
        return substr((const char *)start, (const char *)end);
    }

    long atoi(bool &successful) const;
    StringView itoa(long num, char *str) const;

    bool hasLowerCase() const;
    bool hasUpperCase() const;
    void toLowerCase();

    template <class _Container>
    void splitLines(_Container &container) const {
        if (len == 0) {
            return;
        }

        const char *p = data, *start = data, *end = data + len;

        while (p < end) {
            while (p < end && *p != '\n' && *p != '\r') {
                p++;
            }

            if (p < end) {
                auto size = size_t(p - start);
                if (*p == '\r' && size + 1 < len && start[size + 1] == '\n') {
                    // \r\n 是一起的
                    p++;
                }

                typename _Container::value_type tmp((const char *)start, size);
                container.push_back(tmp);
                p++;
            } else {
                auto size = size_t(end - start);
                typename _Container::value_type tmp((const char *)start, size);
                container.push_back(tmp);
                return;
            }

            start = p;
        }

        typename _Container::value_type tmp((const char *)start, size_t(end - start));
        container.push_back(tmp);
    }

    template <class _Container>
    void split(char chSeparator, _Container &container, int count = -1) const {
        if (len == 0) {
            return;
        }

        const char *p = data, *start = data, *end = data + len;

        while (p < end && count != 0) {
            while (p < end && *p != chSeparator) {
                p++;
            }

            if (p < end) {
                typename _Container::value_type tmp((const char *)start, size_t(p - start));
                container.push_back(tmp);
                count--;
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

    bool split(char chSeparator, StringView &left, StringView &right) const;
    bool split(const char *separator, StringView &left, StringView &right) const;

    string toString() const { return string(data, len); }
    void toString(string &outStr) const { outStr.assign(data, len); }

    uint32_t offsetOf(const void *p) const { return (uint32_t)((char *)p - data); }

public:
    char                        *data;
    uint32_t                    len;

protected:
    friend class StringViewUtf16;

    // 为 true 表示此字符串的 data 是稳定的 buffer，不会被释放.
    uint32_t                    _isStable : 1;

    // 给 StringViewUtf16 的成员变量
    uint32_t                    _isAnsi : 1;
    uint32_t                    _unused1 : 1;
    uint32_t                    _lenUtf16 : 29;

};

static_assert(sizeof(StringView) == 16, "Expected same size.");

/**
 * StringView 保存的是 utf-8 编码, 而 dataUtf16 和 lenUtf16 保存的是 utf-16 编码
 */
class StringViewUtf16 {
public:
    StringViewUtf16() {
        _dataUtf16 = nullptr;
        setAnsi(true);
    }

    StringViewUtf16(const utf16_t *dataUtf16, uint32_t lenUtf16) : _dataUtf16((utf16_t *)dataUtf16) {
        setUtf16Size(lenUtf16);
    }

    StringViewUtf16(const StringView &s) : _utf8Str(s) {
        onSetUtf8String();
    }

    StringViewUtf16(const uint8_t *data, uint32_t len, bool isAnsi) : _utf8Str(data, len) {
        if (isAnsi) {
            _dataUtf16 = nullptr;
            _utf8Str._lenUtf16 = len;
            _utf8Str._isAnsi = true;
        } else {
            onSetUtf8String();
        }
    }

    void set(const StringView &other) {
        _utf8Str = other;
        onSetUtf8String();
    }

    void setUtf16(const utf16_t *dataUtf16, uint32_t lenUtf16) {
        _dataUtf16 = (utf16_t *)dataUtf16;
        assert(lenUtf16 == size()); // _lenUtf16 = lenUtf16;
        setAnsi(false);
    }

    const StringView &utf8Str() const { return _utf8Str; }

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

    int indexOf(const StringView &find, int32_t start = 0) const;
    int lastIndexOf(const StringView &find, int32_t start = 0x7FFFFFFF) const;

    StringView substr(uint32_t offset, uint32_t size) const;

protected:
    void onSetUtf8String();
    inline void setAnsi(bool isAnsi) { _utf8Str._isAnsi = isAnsi; }
    inline void setUtf16Size(uint32_t len) { _utf8Str._lenUtf16 = len; }

protected:
    StringView                 _utf8Str;

    // _lenUtf16 是一定有效的，_dataUtf16 不一定有效
    utf16_t                     *_dataUtf16;

};


using VecStringViews = std::vector<StringView>;
using VecStringViewUtf16s = std::vector<StringViewUtf16>;

#define MAKE_STABLE_STR(s)  StringView(s, sizeof(s) - 1, true)

inline StringView makeStableStr(const char *str) { return StringView(str, (uint32_t)strlen(str), true); }

const StringView stringViewEmpty(nullptr, 0, true);
const StringView stringViewBlanks(" \t\r\n");

struct SizedStrCmpLess {

    bool operator()(const StringView &a, const StringView &b) const {
        return a.cmp(b) < 0;
    }

};

struct SizedStrCmpEqual {
    bool operator()(const StringView &first, const StringView &other) const {
        return first.equal(other);
    }
};


class StringViewHash {
public:
    uint64_t operator()(const StringView& s) const {
        return hashBytes(s.data, s.len);
    }
};

bool strIsInList(StringView &str, StringView *arr, size_t count);
bool IStrIsInList(StringView &str, StringView *arr, size_t count);

inline void operator+=(std::string &str, const StringView &other) {
    str.append(other.data, other.len);
}

namespace testing {
inline std::string PrintToString(const StringView &value) {
    return value.toString();
}
}

#endif /* StringView_hpp */
