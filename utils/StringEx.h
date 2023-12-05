#pragma once

#include "UtilsTypes.h"


#define isSpace             isspace

inline bool isEmptyString(const char *s) { return s == nullptr || s[0] == '\0'; }
inline void emptyStr(char *s) { s[0] = '\0'; }

inline bool strIsSame(cstr_t s1, cstr_t s2) { return strcmp(s1, s2) == 0; }
inline bool strIsISame(cstr_t s1, cstr_t s2) { return strcasecmp(s1, s2) == 0; }
bool isInArray(const string &str, const VecStrings &vStrs);

bool startsWith(cstr_t szText, cstr_t szBeginWith);
bool iStartsWith(cstr_t szText, cstr_t szBeginWith);

bool endsWith(cstr_t szText, cstr_t szEndWith);
bool iEndsWith(cstr_t szText, cstr_t szEndWith);

char *stristr(cstr_t source, cstr_t find);

const char * strignore(const char *str, const char *charsToIgnore);

size_t strlen_safe(const char * str, size_t maxLength);
char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource);
size_t strncpy_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy);
char *strcat_safe(char *dst, size_t size, const char *src);

char *strrep(char * str, char chSrc, char chDest);
void strrep(string &str, char chSrc, char chDest);
void strrep(string &str, const char *szSrc, const char *szDest);

void trimStr(string &str, cstr_t szChars = " \t\r\n");
void trimStr(string &str, char ch);
void trimStr(char * pszString, char ch = ' ');
void trimStr(VecStrings &vStrs, char ch = ' ');
void trimStrRight(char * pszString, cstr_t pszChars = " ");

void strSplit(const char *str, char sep, VecStrings &vStrs);
void strSplit(const char *str, char sep, VecStringViews &vStrs);
bool strSplit(const char *str, char sep, string &leftOut, string &rightOut);
bool strSplit(const char *str, const char *sep, string &leftOut, string &rightOut);

size_t itoa(int64_t value, char *buffer);
size_t itoa_ex(int64_t value, char *buffer, int radix);
inline string itos(int64_t value) { return std::to_string(value); }

enum FloatToStringFlags {
    F_TRIM_TAILING_ZERO         = 1,
    F_EXPONENTIAL_NOTATION      = 1 << 1,
    F_FIXED_DIGITS              = 1 << 2,
    F_FIXED_PRECISION           = 1 << 3,
};

uint32_t floatToStringEx(double value, char *buf, uint32_t bufSize, int32_t precisionCount = -1, uint32_t flags = F_TRIM_TAILING_ZERO);
uint32_t floatToStringWithRadix(double value, char *buf, size_t bufSize, int radix);
inline uint32_t floatToString(double value, char *buf) { return floatToStringEx(value, buf, 32); }

string stringPrintf(cstr_t format, ...);
string stringPrintf(cstr_t format, va_list args);

string toUpper(cstr_t szString);
string toLower(cstr_t szString);

inline int toLower(int ch) { return ('A' <= ch && ch <= 'Z') ? (ch + 'a' - 'A') : ch; }
inline int toUpper(int ch) { return ('a' <= ch && ch <= 'z') ? (ch + 'A' - 'a') : ch; }

