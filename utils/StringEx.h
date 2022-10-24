#pragma once

#include "UtilsTypes.h"


#define isSpace isspace

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

size_t wcslen(const WCHAR *str);

char *strrep(char * str, char chSrc, char chDest);
void strrep(string &str, char chSrc, char chDest);
void strrep(string &str, const char *szSrc, const char *szDest);

void trimStr(string &str, cstr_t szChars = " \t\r\n");
void trimStr(string &str, char ch);
void trimStr(char * pszString, char ch = ' ');
void trimStr(VecStrings &vStrs, char ch = ' ');
void trimStrRight(char * pszString, cstr_t pszChars = " ");

void strSplit(const char *str, char sep, VecStrings &vStrs);
bool strSplit(const char *str, char sep, string &leftOut, string &rightOut);
bool strSplit(const char *str, const char *sep, string &leftOut, string &rightOut);

size_t itoa(int64_t value, char *buffer);
size_t itoa(int64_t value, char *buffer, int radix);
string itos(int64_t value);

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

int hexToInt(int chHex);
uint32_t hexToInt(cstr_t str);

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

void multiStrToVStr(cstr_t szText, vector<string> &vStr);

template<class _iterator>
string strJoin(_iterator first, _iterator end, cstr_t format, cstr_t seperator) {
    string str;
    if (first != end) {
        str += stringPrintf(format, *first);
        ++first;
    }

    for (; first != end; ++first) {
        str += seperator;
        str += stringPrintf(format, *first);
    }

    return str;
}

template<class _iterator>
string strJoin(_iterator first, _iterator end, cstr_t seperator) {
    string str;
    if (first != end) {
        str += *first;
        ++first;
    }

    for (; first != end; ++first) {
        str += seperator;
        str += *first;
    }

    return str;
}

int stringToInt(cstr_t str, int nDefault = 0);
COLORREF stringToColor(cstr_t szColor, COLORREF nDefault = 0);

string stringFromInt(int n);
string stringFromColor(COLORREF clr);

void stringFromColor(char szStr[], COLORREF clr);

class SetStrLessICmp
{
public:
    bool operator()(const string &str1, const string &str2) const
    {
        return strcasecmp(str1.c_str(), str2.c_str()) < 0;
    }
};

typedef set<string, SetStrLessICmp>         SetICaseStr;
