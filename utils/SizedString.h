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


class SizedString {
public:
    SizedString() { auto p = (uint64_t *)this; p[0] = 0; p[1] = 0; }
    SizedString(const char *data);
    SizedString(const string &s) : data((uint8_t *)s.c_str()), len((uint32_t)s.size()), _isStable(false) { }
    SizedString(const void *data, size_t len) : data((uint8_t *)data), len((uint32_t)len), _isStable(false) { }
    SizedString(size_t len, const void *data) : data((uint8_t *)data), len((uint32_t)len), _isStable(false) { }
    SizedString(const uint8_t *data, size_t len) : data((uint8_t *)data), len((uint32_t)len), _isStable(false) { }
    SizedString(const void *data, size_t len, bool isStable) : data((uint8_t *)data), len((uint32_t)len), _isStable(isStable) { }

    inline bool isStable() const { return _isStable; }

    uint8_t *strlchr(uint8_t c) const;
    uint8_t *strrchr(uint8_t c) const;

    int strStr(const SizedString &find) const;
    int strIStr(const SizedString &find) const;

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

public:
    uint8_t                         *data;
    uint32_t                        len;

private:
    uint8_t                         _unused[3];

    // 为 true 表示此字符串的 data 是稳定的 buffer，不会被释放.
    bool                            _isStable;

};

using VecSizedStrings = std::vector<SizedString>;

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
