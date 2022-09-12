#include <math.h>
#include "UtilsTypes.h"
#include "StringEx.h"


//
//int isCharWordBoundary(WCHAR c) {
//    return ((c >= 0x4E00 && c <= 0x9FFF)
//        || (c >= 0x3000 && c <= 0x303F) // Chinese punctuation
//        || (c >= 0x3400 && c <= 0x4DFF)
//        || (c >= 0x20000 && c <= 0x2A6DF)
//        || (c >= 0xF900 && c <= 0xFAFF)
//        || (c >= 0xF900 && c <= 0xFAFF)
//        || (c >= 0x2F800 && c <= 0x2FA1F)
//        || (!isAlphaDigit(c) && c != '\'' && c != '-' && c != '_'));
//}

bool isInArray(const string &str, const VecStrings &vStrs) {
    for (auto &s : vStrs) {
        if (s == str) {
            return true;
        }
    }

    return false;
}

bool startsWith(cstr_t szText, cstr_t szBeginWith)
{
    while (*szBeginWith == *szText && *szBeginWith && *szText)
    {
        szBeginWith++;
        szText++;
    }

    return *szBeginWith == '\0';
}


bool iStartsWith(cstr_t szText, cstr_t szBeginWith)
{
    while (toLower(*szBeginWith) == toLower(*szText) && *szBeginWith && *szText)
    {
        szBeginWith++;
        szText++;
    }

    return *szBeginWith == '\0';
}

bool endsWith(cstr_t szText, cstr_t szEndWith)
{
    size_t lenText = strlen(szText);
    size_t lenEndWith = strlen(szEndWith);

    if (lenText < lenEndWith)
        return false;

    return strcmp(szText + lenText - lenEndWith, szEndWith) == 0;
}

bool iEndsWith(cstr_t szText, cstr_t szEndWith)
{
    size_t lenText = strlen(szText);
    size_t lenEndWith = strlen(szEndWith);

    if (lenText < lenEndWith)
        return false;

    return strcasecmp(szText + lenText - lenEndWith, szEndWith) == 0;
}

char *stristr(cstr_t source, cstr_t find) {
    SizedString str(source);
    int pos = str.strIStr(SizedString(find));
    if (pos == -1) {
        return nullptr;
    }

    return (char *)source + pos;
}

size_t strlen_safe(const char * str, size_t maxLength)
{
	size_t n = 0;
	while (n < maxLength && str[n])
		n++;

	return n;
}

char *strcpy_safe(char *strDestination, size_t nLenMax, const char *strSource)
{
    char    *cp = strDestination;
    char    *cpEnd = strDestination + nLenMax - 1;

    for (; cp < cpEnd && *strSource; cp++, strSource++)
    {
        *cp = *strSource;
    }

    *cp = '\0';

    return(strDestination);
}

size_t strncpy_safe(char *strDestination, size_t nLenMax, const char *strSource, size_t nToCopy)
{
    if (nToCopy >= nLenMax)
        nToCopy = nLenMax - 1;

    size_t        org = nToCopy;

    while (nToCopy && (*strDestination++ = *strSource++))    /* copy string */
            nToCopy--;

    *strDestination = '\0';

    return org - nToCopy;
}

size_t wcslen(const WCHAR *str) {
    auto p = str;
    while (*p) {
        p++;
    }

    return size_t(p - str);
}

void strrep(string &str, const char *szSrc, const char *szDest)
{
    size_t        nPos;
    size_t        nLenSrc = strlen(szSrc);
    size_t        nLenDest = strlen(szDest);
    cstr_t    szPos;

    nPos = 0;
    while (nPos < str.size())
    {
        szPos = strstr(str.c_str() + nPos, szSrc);
        if (szPos)
        {
            nPos = size_t(szPos - str.c_str());
            str.erase(str.begin() + nPos, str.begin() + nPos + nLenSrc);
            str.insert(str.begin() + nPos, szDest, szDest + nLenDest);
            nPos += nLenDest;
        }
        else
            break;
    }
}