inline bool isDigit(int ch) { return ('0' <= ch) && (ch <= '9'); }
inline bool isAlpha(int ch) { return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'); }
inline bool isAlphaDigit(int c) { return isAlpha(c) || isDigit(c); }
inline bool isWhiteSpace(int c) { return c == ' ' || c == '\t'; }
inline bool isHexChar(int ch) { return (isDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')); }
inline char hexToChar(int n) { return n <= 9 ? n + '0' : 'A' + n - 10; }

bool isNumeric(cstr_t szString);

string hexToStr(const uint8_t *data, size_t len);
inline string hexToStr(const StringView &data)
    { return hexToStr((uint8_t *)data.data, data.len); }

int hexToInt(int chHex);
uint32_t hexToInt(cstr_t str);

inline int toDigit(uint8_t ch) {
    if ('0' <= ch && ch <= '9') {
        return ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'z') {
        return ch - 'A' + 10;
    } else {
        return 255;
    }
}

inline cstr_t ignoreSpace(cstr_t str, cstr_t end) {
    while (str < end && isSpace(*str)) {
        str++;
    }

    return str;
}

template<typename _int>
inline cstr_t parseInt(cstr_t str, cstr_t end, _int &value) {
    bool isNegative = false;

    if (str + 1 < end && *str == '-') {
        if (!isDigit(str[1])) {
            // 下一位必须是数字，否则失败
            return str;
        }
        isNegative = true;
        str++;
    }

    value = 0;
    while (str < end && isDigit(*str)) {
        value *= 10;
        value += *str - '0';
        str++;
    }

    if (isNegative) {
        value = -value;
    }

    return str;
}

template<typename _int>
inline cstr_t parseInt(cstr_t str, _int &value) {
    bool bNegative = false;

    if (*str == '-') {
        bNegative = true;
        str++;
    }

    value = 0;
    while (isDigit(*str)) {
        value *= 10;
        value += *str - '0';
        str++;
    }

    if (bNegative) {
        value = -value;
    }

    return str;
}

void multiStrToVStr(cstr_t szText, VecStrings &vStr);

template<class _iterator>
string strJoin(_iterator first, _iterator end, cstr_t format, cstr_t separator) {
    string str;
    if (first != end) {
        str += stringPrintf(format, *first);
        ++first;
    }

    for (; first != end; ++first) {
        str += separator;
        str += stringPrintf(format, *first);
    }

    return str;
}

template<class _iterator>
string strJoin(_iterator first, _iterator end, cstr_t separator) {
    string str;
    if (first != end) {
        str += *first;
        ++first;
    }

    for (; first != end; ++first) {
        str += separator;
        str += *first;
    }

    return str;
}

inline void strJoin(string &target, cstr_t separator, const StringView &another) {
    if (!target.empty()) {
        target.append(separator);
    }

    target.append((char *)another.data, another.len);
}

template<class _iterator>
string strJoinConvert(_iterator first, _iterator end, cstr_t separator) {
    string str;
    if (first != end) {
        str += std::to_string(*first);
        ++first;
    }

    for (; first != end; ++first) {
        str += separator;
        str += std::to_string(*first);
    }

    return str;
}

int stringToInt(cstr_t str, int nDefault = 0);
COLORREF stringToColor(cstr_t szColor, COLORREF nDefault = 0);

string stringFromInt(int n);
string stringFromColor(COLORREF clr);

void stringFromColor(char szStr[], COLORREF clr);

template<class _TCHARPTR, class _int_t>
_TCHARPTR readInt_t(_TCHARPTR str, _int_t &value) {
    bool bNegative;

    value = 0;
    if (*str == '-') {
        bNegative = true;
        str++;
    } else {
        bNegative = false;
    }
    while (isDigit(*str)) {
        value *= 10;
        value += *str - '0';
        str++;
    }
    if (bNegative) {
        value = -value;
    }

    return str;
}

// szValue format: %d,%d,%d,%d
template<class TCHARDEF, class _int_t>
bool scan4IntX(TCHARDEF szValue, _int_t &n1, _int_t &n2, _int_t &n3, _int_t &n4) {
    szValue = readInt_t(szValue, n1);
    if (*szValue != ',') {
        return false;
    }
    szValue++;
    while (*szValue == ' ') {
        szValue++;
    }

    szValue = readInt_t(szValue, n2);
    if (*szValue != ',') {
        return false;
    }
    szValue++;
    while (*szValue == ' ') {
        szValue++;
    }

    szValue = readInt_t(szValue, n3);
    if (*szValue != ',') {
        return false;
    }
    szValue++;
    while (*szValue == ' ') {
        szValue++;
    }

    szValue = readInt_t(szValue, n4);

    return true;
}

// szValue format: %d,%d
template<class TCHARDEF, class _int_t>
bool scan2IntX(TCHARDEF szValue, _int_t &n1, _int_t &n2) {
    szValue = readInt_t(szValue, n1);
    if (*szValue != ',') {
        return false;
    }
    szValue++;
    while (*szValue == ' ') {
        szValue++;
    }

    readInt_t(szValue, n2);

    return true;
}

class SetStrLessICmp {
public:
    bool operator()(const string &str1, const string &str2) const {
        return strcasecmp(str1.c_str(), str2.c_str()) < 0;
    }
};

typedef std::set<string, SetStrLessICmp> SetICaseStr;

#ifdef _WIN32
#define SZ_NEW_LINE       "\r\n"
#else
#define SZ_NEW_LINE       "\n"

size_t wcslen(const WCHAR* str);

#endif
