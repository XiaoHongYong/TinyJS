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
    if (isEmptyString(szEncoding)) {
        return ED_SYSDEF;
    }

    assert(ED_END + 1 == getCharEncodingCount());
    for (int i = getCharEncodingCount() - 1; i >= 0; i--) {
        EncodingCodePage &ec = g_encodingCodePages[i];
        if (strcasecmp(szEncoding, ec.szEncoding) == 0) {
            return ec.encodingID;
        }

        if (strcasecmp(szEncoding, ec.szAlias) == 0) {
            return ec.encodingID;
        }
        if (strcasecmp(szEncoding, ec.szDesc) == 0) {
            return ec.encodingID;
        }
#if defined(_LINUX) || defined(_ANDROID)
        if (strcasecmp(szEncoding, ec.szIConvCode) == 0) {
            return ec.encodingID;
        }
#endif

    }

    return ED_SYSDEF;
}

EncodingCodePage &getCharEncodingByID(CharEncodingType encoding) {
    if (encoding >= getCharEncodingCount() || encoding < 0) {
        encoding = ED_SYSDEF;
    }

    assert(g_encodingCodePages[encoding].encodingID == encoding);
    return g_encodingCodePages[encoding];
}

bool isUTF8Encoding(cstr_t str, size_t nLen) {
    unsigned char* p = (unsigned char *)str;
    unsigned char* last = (unsigned char *)str + nLen;

    bool isAnsi = true;
    const int MAX_BOUND = 1024 * 100;
    int ul;
    for (ul = 0; (p < last) && (ul < MAX_BOUND); ) {
        ul++;
        if ((*p) < 0x80) {
            p += 1;
            continue;
        }

        isAnsi = false;
        if ((*p) < 0xc0) {
            return false;
        } else if ((*p) < 0xe0) {
            if (p[1] < 0x80) {
                return false;
            }
            p += 2;
        } else if ((*p) < 0xf0) {
            if (p[1] < 0x80 || p[2] < 0x80) {
                return false;
            }
            p += 3;
        } else if ((*p) < 0xf8) {
            if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80) {
                return false;
            }
            p += 4;
        } else if ((*p) < 0xfc) {
            if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80 || p[4] < 0x80) {
                return false;
            }
            p += 5;
        } else if ((*p) < 0xfe) {
            if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80 || p[4] < 0x80 || p[5] < 0x80) {
                return false;
            }
            p += 6;
        } else {
            return false;
        }
    }

    return !isAnsi;
}

CharEncodingType detectFileEncoding(const void *lpData, size_t length, int &bomSize) {
    uint8_t *szBuffer = (uint8_t *)lpData;
    if (szBuffer[0] == 0xFF && szBuffer[1] == 0xFE) {
        // FF FE
        bomSize = 2;
        return ED_UNICODE;
    } else if (szBuffer[0] == 0xFE && szBuffer[1] == 0xFF) {
        // FE FF
        bomSize = 2;
        return ED_UNICODE_BIG_ENDIAN;
    } else if (szBuffer[0] == 0xEF && szBuffer[1] == 0xBB && szBuffer[2] == 0xBF) {
        // EF BB BF
        bomSize = 3;
        return ED_UTF8;
    }

    bomSize = 0;

    if (isUTF8Encoding((cstr_t)szBuffer, length)) {
        return ED_UTF8;
    }

    return ED_SYSDEF;
}

cstr_t getFileEncodingBom(CharEncodingType encoding) {
    switch (encoding) {
        case ED_UNICODE: return SZ_FE_UCS2;
        case ED_UNICODE_BIG_ENDIAN: return SZ_FE_UCS2_BE;
        case ED_UTF8: return SZ_FE_UTF8;
        default: return "";
    }
}

void autoInsertWithUtf8Bom(string &text) {
    if (!isAnsiStr(text.c_str())) {
        text.insert(0, SZ_FE_UTF8);
    }
}