char * __cdecl strrep(char * str, char chSrc, char chDest)
{
    char *cp = (char *) str;
    while (*cp)
    {
        if (*cp == chSrc)
            *cp = chDest;
        cp++;
    }

    return str;
}

void strrep(string &str, char chSrc, char chDest)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == chSrc)
            str.replace(i, 1, 1, chDest);
    }
}

#ifndef WIN32
char *_strupr(char *str)
{
    char    *s = str;
    while (*s)
    {
        int        f = (unsigned char)(*(s));
        if ( (f >= 'a') && (f <= 'z') )
        {
            *s -= ('a' - 'A');
        }
        s++;
    }
    return str;
}
#endif

const char * strignore(const char *str, const char *strIgnore)
{
    const char *cp = (char *) str;
    while (*cp)
    {
        const char *cp2 = strIgnore;
        while (*cp2 && *cp2 != *cp)
            cp2++;
        if (*cp2 == '\0')
            return cp;
        cp++;
    }

    return cp;
}

void strdel(string &str, char ch)
{
    for (size_t i = 0; i < str.size(); )
    {
        if (str[i] == ch)
            str.erase(i, 1);
        else
            i++;
    }
}

void strdel(char *szStr, char ch)
{
    char    *szPos;
    char    *szPos2;
    szPos = szStr;
    szPos2 = szStr;
    while (*szPos)
    {
        if (*szPos != ch)
        {
            *szPos2 = *szPos;
            szPos2++;
        }
        szPos++;
    }
    *szPos2 = '\0';
}


char * __cdecl strchrtill (
        const char * string,
        int ch,
        int chTill
        )
{
        while (*string && *string != (char)ch && *string != (char)chTill)
                string++;

        if (*string == (char)chTill)
            return nullptr;

        if (*string == (char)ch)
                return((char *)string);
        return(nullptr);
}

void trimStr(string &str, cstr_t szChars)
{
    string::iterator    it;
    long        i;

    for (i = str.size() - 1; i >= 0; i--)
    {
        if (!strchr(szChars, str[i]))
            break;
    }
    if (i != str.size() - 1)
        str.erase(str.begin() + 1 + i, str.end());

    for (it = str.begin(); it != str.end(); it++)
    {
        if (!strchr(szChars, *it))
            break;
    }
    if (it != str.begin())
        str.erase(str.begin(), it);
}

void trimStr(string &str, char ch)
{
    string::iterator    it;
    long        i;

    for (i = str.size() - 1; i >= 0; i--)
    {
        if (str[i] != ch)
            break;
    }
    if (i != str.size() - 1)
        str.erase(str.begin() + 1 + i, str.end());

    for (it = str.begin(); it != str.end(); it++)
    {
        if (*it != ch)
            break;
    }
    if (it != str.begin())
        str.erase(str.begin(), it);
}

//
// 去掉字符串两端的空格
void trimStr(char * szString, char ch)
{
    char *    szBeg;
    szBeg = szString;

    while (*szBeg == ch)
        szBeg++;

    long nLen = strlen(szBeg);
    if (nLen > 0)
    {
        while (szBeg[nLen - 1] == ch)
            nLen--;
        szBeg[nLen] = '\0';
    }

    if (szBeg != szString)
        memmove(szString, szBeg, sizeof(char) * (nLen + 1));
}

void trimStrRight(char * pszStr, cstr_t pszChars/* = " "*/)
{
    if (nullptr == pszStr || nullptr == pszChars)
        return;

    char * p = pszStr + strlen(pszStr) - 1;
    while (p >= pszStr)
    {
        cstr_t pCh = pszChars;

        // Matching taget char set.
        while (*pCh)
        {
            if (*pCh == *p)
            {
                *p = 0;
                break;
            }
            ++pCh;
        }

        // Not matched?
        if (*p != 0)
            return;
        --p;
    }
}

