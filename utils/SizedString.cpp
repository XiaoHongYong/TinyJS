//
//  Created by HongyongXiao on 2021/11/12.
//

#include "UtilsTypes.h"
#include "SizedString.h"
#include <ctype.h>
#include <strings.h>
#include <climits>


SizedString::SizedString(const char *data) : data((uint8_t *)data), len((uint32_t)strlen((const char *)data)) {
}

uint8_t *SizedString::strlchr(uint8_t c) const {
    const uint8_t *p = data, *last = data + len;

    while (p < last) {
        if (*p == c) {
            return (uint8_t *)p;
        }

        p++;
    }

    return nullptr;
}

uint8_t *SizedString::strrchr(uint8_t c) const {
    const uint8_t *start = data, *p = data + len;

    if (len == 0) {
        return nullptr;
    }

    while (p >= start) {
        if (*p == c) {
            return (uint8_t *)p;
        }

        p--;
    }

    return nullptr;
}

int SizedString::strStr(const SizedString &find) const {
    if (len < find.len) {
        return -1;
    }
    if (find.len == 0) {
        return 0;
    }

    const uint8_t *last = data + len - find.len;
    const uint8_t *s1 = data, *s2 = find.data;
    size_t len = find.len - 1;
    uint8_t  c1, c2;

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

int SizedString::strIStr(const SizedString &find) const {
    if (len < find.len) {
        return -1;
    }

    const uint8_t *last = data + len - find.len;
    const uint8_t *s1 = data, *s2 = find.data;
    size_t len = find.len - 1;
    uint8_t  c1, c2;

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

int SizedString::cmp(const SizedString &other) const {
    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = other.data, *p2End = other.data + other.len;

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

int SizedString::iCmp(const SizedString &other) const {
    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = other.data, *p2End = other.data + other.len;

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

bool SizedString::startsWith(const SizedString &with) const {
    if (len < with.len) {
        return false;
    }

    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = with.data, *p2End = with.data + with.len;

    for (; p1 < p1End && p2 < p2End; p1++, p2++) {
        if (*p1 == *p2) {
            continue;
        }
        return false;
    }

    return p2 == p2End;
}

bool SizedString::iStartsWith(const SizedString &with) const {
    if (len < with.len) {
        return false;
    }

    const uint8_t *p1 = data, *p1End = data + len;
    const uint8_t *p2 = with.data, *p2End = with.data + with.len;

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

void SizedString::trim(uint8_t charToTrim) {
    uint8_t *end = data + len;
    uint8_t *p = data;

    while (p < end && *p == (uint8_t)charToTrim) {
        p++;
    }

    while (end > p && *(end - 1) == (uint8_t)charToTrim) {
        end--;
    }

    len -= (p - data);
    data = p;
}

void SizedString::trim(const SizedString &toTrim) {
    // Trim from tail
    uint8_t *start = data, *end = data + len;
    while (start < end) {
        if (toTrim.strlchr(*(end - 1)) == nullptr) {
            break;
        }
        --end;
    }

    // Trim from head
    while (start < end) {
        if (toTrim.strlchr(*(start)) == nullptr) {
            break;
        }
        ++start;
    }

    data = start;
    len = (uint32_t)(end - start);
}

void SizedString::trim() {
    static SizedString SPACES(" \t\r\n");
    trim(SPACES);
}

void SizedString::shrink(int startShrinkSize, int endShrinkSize) {
    data += startShrinkSize;
    if (startShrinkSize + endShrinkSize >= (int)len) {
        len = 0;
    } else {
        len -= startShrinkSize + endShrinkSize;
    }
}

SizedString SizedString::subStr(size_t offset, size_t size) const {
    if (offset + size <= len) {
        return SizedString(data + offset, size, _isStable);
    } else if (offset <= len) {
        return SizedString(data + offset, len - offset, _isStable);
    }

    return sizedStringNull;
}

long SizedString::atoi(bool &successful) const {
    long value, cutoff, cutlim;

    successful = false;
    if (len == 0) {
        return -1;
    }

    cutoff = LONG_MAX / 10;
    cutlim = LONG_MAX % 10;

    int n = (int)len;
    const uint8_t *p = data;
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

SizedString SizedString::itoa(long num, char *str) const {
    size_t i = 0;
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

void SizedString::toLowerCase() {
    assert(!_isStable);

    uint8_t *p = (uint8_t *)data, *last = (uint8_t *)data + len;

    while (p < last) {
        if ('A' <= *p && *p <= 'Z') {
            *p += 'a' - 'A';
        }

        p++;
    }
}

bool SizedString::split(char chSeparator, SizedString &left, SizedString &right) {
    uint8_t *p = data;
    uint8_t *end = data + len;

    while (p < end && *p != chSeparator) {
        p++;
    }

    if (p >= end) {
        return false;
    }

    left = SizedString(data, (uint32_t)(p - data), _isStable);
    right = SizedString(p + 1, (uint32_t)(end - p - 1), _isStable);
    return true;
}

bool SizedString::split(const char *separator, SizedString &left, SizedString &right) {
    size_t lenSep = strlen(separator);
    const char *p = (const char *)data;
    const char *end = p + len - lenSep + 1;

    while (p < end) {
        if (*p == *separator && strncmp(p, separator, lenSep) == 0) {
            // Found it.
            left = SizedString(data, (uint32_t)((uint8_t *)p - data), _isStable);
            right = SizedString((uint8_t *)p + lenSep, (uint32_t)(len - left.len - lenSep), _isStable);
            return true;
        }
        p++;
    }

    return false;
}

bool strIsInList(SizedString &str, SizedString *arr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str.equal(arr[i])) {
            return true;
        }
    }

    return false;
}

bool istrIsInList(SizedString &str, SizedString *arr, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (str.iEqual(arr[i])) {
            return true;
        }
    }

    return false;
}
