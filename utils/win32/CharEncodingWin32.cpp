

#include "../CharEncoding.h"
#include "utils/StringEx.h"


#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET      129
#endif

//
// ED_XXX 的定义和 g_encodingCodePages 的索引顺序是一直的。
// 即g_encodingCodePages[ED_XXX].encodingID = ED_XXX
EncodingCodePage    g_encodingCodePages[] = {
    { ED_SYSDEF, CP_ACP, DEFAULT_CHARSET, "", "", "Default" },
    { ED_UNICODE, 0, DEFAULT_CHARSET, "unicode", "", "Unicode" },
    { ED_UNICODE_BIG_ENDIAN, 0, DEFAULT_CHARSET, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)" },
    { ED_UTF8, CP_UTF8, DEFAULT_CHARSET, "utf-8", "", "Unicode (UTF-8)" },
    { ED_ARABIC, 1256, ARABIC_CHARSET, "windows-1256", "arabic", "Arabic" },
    //    { ED_BALTIC_ISO, 28594, DEFAULT_CHARSET, "iso-8859-4", "", "Baltic (ISO)" },
    { ED_BALTIC_WINDOWS, 1257, BALTIC_CHARSET, "windows-1257", "baltic", "Baltic (Windows)" },
    //    { ED_CENTRAL_EUROPEAN_ISO, 28592, DEFAULT_CHARSET, "iso-8859-2", "", "Central European (ISO)" },
    { ED_CENTRAL_EUROPEAN_WINDOWS, 1250, DEFAULT_CHARSET, "windows-1250", "", "Central European (Windows)" },
    { ED_GB2312, 54936, GB2312_CHARSET, "gb2312", "gb2312", "Chinese Simplified (GB2312)" },
    { ED_BIG5, 950, CHINESEBIG5_CHARSET, "big5", "big5", "Chinese Traditional (Big5)" },
    //    { ED_CYRILLIC, 28595, DEFAULT_CHARSET, "iso-8859-5", "", "Cyrillic (ISO)" },
    { ED_CYRILLIC_WINDOWS, 1251, DEFAULT_CHARSET, "windows-1251", "", "Cyrillic (Windows)" },
    //    { ED_GREEK_ISO, 28597, DEFAULT_CHARSET, "iso-8859-7", "", "Greek (ISO)" },
    { ED_GREEK_WINDOWS, 1253, GREEK_CHARSET, "windows-1253", "greek", "Greek (Windows)" },
    { ED_HEBREW_WINDOWS, 1255, HEBREW_CHARSET, "windows-1255", "hebrew", "Hebrew (Windows)" },
    { ED_JAPANESE_SHIFT_JIS, 932, SHIFTJIS_CHARSET, "shift_jis", "shiftjis", "Japanese (Shift-JIS)" },
    { ED_KOREAN, 949, HANGUL_CHARSET, "ks_c_5601-1987", "hangeul", "Korean" },
    { ED_LATIN9_ISO, 1252, DEFAULT_CHARSET, "iso-8859-15", "", "Latin 9 (ISO)" },
    { ED_THAI, 874, THAI_CHARSET, "windows-874", "Thai", "Thai (Windows)" },
    //    { ED_TURKISH_ISO, 28599, DEFAULT_CHARSET, "iso-8859-9", 0, "Turkish (ISO)" },
    { ED_TURKISH_WINDOWS, 1254, TURKISH_CHARSET, "windows-1254", "turkish", "Turkish (Windows)" },
    { ED_VIETNAMESE, 1258, VIETNAMESE_CHARSET, "windows-1258", "vietnamese", "Vietnamese (Windows)" },
    //    { ED_WESTERN_EUROPEAN_ISO, 28591, DEFAULT_CHARSET, "iso-8859-1", "", "Western European (ISO)" },
    { ED_WESTERN_EUROPEAN_WINDOWS, 1252, DEFAULT_CHARSET, "Windows-1252", "", "Western European (Windows)" },
    { ED_EASTERN_EUROPEAN_WINDOWS, 1250, EASTEUROPE_CHARSET, "Windows-1250", "easteurope", "Eastern European (Windows)" },
    { ED_RUSSIAN_WINDOWS, 1251, RUSSIAN_CHARSET, "Windows-1251", "russian", "Russian (Windows)" },
};

#define         __MaxEncodings        CountOf(g_encodingCodePages)

int getCharEncodingCount() { return __MaxEncodings; }

EncodingCodePage &getSysDefaultCharEncoding() {
    uint32_t codePage = GetACP();
    for (int i = 0; i < __MaxEncodings; i++) {
        if (codePage == g_encodingCodePages[i].codePage) {
            return g_encodingCodePages[i];
        }
    }

    return g_encodingCodePages[0];
}

int utf8ToMbcs(const char* str, int size, string& strOut, int encodingID) {
    if (size == -1) {
        size = (int)strlen(str);
    }

    if (encodingID == ED_UTF8) {
        strOut.assign(str, size);
        return (int)strOut.size();
    }

    // Convert to UCS2
    u16string ucs2;
    utf8ToUCS2(str, size, ucs2);

    // Convert UCS2 to MBCS
    strOut.resize(ucs2.size() * 3);

    int n = WideCharToMultiByte(g_encodingCodePages[encodingID].codePage, 0, 
        (LPCWCH)ucs2.c_str(), ucs2.size(), (char *)strOut.data(), strOut.capacity(), nullptr, nullptr);
    if (n <= 0) {
        // Error occured.
        n = 0;
    }
    strOut.resize(n);
    return n;
}

int mbcsToUtf8(const char* str, int size, string& strOut, int encodingID) {
    if (size == -1) {
        size = (int)strlen(str);
    }

    strOut.clear();

    if (encodingID == ED_UTF8) {
        strOut.assign(str, size);
        return (int)size;
    }

    // Convert to UCS2
    u16string ucs2;

    int n = MultiByteToWideChar(g_encodingCodePages[encodingID].codePage, 0, str, size,
        (LPWSTR)ucs2.data(), ucs2.capacity());
    if (n <= 0) {
        // Error occured.
        n = 0;
    }
    ucs2.resize(n);

    // Convert UCS2 to Utf8
    return ucs2ToUtf8(ucs2.c_str(), ucs2.size(), strOut);
}