void trimStr(VecStrings &vStrs, char ch) {
    for (auto &s : vStrs) {
        trimStr(s, ch);
    }
}

void strSplit(const char *str, char sep, VecStrings &vStrs) {
    SizedString(str).split(sep, vStrs);
}

bool strSplit(const char *str, char sep, string &leftOut, string &rightOut) {
    SizedString s(str), left, right;

    if (s.split(sep, left, right)) {
        leftOut.assign((const char *)left.data, left.len);
        rightOut.assign((const char *)right.data, right.len);
        return true;
    }

    return false;
}

bool strSplit(const char *str, const char *sep, string &leftOut, string &rightOut) {
    SizedString s(str), left, right;

    if (s.split(sep, left, right)) {
        leftOut.assign((const char *)left.data, left.len);
        rightOut.assign((const char *)right.data, right.len);
        return true;
    }

    return false;
}

size_t u64toa(uint64_t num, char *buffer) {
    size_t i = 0;

    do {
        buffer[i++] = (num % 10) + '0';
        num = num / 10;
    } while (num != 0);

    // '\0' end
    buffer[i] = '\0';

    // Reverse buffer
    char *p = buffer, *end = buffer + i - 1;
    while (p < end) {
        char tmp = *p;
        *p = *end;
        *end = tmp;

        p++;
        end--;
    }

    return i;
}

size_t itoa(int64_t num, char *buffer) {
    if (num < 0) {
        buffer[0] = '-';
        return u64toa((uint64_t)-num, buffer + 1) + 1;
    }

    return u64toa((uint64_t)num, buffer);
}

static const char NUM_TABLE[] = "0123456789abcdefghijklmnopqrstuvwxyz";

size_t itoa(int64_t num, char *buffer, int radix) {
    if (radix < 0 || radix >= CountOf(NUM_TABLE)) {
        return 0;
    }

    bool isNegative = false;
    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    size_t i = 0;

    do {
        buffer[i++] = NUM_TABLE[num % radix];
        num = num / radix;
    } while (num != 0);

    // append '-'
    if (isNegative) {
        buffer[i++] = '-';
    }

    // '\0' end
    buffer[i] = '\0';

    // Reverse buffer
    char *p = buffer, *end = buffer + i - 1;
    while (p < end) {
        char tmp = *p;
        *p = *end;
        *end = tmp;

        p++;
        end--;
    }

    return i;
}

string itos(int64_t value) {
    char buf[32];

    size_t n = itoa(value, buf);

    return string(buf, n);
}

uint32_t dealDecimalPointPlusCount(double &value) {
    if (value <= 0xFFFFFFFFFFFFFFFFL) {
        // 0xFFFFFFFFFFFFFFFFL == 1.8446744073709551e19
        return 0;
    }

    // 超过的数转换为 e 表示的浮点数
    uint32_t count = 0;

    while (value >= 1e50) {
        count += 50;
        value /= 1e50;
    }

    while (value >= 1e10) {
        count += 10;
        value /= 1e10;
    }

    while (value >= 10.0) {
        value /= 10.0;
        count += 1;
    }

    // count 至少是 19 位
    assert(count >= 19);
    if (count < 20 || (count == 20 && value <= 9.999999999999999)) {
        // 小于 9.999999999999999e20 的数转换为
        count -= 18;
        value *= 1e18;
    }

    return count;
}

uint32_t dealDecimalPointSubCount(double &value) {
    double valueOrg = value;

    uint32_t count = 0;
    while (value <= 1e-50) {
        count += 50;
        value *= 1e50;
    }

    while (value <= 1e-10) {
        count += 10;
        value *= 1e10;
    }

    while (value < 1.0) {
        value *= 10.0;
        count += 1;
    }

    if (count <= 6) {
        // count 至少是 6 位
        value = valueOrg;
        return 0;
    }

    return count;
}

/**
 * 将 double 转化为字符串，截断为 0 的浮点部分，比如:
 *     10.00     => 10
 *     1.11000 => 1.11
 */
