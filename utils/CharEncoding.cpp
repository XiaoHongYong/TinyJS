//
//  CharEncoding.cpp
//
//  Created by HongyongXiao on 2021/11/21.
//

#include "CharEncoding.h"
#include "FileApi.h"
#include "StringEx.h"


EncodingCodePage &getSysDefaultCharEncoding() {
    return g_encodingCodePages[ED_SYSDEF];
}

CharEncodingType getCharEncodingID(const char *szEncoding) {
    if (isEmptyString(szEncoding))
        return ED_SYSDEF;

    assert(ED_END + 1 == getCharEncodingCount());
    for (int i = getCharEncodingCount() - 1; i >= 0; i--) {
        EncodingCodePage &ec = g_encodingCodePages[i];
        if (strcasecmp(szEncoding, ec.szEncoding) == 0)
            return ec.encodingID;

        if (strcasecmp(szEncoding, ec.szAlias) == 0)
            return ec.encodingID;
        if (strcasecmp(szEncoding, ec.szDesc) == 0)
            return ec.encodingID;
#if defined(_LINUX) || defined(_ANDROID)
        if (strcasecmp(szEncoding, ec.szIConvCode) == 0)
            return ec.encodingID;
#endif

    }

    return ED_SYSDEF;
}

EncodingCodePage &getCharEncodingByID(CharEncodingType encoding)
{
    if (encoding >= getCharEncodingCount() || encoding < 0)
        encoding = ED_SYSDEF;

    assert(g_encodingCodePages[encoding].encodingID == encoding);
    return g_encodingCodePages[encoding];
}

bool isUTF8Encoding(cstr_t str, int nLen) {
    unsigned char* p = (unsigned char *)str;
    unsigned char* last = (unsigned char *)str + nLen;

    bool isAnsi = true;
    const int MAX_BOUND = 1024 * 100;
    int        ul;
    for (ul = 0; (p < last) && (ul < MAX_BOUND); )
    {
        ul++;
        if ((*p) < 0x80)
        {
            p += 1;
            continue;
        }

        isAnsi = false;
        if ((*p) < 0xc0)
        {
            return false;
        }
        else if ((*p) < 0xe0)
        {
            if (p[1] < 0x80)
                return false;
            p += 2;
        }
        else if ((*p) < 0xf0)
        {
            if (p[1] < 0x80 || p[2] < 0x80)
                return false;
            p += 3;
        }
        else if ((*p) < 0xf8)
        {
            if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80)
                return false;
            p += 4;
        }
        else if ((*p) < 0xfc)
        {
            if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80 || p[4] < 0x80)
                return false;
            p += 5;
        }
        else if ((*p) < 0xfe)
        {
            if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80 || p[4] < 0x80 || p[5] < 0x80)
                return false;
            p += 6;
        }
        else
        {
            return false;
        }
    }

    return !isAnsi;
}

CharEncodingType detectFileEncoding(const void *lpData, int length, int &bomSize)
{
    uint8_t *szBuffer = (uint8_t *)lpData;
    if (szBuffer[0] == 0xFF && szBuffer[1] == 0xFE)
    {
        // FF FE
        bomSize = 2;
        return ED_UNICODE;
    }
    else if (szBuffer[0] == 0xFE && szBuffer[1] == 0xFF)
    {
        // FE FF
        bomSize = 2;
        return ED_UNICODE_BIG_ENDIAN;
    }
    else if (szBuffer[0] == 0xEF && szBuffer[1] == 0xBB && szBuffer[2] == 0xBF)
    {
        // EF BB BF
        bomSize = 3;
        return ED_UTF8;
    }

    bomSize = 0;

    if (isUTF8Encoding((cstr_t)szBuffer, length))
        return ED_UTF8;

    return ED_SYSDEF;
}

cstr_t getFileEncodingBom(CharEncodingType encoding)
{
    switch (encoding)
    {
    case ED_UNICODE: return SZ_FE_UCS2;
    case ED_UNICODE_BIG_ENDIAN: return SZ_FE_UCS2_BE;
    case ED_UTF8: return SZ_FE_UTF8;
    default: return "";
    }
}

string insertWithFileBom(const string &text) {
    if (!isAnsiStr(text.c_str())) {
        string out;
        out.append(SZ_FE_UTF8);
        out.append(text);
        return out;
    } else {
        return text;
    }
}

