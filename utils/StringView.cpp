//
//  Created by HongyongXiao on 2021/11/12.
//

#include "UtilsTypes.h"
#include "StringView.h"
#include "CharEncoding.h"
#include "StringEx.h"
#include <ctype.h>
#include <strings.h>
#include <climits>


StringView::StringView(const char *data) : data((char *)data), len((uint32_t)strlen((const char *)data)) {
}

bool StringView::isNumeric() const {
    auto end = data + len;
    auto p = data;

    for (; p < end; ++p) {
        if (!isDigit(*p)) {
            return false;
        }
    }

    return true;
}

bool StringView::isAnsi() const {
    auto end = (uint8_t *)data + len;
    auto p = (uint8_t *)data;

    for (; p < end; ++p) {
        if (*p > 127) {
            return false;
        }
    }

    return true;
}

int StringView::strchr(uint8_t c, int start) const {
    const auto *p = data + start, *last = data + len;

    while (p < last) {
        if (*p == c) {
            return (int)(p - data);
        }

        p++;
    }

    return -1;
}

int StringView::strrchr(uint8_t c, int startPos) const {
    const auto *start = data, *p = data + startPos;

    while (p >= start) {
        if (*p == c) {
            return (int)(p - data);
        }

        p--;
    }

    return -1;
}

int StringView::strstr(const StringView &find, int32_t start) const {
    if (len < find.len) {
        return -1;
    } else if (find.len == 0) {
        return 0;
    }

    const auto *last = data + len - find.len;
    const auto *s1 = data, *s2 = find.data;
    uint32_t len = find.len - 1;
    char c1, c2;

    if (start > 0) {
        s1 += start;
    }

    c2 = (uint8_t)*s2++;

    do {
        do {
            if (s1 > last) {
                return -1;
            }

            c1 = *s1++;
        } while (c1 != c2);

    } while (memcmp(s1, s2, len) != 0);

    return (int)(s1 - data - 1);
}

int StringView::stristr(const StringView &find, int32_t start) const {
    if (len < find.len) {
        return -1;
    } else if (find.len == 0) {
        return 0;
    }

    const auto *last = data + len - find.len;
    const auto *s1 = data + start, *s2 = find.data;
    uint32_t len = find.len - 1;
    char c1, c2;

    c2 = (uint8_t)*s2++;

    do {
        do {
            if (s1 > last) {
                return -1;
            }

            c1 = *s1++;
        } while (tolower((int)c1) != tolower((int)c2));

    } while (strncasecmp((char *)s1, (char *)s2, len) != 0);

    return (int)(s1 - data - 1);
}

int StringView::strrstr(const StringView &find, int32_t start) const {
    if (len < find.len) {
        return -1;
    } else if (find.len == 0) {
        return len;
    }

    const auto *s1 = data + len - find.len;
    const auto *begin = data, *s2 = find.data;
    uint32_t len = find.len - 1;
    char c1, c2;

    if (start > 0) {
        auto tmp = data + start;
        if (tmp < s1) {
            s1 = tmp;
        }
    } else {
        s1 = begin;
    }

    c2 = (uint8_t)*s2++;

    do {
        do {
            if (s1 < begin) {
                return -1;
            }

            c1 = *s1--;
        } while (c1 != c2);

    } while (memcmp(s1 + 2, s2, len) != 0);

    return (int)(s1 + 1 - data);
}

int StringView::cmp(const StringView &other) const {
    const uint8_t *p1 = (uint8_t *)data, *p1End = (uint8_t *)data + len;
    const uint8_t *p2 = (uint8_t *)other.data, *p2End = (uint8_t *)other.data + other.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        if (*p1 == *p2) {
            continue;
        }
        return *p1 - *p2;
    }

    if (p2 == p2End) {
        if (p1 == p1End) {
            return 0;
        }
        return 1;
    } else {
        return -1;
    }
}

int StringView::iCmp(const StringView &other) const {
    const uint8_t *p1 = (uint8_t *)data, *p1End = (uint8_t *)data + len;
    const uint8_t *p2 = (uint8_t *)other.data, *p2End = (uint8_t *)other.data + other.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        uint8_t c1 = *p1, c2 = *p2;

        if ('A' <= c1 && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if ('A' <= c2 && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }

        if (c1 == c2) {
            continue;
        }
        return c1 - c2;
    }

    if (p2 == p2End) {
        if (p1 == p1End) {
            return 0;
        }
        return 1;
    } else {
        return -1;
    }
}

bool StringView::startsWith(const StringView &with) const {
    if (len < with.len) {
        return false;
    }

    const auto *p1 = data, *p1End = data + len;
    const auto *p2 = with.data, *p2End = with.data + with.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        if (*p1 == *p2) {
            continue;
        }
        return false;
    }

    return p2 == p2End;
}