uint32_t floatToString(double value, char *buf) {
    uint32_t i = 0;

    if (isnan(value)) {
        strcpy(buf + i, "NaN");
        return (int)i + 3;
    }

    if (signbit(value)) {
        value = -value;
        buf[i++] = '-';
    }

    if (isinf(value)) {
        strcpy(buf + i, "Infinity");
        return (int)i + 8;
    }

    const uint32_t PRECISION = 16;
    uint32_t precisionCount = PRECISION;
    uint32_t zeroCount = 0;

    uint32_t decimalPointCount = dealDecimalPointPlusCount(value);

    if (decimalPointCount == 0 && value > 0 && value < 1) {
        // 快速确定 zeroCount
        zeroCount = dealDecimalPointSubCount(value);
    }

    auto tmp = u64toa((uint64_t)value, buf + i);
    i += tmp;
    if ((int64_t)value > 0) {
        precisionCount -= tmp;
    }

    if (decimalPointCount > 0 && decimalPointCount < 10) {
        // > 0xFFFFFFFFFFFFFFFF && < 9.999999999999999e20 的数，采用正整数的显示方式
        while (decimalPointCount-- > 0) {
            buf[i++] = '0';
        }
        buf[i] = '\0';
        return i;
    }

    value -= (uint64_t)value;

    // zeroPos 用于记录最后一个非 0 的位置
    uint32_t zeroPos = i;

    if (value > 0) {
        buf[i++] = '.';

        while (precisionCount > 0) {
            value *= 10;
            int n = int(value);
            buf[i++] = '0' + n;
            if (n > 0) {
                zeroPos = i;
                value -= n;

                precisionCount--;
            } else if (precisionCount < PRECISION) {
                precisionCount--;
            }
        }
    }

    if (decimalPointCount > 0) {
        buf[zeroPos++] = 'e';
        buf[zeroPos++] = '+';
        auto tmp = u64toa(decimalPointCount, buf + zeroPos);
        zeroPos += tmp;
    } else if (zeroCount > 0) {
        buf[zeroPos++] = 'e';
        buf[zeroPos++] = '-';
        auto tmp = u64toa(zeroCount, buf + zeroPos);
        zeroPos += tmp;
    }

    // '\0' end
    buf[zeroPos] = '\0';

    return (uint32_t)zeroPos;
}

/**
 * 将 double 转化为字符串，截断为 0 的浮点部分，比如:
 *     10.00     => 10
 *     1.11000 => 1.11
 */
uint32_t floatToString(double value, char *buf, int radix) {
    size_t i = 0;
    size_t LOOP_COUNT = 18;

    if (value < 0) {
        value = -value;
        buf[i++] = '-';
        LOOP_COUNT++;
    }

    if (isinf(value)) {
        strcpy(buf + i, "inf");
        return (int)i + 3;
    }

    i += itoa((int64_t)value, buf + i, radix);
    value -= (int64_t)value;

    // zeroPos 用于记录最后一个非 0 的位置
    size_t zeroPos = i;

    buf[i++] = '.';

    while (i < LOOP_COUNT) {
        value *= radix;
        int n = int(value);
        buf[i++] = NUM_TABLE[n];
        if (n > 0) {
            zeroPos = i;
            value -= n;
        }
    }

    // '\0' end
    buf[zeroPos] = '\0';

    return (uint32_t)zeroPos;
}

string stringPrintf(cstr_t format, ...) {
    va_list args;

    va_start(args, format);

    auto str = stringPrintf(format, args);

    return str;
}

string stringPrintf(cstr_t format, va_list args) {
    string str;
    str.resize(256);

    while (1) {
        auto capacity = str.size();
        int size = vsnprintf(str.data(), capacity, format, args);

        if (size < 0) {
            if (capacity >= 100 * 1024)
                break;

            str.resize(capacity * 2);
        } else {
            str.resize(size);
            break;
        }
    }

    return str;
}

