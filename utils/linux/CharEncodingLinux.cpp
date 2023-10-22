#include <iconv.h>
#include "../CharEncoding.h"


//
// ED_XXX 的定义和 g_encodingCodePages 的索引顺序是一直的。
// 即g_encodingCodePages[ED_XXX].encodingID = ED_XXX
EncodingCodePage	g_encodingCodePages[] = 
{
	{ ED_SYSDEF, "", "", "systemdefault", "ISO-8859-15" },
	{ ED_UNICODE, "unicode", "", "Unicode", "unicode" },
	{ ED_UNICODE_BIG_ENDIAN, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)", "unicode" },
	{ ED_UTF8, "utf-8", "", "Unicode (UTF-8)", "utf-8" },
	{ ED_ARABIC, "windows-1256", "arabic", "Arabic", "windows-1256" },
	{ ED_BALTIC_WINDOWS, "windows-1257", "baltic", "Baltic (Windows)", "windows-1257" },
	{ ED_CENTRAL_EUROPEAN_WINDOWS, "windows-1250", "", "Central European (Windows)", "windows-1250" },
	{ ED_GB2312, "gb2312", "gb2312", "Chinese Simplified (GB2312)", "gb2312" },
	{ ED_BIG5, "big5", "big5", "Chinese Traditional (Big5)", "big5" },
	{ ED_CYRILLIC_WINDOWS, "windows-1251", "", "Cyrillic (Windows)", "windows-1251" },
	{ ED_GREEK_WINDOWS, "windows-1253", "greek", "Greek (Windows)", "windows-1253" },
	{ ED_HEBREW_WINDOWS, "windows-1255", "hebrew", "Hebrew (Windows)", "windows-1255" },
	{ ED_JAPANESE_SHIFT_JIS, "shift_jis", "shiftjis", "Japanese (Shift-JIS)", "shift_jis" },
	{ ED_KOREAN, "ks_c_5601-1987", "hangeul", "Korean", "CP949" },
	{ ED_LATIN9_ISO, "iso-8859-15", "", "Latin 9 (ISO)", "iso-8859-15" },
	{ ED_THAI, "windows-874", "Thai", "Thai (Windows)", "CP874" },
	{ ED_TURKISH_WINDOWS, "windows-1254", "turkish", "Turkish (Windows)", "windows-1254" },
	{ ED_VIETNAMESE, "windows-1258", "vietnamese", "Vietnamese (Windows)", "windows-1258" },
	{ ED_WESTERN_EUROPEAN_WINDOWS, "Windows-1252", "", "Western European (Windows)", "Windows-1252" },
	{ ED_EASTERN_EUROPEAN_WINDOWS, "Windows-1250", "easteurope", "Eastern European (Windows)", "Windows-1250" },
	{ ED_RUSSIAN_WINDOWS, "Windows-1251", "russian", "Russian (Windows)", "Windows-1251" },
};

int getCharEncodingCount() { return CountOf(g_encodingCodePages); }

EncodingCodePage &GetSysDefaultCharEncoding() {
    return g_encodingCodePages[ED_UTF8];
}

int mbcsToUtf8(const char *str, int size, string &strOut, int encodingID) {
    if (size == -1) {
        size = (int)strlen(str);
    }

    strOut.clear();

    if (encodingID == ED_UTF8) {
        strOut.assign(str, size);
        return size;
    }

	iconv_t code = iconv_open(SZ_UTF8, g_encodingCodePages[encodingID].szIConvCode);
	if (code == (iconv_t)-1) {
		return 0;
    }

    strOut.resize(size * 2);

	size_t inBytesLeft = size, outBytesLeft = strOut.size();
    char *outData = (char *)strOut.data();
	size_t n = iconv(code, (char **)&str, &inBytesLeft, &outData, &outBytesLeft);
	if (n == -1) {
		n = 0;
	}

	strOut.resize(n);
	iconv_close(code);

    return (int)n;
}

int utf8ToMbcs(const char *str, int size, string &strOut, int encodingID) {
    if (size == -1) {
        size = (int)strlen(str);
    }

    if (encodingID == ED_UTF8) {
        strOut.assign(str, size);
        return size;
    }

	iconv_t code = iconv_open(g_encodingCodePages[encodingID].szIConvCode, SZ_UTF8);
	if (code == (iconv_t)-1) {
		return 0;
    }

    strOut.resize(size);

	size_t inBytesLeft = size, outBytesLeft = strOut.size();
    char *outData = (char *)strOut.data();
	size_t n = iconv(code, (char **)&str, &inBytesLeft, &outData, &outBytesLeft);
	if (n == -1) {
		n = 0;
	}

	strOut.resize(n);
	iconv_close(code);

    return (int)n;
}