string strToUtf8ByBom(const char *data, size_t size) {
    int bomSize = 0;
    CharEncodingType encoding = detectFileEncoding(data, (int)size, bomSize);
    data += bomSize;
    size -= bomSize;

    std::string str;
    if (encoding == ED_UNICODE) {
        ucs2ToUtf8((WCHAR *)data, (int)size / 2, str);
    } else if (encoding == ED_UNICODE_BIG_ENDIAN) {
        ucs2EncodingReverse((WCHAR *)data, (int)size / 2);
        ucs2ToUtf8((WCHAR *)data, (int)size / 2, str);
    } if (encoding == ED_UTF8) {
        str.assign(data, size);
    } else {
        str.assign(data, size);
    }

    return str;
}

bool readFileByBom(const char *fn, std::string &str) {
    string tmp;
    if (!readFile(fn, tmp)) {
        return false;
    }

    str.assign(strToUtf8ByBom(tmp.c_str(), tmp.size()));
    return true;
}

bool isAnsiStr(const WCHAR *szStr)
{
    while (*szStr)
    {
        if (*szStr >= 128)
            return false;
        szStr++;
    }

    return true;
}

bool isAnsiStr(const char *szStr)
{
    while (*szStr)
    {
        if ((unsigned char)*szStr >= 128)
            return false;
        szStr++;
    }

    return true;
}

template<typename CHAR>
inline void ensureInputStrLen(const CHAR *str, int &nLen) {
    if (nLen == -1) {
        auto p = str;
        while (*p) {
            p++;
        }
        nLen = (int)(p - str);
    }
}

int ucs2ToUtf8(const WCHAR *str, int nLen, string &strOut)
{
    ensureInputStrLen(str, nLen);

    const WCHAR    *wchBegin = str, *wEnd = str + nLen;
    int            nOutPos = 0;

    strOut.resize(nLen * 5);

    while (wchBegin < wEnd)
    {
        if (*wchBegin < 0x80)
        {
            strOut[nOutPos] = (uint8_t)*wchBegin;
            nOutPos++;
        }
        else if (*wchBegin < 0x0800)
        {
            strOut[nOutPos] = (uint8_t)((*wchBegin >> 6) | 0xC0);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin & 0x3F) | 0x80);
            nOutPos++;
        }
        else
        {
            strOut[nOutPos] = (uint8_t)((*wchBegin >> 12) | 0xE0);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin >> 6 & 0x3F) | 0x80);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin & 0x3F) | 0x80);
            nOutPos++;
        }
        wchBegin++;
    }
    strOut.resize(nOutPos);

    return nOutPos;
}

#define MXS(c,x,s)            (((c) - (x)) <<  (s))
#define M80S(c,s)            MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)    ((WCHAR) (in)[0])
#define UTF8_2_to_UCS2(in)    ((WCHAR) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)    ((WCHAR) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))

int utf8ToUCS2(const char *str, int nLen, u16string &strOut)
{
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen);

    unsigned char* p = (unsigned char *)str;
    unsigned char* last = (unsigned char *)str + nLen;
    int ul = 0;
    while (p < last)
    {
        if ((*p) < 0x80)
        {
            strOut[ul++] = UTF8_1_to_UCS2(p);
            p += 1;
        }
        else if ((*p) < 0xc0)
        {
            assert((*p));    // Invalid UTF8 First Byte
            strOut[ul++] = '?';
            p += 1;
        }
        else if ((*p) < 0xe0)
        {
            strOut[ul++] = UTF8_2_to_UCS2(p);
            p += 2;
        }
        else if ((*p) < 0xf0)
        {
            strOut[ul++] = UTF8_3_to_UCS2(p);
            p += 3;
        }
        else if ((*p) < 0xf8)
        {
            strOut[ul++] = '?';
            p += 4;
        }
        else if ((*p) < 0xfc)
        {
            strOut[ul++] = '?';
            p += 5;
        }
        else if ((*p) < 0xfe)
        {
            strOut[ul++] = '?';
            p += 6;
        }
        else
        {
            assert((*p));    // Invalid UTF8 First Byte
            strOut[ul++] = '?';
            p += 1;
        }
    }

    strOut.resize(ul);
    return ul;
}

void ucs2EncodingReverse(WCHAR *str, int nLen)
{
    assert(nLen >= 0);

    uint8_t *p = (uint8_t *)str;
    int lenBytes = nLen * sizeof(WCHAR);
    for (int i = 0; i < lenBytes; i += 2) {
        uint8_t t = p[i];
        p[i] = p[i + 1];
        p[i + 1] = t;
    }
}