string toUpper(cstr_t szString)
{
    string        str = szString;

    for (int i = 0; szString[i] != '\0'; i ++)
        if ('a' <= szString[i]  && szString[i] <= 'z')
            str[i] += 'A' - 'a';

    return str;
}

string toLower(cstr_t szString)
{
    string        str = szString;

    str = szString;
    for (int i = 0; szString[i] != '\0'; i ++)
        if ('A' <= szString[i]  && szString[i] <= 'Z')
            str[i] += 'a' - 'A';

    return str;
}

// Return true if:
// * Has at least one number.
// * No other chars.
bool isNumeric(cstr_t szString)
{
    cstr_t szTemp = szString;

    while (isDigit(*szTemp))
        szTemp++;

    if (*szTemp != '\0')
        return false;

    // empty String
    if (szString == szTemp)
        return false;

    return true;
}

int hexToInt(int chHex) {
    if (isDigit(chHex)) {
        return chHex - '0';
    } else if (chHex >= 'a' && chHex <= 'f')
        return (chHex - 'a' + 10);
    else if (chHex >= 'A' && chHex <= 'F')
        return (chHex - 'A' + 10);
    else
        return 0;
}

uint32_t hexToInt(cstr_t str)
{
    if (strncasecmp(str, "0x", 2) == 0)
        str += 2;

    size_t len = strlen(str);
    uint32_t    val = 0;

    for (size_t i = 0; i < len; i++)
    {
        if ('0' <= str[i] && str[i] <= '9')
            val = val * 16 + str[i] - '0';
        else if ('a' <= str[i] && str[i] <= 'f')
            val = val * 16 + str[i] - 'a' + 10;
        else if ('A' <= str[i] && str[i] <= 'F')
            val = val * 16 + str[i] - 'A' + 10;
    }

    return val;
}

string hexToStr(const uint8_t *data, size_t len) {
    string str;
    str.resize(len * 2);
    char *p = (char *)str.data();
    for (size_t i = 0; i < len; i++) {
        *p++ = hexToChar(data[i] / 16);
        *p++ = hexToChar(data[i] % 16);
    }

    return str;
}

void multiStrToVStr(cstr_t szText, vector<string> &vStr)
{
    while (*szText)
    {
        vStr.push_back(szText);
        while (*szText != '\0')
            szText++;
        szText++;
    }
}

// "#FFFFFF"
template<class _TCHARPTR, class _int_t>
_TCHARPTR readColorValue_t(_TCHARPTR szColor, _int_t &nClr)
{
    nClr = 0;

    if (*szColor == '#')
        szColor++;

    int        n;

    for (int i = 0; i < 6 && *szColor; i++)
    {
        if (*szColor >= '0' && *szColor <= '9')
            n = *szColor - '0';
        else if (*szColor >= 'a' && *szColor <= 'f')
            n = 10 + *szColor - 'a';
        else if (*szColor >= 'A' && *szColor <= 'F')
            n = 10 + *szColor - 'A';
        else
            break;

        nClr = nClr * 16 + n;
        szColor++;
    }

    nClr = ((nClr & 0xFF) << 16) + (nClr & 0xFF00) + ((nClr & 0xFF0000) >> 16);

    return szColor;
}

int stringToInt(cstr_t str, int nDefault)
{
    int value = 0;
    cstr_t end = parseInt(str, value);
    if (end == str)
        return nDefault;

    return value;
}

// "#FFFFFF"
COLORREF stringToColor(cstr_t szColor, COLORREF nDefault)
{
    if (isEmptyString(szColor))
        return nDefault;

    uint32_t        clr;
    cstr_t p = readColorValue_t(szColor, clr);
    if (*p != '\0')
        return nDefault;
    else
        return clr;
}

string stringFromInt(int value)
{
    return itos(value);
}

string stringFromColor(COLORREF clr)
{
    char    szColor[32];

    stringFromColor(szColor, clr);
    return szColor;
}

