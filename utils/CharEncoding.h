//
//  CharEncoding.hpp
//  Mp3Player
//
//  Created by HongyongXiao on 2021/11/21.
//

#ifndef CharEncoding_hpp
#define CharEncoding_hpp

#include "UtilsTypes.h"


// FE = FileEncoding
#define SZ_FE_UCS2              "\xFF\xFE"
#define SZ_FE_UCS2_BE           "\xFE\xFF"
#define SZ_FE_UTF8              "\xEF\xBB\xBF"

#define SZ_UTF8                 "utf-8"
#define SZ_UNICODE              "unicode"


enum CharEncodingType {
    ED_SYSDEF,                      // System default encoding
    ED_UNICODE,                     // unicode,
    ED_UNICODE_BIG_ENDIAN,          // unicode big-endian,
    ED_UTF8,                        // utf-8
    ED_ARABIC,                      // windows-1256
    // ED_BALTIC_ISO,               // iso-8859-4
    ED_BALTIC_WINDOWS,              // windows-1257
    // ED_CENTRAL_EUROPEAN_ISO,     // iso-8859-2
    ED_CENTRAL_EUROPEAN_WINDOWS,    // windows-1250,
    ED_GB2312,                      // gb2312
    ED_BIG5,                        // big5
    // ED_CYRILLIC,                 // iso-8859-5
    ED_CYRILLIC_WINDOWS,            // windows-1251
    // ED_GREEK_ISO,                // iso-8859-7
    ED_GREEK_WINDOWS,               // windows-1253
    ED_HEBREW_WINDOWS,              // windows-1255
    ED_JAPANESE_SHIFT_JIS,          //shift_jis
    ED_KOREAN,                      // ks_c_5601-1987
    ED_LATIN9_ISO,                  // iso-8859-15,
    ED_THAI,                        // windows-874,
    // ED_TURKISH_ISO,              // iso-8859-9
    ED_TURKISH_WINDOWS,             // windows-1254,
    ED_VIETNAMESE,                  // windows-1258
    // ED_WESTERN_EUROPEAN_ISO,     // iso-8859-1
    ED_WESTERN_EUROPEAN_WINDOWS,    // Windows-1252,
    ED_EASTERN_EUROPEAN_WINDOWS,    // Windows-1250,
    ED_RUSSIAN_WINDOWS,             // Windows-1251,
    ED_END = ED_RUSSIAN_WINDOWS,
};

struct EncodingCodePage {
    CharEncodingType        encodingID;

#ifdef _MAC_OS
    int                     cfStringEncoding;
#endif

#ifdef _WIN32
    int                     codePage;        // windows codepage
    int                     fontCharset;
#endif

    cstr_t                  szEncoding;
    cstr_t                  szAlias;
    cstr_t                  szDesc;

#ifdef _LINUX
    cstr_t                  szIConvCode;
#endif

#ifdef _ANDROID
    cstr_t                  szIConvCode;
#endif

};

extern EncodingCodePage g_encodingCodePages[];

int getCharEncodingCount();

CharEncodingType getCharEncodingID(const char *szEncoding);

EncodingCodePage &getSysDefaultCharEncoding();
EncodingCodePage &getCharEncodingByID(CharEncodingType encoding);

CharEncodingType detectFileEncoding(const void *lpData, int length, int &bomSize);
cstr_t getFileEncodingBom(CharEncodingType fe);
string insertWithFileBom(const string &text);

string strToUtf8ByBom(const char *data, size_t size);

// Is the string only including ANSI characters.
bool isAnsiStr(const WCHAR *szStr);
bool isAnsiStr(const char *szStr);

int mbcsToUtf8(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);
int ucs2ToUtf8(const WCHAR *str, int nLen, string &strOut);
int utf8ToUCS2(const char *str, int nLen, u16string &strOut);
int utf8ToMbcs(const char *str, int nLen, string &strOut, int encodingID = ED_SYSDEF);

uint32_t utf8ToUtf16Length(const uint8_t *str, uint32_t len);
inline uint32_t utf8ToUtf16Length(const SizedString &str) { return utf8ToUtf16Length(str.data, str.len); }
uint32_t utf8ToUtf16(const uint8_t *str, uint32_t len, utf16_t *u16BufOut, uint32_t sizeU16Buf);

uint32_t utf8ToUtf32(const uint8_t *data, uint32_t len, utf32_t *bufOut, uint32_t capacityBufOut);

// 将 utf-32 的 code 转换为 utf-8 编码, @bufOut 需要至少 4 个字节的长度
// 返回填入 bufOut 的字节长度
uint32_t utf32CodeToUtf8Length(uint16_t code);
uint32_t utf32CodeToUtf8(uint16_t code, uint8_t *bufOut);
void utf32CodeToUtf8(uint16_t code, string &out);

// Big Endian to Little Endian, or vice versa
void ucs2EncodingReverse(WCHAR *str, uint32_t nLen);

#endif /* CharEncoding_hpp */