bool StringView::iStartsWith(const StringView &with) const {
    if (len < with.len) {
        return false;
    }

    const auto *p1 = data, *p1End = data + len;
    const auto *p2 = with.data, *p2End = with.data + with.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        uint8_t c1 = *p1, c2 = *p2;

        if ('A' <= c1 && c1 <= 'Z') {
            c1 += 'a' - 'A';
        }
        if ('A' <= c2 && c2 <= 'Z') {
            c2 += 'a' - 'A';
        }

        if (c1 == c2) {
            continue;
        }
        return false;
    }

    return p2 == p2End;
}

StringView StringView::trim(uint8_t charToTrim) const {
    auto *end = data + len;
    auto *p = data;

    while (p < end && *p == (uint8_t)charToTrim) {
        p++;
    }

    while (end > p && *(end - 1) == (uint8_t)charToTrim) {
        end--;
    }

    return StringView(p, uint32_t(end - p), _isStable);
}

StringView StringView::trim(const StringView &toTrim) const {
    // Trim from tail
    auto *start = data, *end = data + len;
    while (start < end) {
        if (toTrim.strchr(*(end - 1)) == -1) {
            break;
        }
        --end;
    }

    // Trim from head
    while (start < end) {
        if (toTrim.strchr(*(start)) == -1) {
            break;
        }
        ++start;
    }

    return StringView(start, (uint32_t)(end - start), _isStable);
}

StringView StringView::trim() const {
    return trim(stringViewBlanks);
}

StringView StringView::trimStart(const StringView &toTrim) const {
    auto *start = data, *end = data + len;
    // Trim from head
    while (start < end) {
        if (toTrim.strchr(*(start)) == -1) {
            break;
        }
        ++start;
    }

    return StringView(start, (uint32_t)(end - start), _isStable);
}

StringView StringView::trimEnd(const StringView &toTrim) const {
    // Trim from tail
    auto *start = data, *end = data + len;
    while (start < end) {
        if (toTrim.strchr(*(end - 1)) == -1) {
            break;
        }
        --end;
    }

    return StringView(start, (uint32_t)(end - start), _isStable);
}

void StringView::shrink(int startShrinkSize, int endShrinkSize) {
    data += startShrinkSize;
    if (startShrinkSize + endShrinkSize >= (int)len) {
        len = 0;
    } else {
        len -= startShrinkSize + endShrinkSize;
    }
}

StringView StringView::substr(uint32_t offset, uint32_t size) const {
    auto end = offset + size;
    if (end <= len && end >= size) {
        return StringView(data + offset, size, _isStable);
    } else if (offset <= len) {
        return StringView(data + offset, len - offset, _isStable);
    }

    return stringViewEmpty;
}

long StringView::atoi(bool &successful) const {
    long value, cutoff, cutlim;

    successful = false;
    if (len == 0) {
        return -1;
    }

    cutoff = LONG_MAX / 10;
    cutlim = LONG_MAX % 10;

    int n = (int)len;
    const auto *p = data;
    bool negative = false;

    if (*p == '-') {
        p++;
        negative = true;
        n--;
    } else if (*p == '+') {
        p++;
        n--;
    }

    for (value = 0; n--; p++) {
        if (*p < '0' || *p > '9') {
            return -1;
        }

        if (value >= cutoff && (value > cutoff || *p - '0' > cutlim)) {
            return -1;
        }

        value = value * 10 + (*p - '0');
    }

    if (negative) {
        value = -value;
    }

    successful = true;
    return value;
}

void reverse(char str[], size_t length) {
    char *p = str, *end = str + length - 1;
    while (p < end) {
        char tmp = *p;
        *p = *end;
        *end = tmp;

        p++;
        end--;
    }
}

StringView StringView::itoa(long num, char *str) const {
    uint32_t i = 0;
    bool isNegative = false;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return {i, (uint8_t *)str};
    }

    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    while (num != 0) {
        str[i++] = (num % 10) + '0';
        num = num / 10;
    }

    // append '-'
    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);

    return {i, (uint8_t *)str};
}

bool StringView::hasLowerCase() const {
    assert(!_isStable);

    uint8_t *p = (uint8_t *)data, *last = (uint8_t *)data + len;

    while (p < last) {
        if ('a' <= *p && *p <= 'z') {
            return true;
        }

        p++;
    }

    return false;
}

bool StringView::hasUpperCase() const {
    assert(!_isStable);

    uint8_t *p = (uint8_t *)data, *last = (uint8_t *)data + len;

    while (p < last) {
        if ('A' <= *p && *p <= 'Z') {
            return true;
        }

        p++;
    }

    return false;
}

void StringView::toLowerCase() {
    assert(!_isStable);

    uint8_t *p = (uint8_t *)data, *last = (uint8_t *)data + len;

    while (p < last) {
        if ('A' <= *p && *p <= 'Z') {
            *p += 'a' - 'A';
        }

        p++;
    }
}

bool StringView::split(char chSeparator, StringView &left, StringView &right) const {
    auto *p = data;
    auto *end = data + len;

    while (p < end && *p != chSeparator) {
        p++;
    }

    if (p >= end) {
        return false;
    }

    left = StringView(data, (uint32_t)(p - data), _isStable);
    right = StringView(p + 1, (uint32_t)(end - p - 1), _isStable);
    return true;
}