void stringFromColor(char szStr[], COLORREF clr)
{
    sprintf(szStr, "#%02X%02X%02X", clr & 0xFF, (clr >> 8) & 0xff, (clr >> 16) & 0xff);
}

#if UNIT_TEST

#include "utils/unittest.h"

TEST(StringEx, floatToString) {
    char buf[64];
    uint32_t len;

    len = floatToString(1 / 2.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("0.5"));

    len = floatToString(1.5 * 20.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("30"));

    len = floatToString(1e-7, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("1e-7"));

    len = floatToString(1e-6, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("0.000001"));

    len = floatToString(9.999e-6, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("0.000009999"));

    len = floatToString(1.332e-10, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("1.332e-10"));

    len = floatToString(4.940656458412468e-324, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("4.940656458412468e-324"));

    len = floatToString(9.999999999999981568e20, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("999999999999998156800"));

    len = floatToString(1e21, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("1e+21"));

    // 1.7976931348623157e+308
    len = floatToString(1.7976931348623157e+308, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("1.797693134862314e+308"));

    len = floatToString(-9.2233720368547758E+18, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("-9223372036854775808"));

    len = floatToString(1 / 3.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("0.3333333333333333"));

    len = floatToString(1 / 300.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("0.003333333333333333"));

    len = floatToString(-1 / 2.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("-0.5"));

    len = floatToString(-1.5 * 20.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("-30"));

    len = floatToString(-1 / 3.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("-0.3333333333333333"));

    len = floatToString(-1 / 300.0, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("-0.003333333333333333"));

    len = floatToString(INFINITY, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("Infinity"));

    len = floatToString(-INFINITY, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("-Infinity"));

    len = floatToString(NAN, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("NaN"));

    len = floatToString(-NAN, buf);
    ASSERT_TRUE(SizedString(buf, len).equal("NaN"));
}

TEST(StringEx, testTrimStrRight) {
    char szStr[1024] = "abcdef1213   ";
    trimStrRight(szStr);
    ASSERT_TRUE(strcmp(szStr, "abcdef1213") == 0);
    trimStrRight(szStr);
    ASSERT_TRUE(strcmp(szStr, "abcdef1213") == 0);
    trimStrRight(szStr, "314");
    ASSERT_TRUE(strcmp(szStr, "abcdef12") == 0);
    trimStrRight(szStr, "cd21f");
    ASSERT_TRUE(strcmp(szStr, "abcde") == 0);
    trimStrRight(szStr, "cd21f");
    ASSERT_TRUE(strcmp(szStr, "abcde") == 0);
    trimStrRight(szStr, "cde21f");
    ASSERT_TRUE(strcmp(szStr, "ab") == 0);
    trimStrRight(szStr, " ab+1234fo");
    ASSERT_TRUE(strcmp(szStr, "") == 0);
}

TEST(StringEx, testReadInt_t) {
    int            value;
    cstr_t        p;

    p = parseInt("10", value);
    ASSERT_TRUE(*p == '\0' && value == 10);

    p = parseInt("-110", value);
    ASSERT_TRUE(*p == '\0' && value == -110);

    p = parseInt("99c", value);
    ASSERT_TRUE(*p == 'c' && value == 99);

    p = parseInt("", value);
    ASSERT_TRUE(*p == '\0' && value == 0);

    p = parseInt("a1bc", value);
    ASSERT_TRUE(*p == 'a' && value == 0);
}

TEST(StringEx, testReadColorValue_t) {
    COLORREF    clr;
    cstr_t        p;

    p = readColorValue_t("#576C91", clr);
    ASSERT_TRUE(*p == '\0' && clr == RGB(0x57, 0x6c, 0x91));

    p = readColorValue_t("#576C91a", clr);
    ASSERT_TRUE(*p == 'a' && clr == RGB(0x57, 0x6c, 0x91));

    p = readColorValue_t("#76C91", clr);
    ASSERT_TRUE(*p == '\0' && clr == RGB(0x7, 0x6c, 0x91));

    p = readColorValue_t("576C91", clr);
    ASSERT_TRUE(*p == '\0' && clr == RGB(0x57, 0x6c, 0x91));

    p = readColorValue_t("576C910", clr);
    ASSERT_TRUE(*p == '0' && clr == RGB(0x57, 0x6c, 0x91));
}