string strToUtf8ByBom(const char *data, size_t size) {
    int bomSize = 0;
    CharEncodingType encoding = detectFileEncoding(data, (int)size, bomSize);
    data += bomSize;
    size -= bomSize;

    std::string str;
    if (encoding == ED_UNICODE) {
        ucs2ToUtf8((utf16_t *)data, (int)size / 2, str);
    } else if (encoding == ED_UNICODE_BIG_ENDIAN) {
        ucs2EncodingReverse((utf16_t *)data, (int)size / 2);
        ucs2ToUtf8((utf16_t *)data, (int)size / 2, str);
    } else if (encoding == ED_UTF8) {
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

bool isAnsiStr(const utf16_t *szStr) {
    while (*szStr) {
        if (*szStr >= 128) {
            return false;
        }
        szStr++;
    }

    return true;
}

bool isAnsiStr(const char *szStr) {
    while (*szStr) {
        if ((unsigned char)*szStr >= 128) {
            return false;
        }
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

int ucs2ToUtf8(const utf16_t *str, int nLen, string &strOut) {
    ensureInputStrLen(str, nLen);

    const utf16_t *wchBegin = str, *wEnd = str + nLen;
    int nOutPos = 0;

    strOut.resize(nLen * 5);

    while (wchBegin < wEnd) {
        if (*wchBegin < 0x80) {
            strOut[nOutPos] = (uint8_t)*wchBegin;
            nOutPos++;
        } else if (*wchBegin < 0x0800) {
            strOut[nOutPos] = (uint8_t)((*wchBegin >> 6) | 0xC0);
            nOutPos++;
            strOut[nOutPos] = (uint8_t)((*wchBegin & 0x3F) | 0x80);
            nOutPos++;
        } else {
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

#define MXS(c,x,s)          (((c) - (x)) <<  (s))
#define M80S(c,s)           MXS(c,0x80,s)
#define UTF8_1_to_UCS2(in)  ((utf16_t) (in)[0])
#define UTF8_2_to_UCS2(in)  ((utf16_t) (MXS((in)[0],0xC0, 6) | M80S((in)[1], 0)))
#define UTF8_3_to_UCS2(in)  ((utf16_t) (MXS((in)[0],0xE0,12) | M80S((in)[1], 6) | M80S((in)[2], 0) ))
#define UTF8_4_to_UCS4(in)  ((utf32_t) (MXS((in)[0],0xF0,18) | M80S((in)[1],12) | M80S((in)[2], 6) | M80S((in)[3], 0) ))

int utf8ToUCS2(const char *str, int nLen, u16string &strOut) {
    ensureInputStrLen(str, nLen);

    strOut.resize(nLen);

    auto size = utf8ToUtf16((uint8_t *)str, nLen, (utf16_t *)strOut.data(), (uint32_t)strOut.size());
    strOut.resize(size);

    return size;
}

void ucs2EncodingReverse(utf16_t *str, uint32_t nLen) {
    assert(nLen >= 0);

    uint8_t *p = (uint8_t *)str;
    int lenBytes = nLen * sizeof(utf16_t);
    for (int i = 0; i < lenBytes; i += 2) {
        uint8_t t = p[i];
        p[i] = p[i + 1];
        p[i + 1] = t;
    }
}

uint32_t utf8ToUtf16Length(const uint8_t *str, uint32_t len) {
    auto p = str, last = str + len;
    uint32_t lenUtf16 = 0;

    // bool hasInvalidChars = false;
    while ((p < last)) {
        lenUtf16++;
        if ((*p) < 0x80) {
            p += 1;
            continue;
        }

        if ((*p) < 0xc0) {
            // hasInvalidChars = true;
            p++;
        } else if ((*p) < 0xe0) {
            // if (p[1] < 0x80) {
            //     hasInvalidChars = true;
            // }
            p += 2;
        } else if ((*p) < 0xf0) {
            // if (p[1] < 0x80 || p[2] < 0x80) {
            //     hasInvalidChars = true;
            // }
            p += 3;
        } else if ((*p) < 0xf8) {
            // Code points from U+010000 to U+10FFFF
            // 由两个 utf-16 char 组成: are encoded as two 16-bit code units called a surrogate pair
            // https://en.wikipedia.org/wiki/UTF-16
            lenUtf16++;
            // if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80) {
            //     hasInvalidChars = true;
            // }
            p += 4;
        } else if ((*p) < 0xfc) {
            // 不支持转 utf-16，转换为 '?'
            // if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80 || p[4] < 0x80) {
            //     hasInvalidChars = true;
            // }
            p += 5;
        } else if ((*p) < 0xfe) {
            // 不支持转 utf-16，转换为 '?'
            // if (p[1] < 0x80 || p[2] < 0x80 || p[3] < 0x80 || p[4] < 0x80 || p[5] < 0x80) {
            //     hasInvalidChars = true;
            // }
            p += 6;
        } else {
            // hasInvalidChars = true;
        }
    }

    return lenUtf16;
}

uint32_t utf8ToUtf16(const uint8_t *str, uint32_t len, utf16_t *u16BufOut, uint32_t sizeU16Buf) {
    auto p = str, last = str + len;
    uint32_t lenUtf16 = 0;

    while (p < last && lenUtf16 < sizeU16Buf) {
        if ((*p) < 0x80) {
            u16BufOut[lenUtf16++] = UTF8_1_to_UCS2(p);
            p += 1;
        } else if ((*p) < 0xc0) {
            assert((*p)); // Invalid UTF8 First Byte
            u16BufOut[lenUtf16++] = '?';
            p += 1;
        } else if ((*p) < 0xe0) {
            u16BufOut[lenUtf16++] = UTF8_2_to_UCS2(p);
            p += 2;
        } else if ((*p) < 0xf0) {
            u16BufOut[lenUtf16++] = UTF8_3_to_UCS2(p);
            p += 3;
        } else if ((*p) < 0xf8) {
            auto n = UTF8_4_to_UCS4(p);
            n -= 0x10000;
            u16BufOut[lenUtf16++] = 0xD800 + (n >> 10);
            if (lenUtf16 < sizeU16Buf) {
                u16BufOut[lenUtf16++] = 0xDC00 + (n & 0x3FF);
            }
            p += 4;
        } else if ((*p) < 0xfc) {
            u16BufOut[lenUtf16++] = '?';
            p += 5;
        } else if ((*p) < 0xfe) {
            u16BufOut[lenUtf16++] = '?';
            p += 6;
        } else {
            u16BufOut[lenUtf16++] = '?';
            p += 1;
        }
    }

    return lenUtf16;
}

uint32_t utf8FirstByteLength(uint8_t c) {
    if ((c) < 0x80) {
        return 1;
    } else if ((c) < 0xc0) {
        return 1;
    } else if ((c) < 0xe0) {
        return 2;
    } else if ((c) < 0xf0) {
        return 3;
    } else if ((c) < 0xf8) {
        return 4;
    } else if ((c) < 0xfc) {
        return 5;
    } else if ((c) < 0xfe) {
        return 6;
    } else {
        return 1;
    }
}

uint8_t *utf8ToUtf16Seek(const uint8_t *str, uint32_t len, uint32_t utf16Pos) {
    auto p = str, last = str + len;
    uint32_t lenUtf16 = 0;

    while (p < last && lenUtf16 < utf16Pos) {
        lenUtf16++;
        if ((*p) < 0x80) {
            p += 1;
        } else if ((*p) < 0xc0) {
            p += 1;
        } else if ((*p) < 0xe0) {
            p += 2;
        } else if ((*p) < 0xf0) {
            p += 3;
        } else if ((*p) < 0xf8) {
            lenUtf16++;
            p += 4;
        } else if ((*p) < 0xfc) {
            p += 5;
        } else if ((*p) < 0xfe) {
            p += 6;
        } else {
            p += 1;
        }
    }

    return (uint8_t *)(p > last ? last : p);
}

uint32_t utf8ToUtf32(const uint8_t *data, uint32_t len, utf32_t *bufOut, uint32_t capacityBufOut) {
    auto p = data, last = data + len;
    uint32_t lenUtf32 = 0;

    while (p < last && lenUtf32 < capacityBufOut) {
        if ((*p) < 0x80) {
            bufOut[lenUtf32] = UTF8_1_to_UCS2(p);
            p += 1;
        } else if ((*p) < 0xc0) {
            assert((*p)); // Invalid UTF8 First Byte
            bufOut[lenUtf32] = '?';
            p += 1;
        } else if ((*p) < 0xe0) {
            bufOut[lenUtf32] = UTF8_2_to_UCS2(p);
            p += 2;
        } else if ((*p) < 0xf0) {
            bufOut[lenUtf32] = UTF8_3_to_UCS2(p);
            p += 3;
        } else if ((*p) < 0xf8) {
            bufOut[lenUtf32] = UTF8_4_to_UCS4(p);
            p += 4;
        } else if ((*p) < 0xfc) {
            bufOut[lenUtf32] = '?';
            p += 5;
        } else if ((*p) < 0xfe) {
            bufOut[lenUtf32] = '?';
            p += 6;
        } else {
            bufOut[lenUtf32] = '?';
            p += 1;
        }
        lenUtf32++;
    }

    return lenUtf32;
}

uint32_t utf32CodeToUtf8Length(uint32_t code) {
    if (code < 0x80) {
        return 1;
    } else if (code < 0x0800) {
        return 2;
    } else if (code < 0xFFFF) {
        return 3;
    } else {
        return 4;
    }
}

uint32_t utf32CodeToUtf16Length(uint32_t code) {
    return code <= 0xFFFF ? 1 : 2;
}


uint32_t utf32CodeToUtf8(uint32_t code, uint8_t *bufOut) {
    if (code < 0x80) {
        bufOut[0] = (uint8_t)code;
        return 1;
    } else if (code < 0x0800) {
        bufOut[0] = (uint8_t)((code >> 6) | 0xC0);
        bufOut[1] = (uint8_t)((code & 0x3F) | 0x80);
        return 2;
    } else if (code < 0xFFFF) {
        bufOut[0] = (uint8_t)((code >> 12) | 0xE0);
        bufOut[1] = (uint8_t)((code >> 6 & 0x3F) | 0x80);
        bufOut[2] = (uint8_t)((code & 0x3F) | 0x80);
        return 3;
    } else {
        bufOut[0] = (uint8_t)((code >> 18) | 0xF0);
        bufOut[1] = (uint8_t)((code >> 12 & 0x3F) | 0x80);
        bufOut[2] = (uint8_t)((code >> 6 & 0x3F) | 0x80);
        bufOut[3] = (uint8_t)((code & 0x3F) | 0x80);
        return 4;
    }
}

void utf32CodeToUtf8(uint32_t code, string &out) {
    if (code < 0x80) {
        out.push_back((uint8_t)code);
    } else if (code < 0x0800) {
        out.push_back((uint8_t)((code >> 6) | 0xC0));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    } else if (code < 0xFFFF) {
        out.push_back((uint8_t)((code >> 12) | 0xE0));
        out.push_back((uint8_t)((code >> 6 & 0x3F) | 0x80));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    } else {
        out.push_back((uint8_t)((code >> 18) | 0xF0));
        out.push_back((uint8_t)((code >> 12 & 0x3F) | 0x80));
        out.push_back((uint8_t)((code >> 6 & 0x3F) | 0x80));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    }
}