bool StringView::split(const char *separator, StringView &left, StringView &right) const {
    size_t lenSep = strlen(separator);
    const char *p = (const char *)data;
    const char *end = p + len - lenSep + 1;

    while (p < end) {
        if (*p == *separator && strncmp(p, separator, lenSep) == 0) {
            // Found it.
            auto lenOrg = len;
            left = StringView(data, (uint32_t)(p - data), _isStable);
            right = StringView((uint8_t *)p + lenSep, (uint32_t)(lenOrg - left.len - lenSep), _isStable);
            return true;
        }
        p++;
    }

    return false;
}

bool strIsInList(StringView &str, StringView *arr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str.equal(arr[i])) {
            return true;
        }
    }

    return false;
}

bool istrIsInList(StringView &str, StringView *arr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str.iEqual(arr[i])) {
            return true;
        }
    }

    return false;
}

bool StringViewUtf16::equal(uint32_t code) const {
    if (size() != 1) {
        return false;
    }

    if (_dataUtf16) {
        return *_dataUtf16 == code;
    }

    utf16_t buf[8];
    utf8ToUtf16((uint8_t *)_utf8Str.data, _utf8Str.len, buf, CountOf(buf));
    return buf[0] == code;
}

void StringViewUtf16::onSetUtf8String() {
    _dataUtf16 = nullptr;

    setUtf16Size(utf8ToUtf16Length((uint8_t *)_utf8Str.data, _utf8Str.len));
    setAnsi(size() == _utf8Str.len);
}

utf32_t StringViewUtf16::codePointAt(uint32_t index) const {
    assert(canRandomAccess());
    assert(index < size());

    if (isAnsi()) {
        return _utf8Str.data[index];
    }

    uint32_t code = _dataUtf16[index];
    if (code >= 0xd800 && code <= 0xdbff) {
        // Code points from the other planes (called Supplementary Planes) are encoded as two 16-bit code units called a surrogate pair,
        // https://en.wikipedia.org/wiki/UTF-16
        if (index + 1 < size()) {
            auto next = _dataUtf16[index + 1];
            if (next >= 0xdc00 && next <= 0xdfff) {
                code = 0x10000 + ((code - 0xd800) << 10) | (next - 0xdc00);
            }
        }
    }

    return code;
}

int StringViewUtf16::indexOf(const StringView &find, int32_t start) const {
    if (isAnsi()) {
        return _utf8Str.strstr(find, start);
    }

    int32_t pos;
    if (start > 0) {
        // 将 utf-16 的偏移转换为 utf-8 的
        auto p = utf8ToUtf16Seek((uint8_t *)_utf8Str.data, _utf8Str.len, start);
        pos = StringView(p, _utf8Str.len - (p - (uint8_t *)_utf8Str.data)).strstr(find);
        if (pos >= 0) {
            // 需要转换为 utf-16 的偏移地址
            pos = start + utf8ToUtf16Length(p, pos);
        }
    } else {
        pos = _utf8Str.strstr(find);
        if (pos >= 0) {
            // 需要转换为 utf-16 的偏移地址
            pos = utf8ToUtf16Length((uint8_t *)_utf8Str.data, pos);
        }
    }

    return pos;
}

int StringViewUtf16::lastIndexOf(const StringView &find, int32_t start) const {
    if (isAnsi()) {
        return _utf8Str.strrstr(find, start);
    }

    int32_t pos;
    if (start > 0) {
        // 将 utf-16 的偏移转换为 utf-8 的
        auto p = utf8ToUtf16Seek((uint8_t *)_utf8Str.data, _utf8Str.len, start);
        start = int32_t(p - (uint8_t *)_utf8Str.data);
    }

    pos = _utf8Str.strrstr(find, start);

    if (pos > 0) {
        // 需要转换为 utf-16 的偏移地址
        pos = utf8ToUtf16Length((uint8_t *)_utf8Str.data, pos);
    }

    return pos;
}

StringView StringViewUtf16::substr(uint32_t offset, uint32_t size) const {
    if (isAnsi()) {
        return _utf8Str.substr(offset, size);
    }

    if (offset >= _utf8Str._lenUtf16) {
        return stringViewEmpty;
    }

    auto start = utf8ToUtf16Seek((uint8_t *)_utf8Str.data, _utf8Str.len, offset);
    auto end = offset + size;
    if (end <= _utf8Str._lenUtf16 && end >= size) {
        auto end = utf8ToUtf16Seek(start, (uint32_t)((uint8_t *)_utf8Str.data + _utf8Str.len - start), size);
        return StringView(start, (uint32_t)(end - start), _utf8Str._isStable);
    } else {
        return StringView(start, (uint32_t)((uint8_t *)_utf8Str.data + _utf8Str.len - start), _utf8Str._isStable);
    }

    return stringViewEmpty;
}