TEST(StringEx, testStrSplit) {
    VecStrings        vStr;
    cstr_t        szInput = "a,ab,a10,,A,";
    cstr_t        strResult[] = { "a", "ab", "a10", "", "A", "" };

    strSplit(szInput, ',', vStr);

    ASSERT_TRUE(vStr.size() == CountOf(strResult));

    for (int i = 0; i < vStr.size(); i++)
    {
        if (strcmp(vStr[i].c_str(), strResult[i]) != 0)
            FAIL() << stringPrintf("strSplit test, case: %d, %s, %s", i, vStr[i].c_str(), strResult[i]).c_str();
    }
}

TEST(StringEx, testStrSplit2) {
    string        strLeft, strRight;
    cstr_t        szInput = "name=valuexxx=x";

    strSplit(szInput, '=', strLeft, strRight);
    ASSERT_TRUE(strLeft == "name");
    ASSERT_TRUE(strRight == "valuexxx=x");

    szInput = "name - valuexxx=x";
    strSplit(szInput, " - ", strLeft, strRight);
    ASSERT_TRUE(strLeft == "name");
    ASSERT_TRUE(strRight == "valuexxx=x");
}

TEST(StringEx, testStrJoin) {
    string        str;

    int vInt[] = { 1, 2, 3, 4 };
    str = strJoin(vInt, vInt + CountOf(vInt), "%d", ",");
    ASSERT_TRUE(str == "1,2,3,4");

    VecStrings        vStr;
    vStr.push_back("1");
    vStr.push_back("2");
    vStr.push_back("3");
    str = strJoin(vStr.begin(), vStr.end(), ",");
    ASSERT_TRUE(str == "1,2,3");
}

TEST(StringEx, testStrReplace) {
    cstr_t        szSrc[] = { "abca", "baAcd", "a" };
    cstr_t        szDst[] = { "xbcx", "bxAcd", "x" };

    for (int i = 0; i < CountOf(szSrc); i++)
    {
        string        str = szSrc[i];
        strrep(str, 'a', 'x');
        ASSERT_TRUE(strcmp(szDst[i], str.c_str()) == 0);
    }
}

TEST(StringEx, testStrBeginWith) {
    cstr_t        szSrc[] = { "abca", "baAcd", "aXDegae", "aXDegae" };
    cstr_t        szBeg[] = { "", "b", "aXD", "aXDegae" };
    cstr_t        szIBeg[] = { "", "B", "axd", "aXDegaE" };
    cstr_t        szBegFalse[] = { "ac", "bb", "x", "aXDegaeD" };

    for (int i = 0; i < CountOf(szSrc); i++)
    {
        ASSERT_TRUE(startsWith(szSrc[i], szBeg[i]));
        ASSERT_TRUE(iStartsWith(szSrc[i], szIBeg[i]));
        ASSERT_TRUE(!startsWith(szSrc[i], szBegFalse[i]));
    }
}

TEST(StringEx, testStrEndWith) {
    cstr_t        szSrc[] = { "abca", "baAcd", "aXDegae", "aXDegae" };
    cstr_t        szEnd[] = { "", "d", "egae", "aXDegae" };
    cstr_t        szIEnd[] = { "", "D", "EGaE", "aXDegaE" };
    cstr_t        szEndFalse[] = { "ac", "bb", "x", "aXDegaeD" };

    for (int i = 0; i < CountOf(szSrc); i++)
    {
        ASSERT_TRUE(endsWith(szSrc[i], szEnd[i]));
        ASSERT_TRUE(iEndsWith(szSrc[i], szIEnd[i]));
        ASSERT_TRUE(!endsWith(szSrc[i], szEndFalse[i]));
    }
}

#endif // UNIT_TEST
