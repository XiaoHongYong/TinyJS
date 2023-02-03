//
//  CharEncodingMac.mm
//
//  Created by HongyongXiao on 2021/11/21.
//

#include <Foundation/Foundation.h>
#include <iconv.h>
#include <CoreFoundation/CoreFoundation.h>
#include "../CharEncoding.h"


//
// ED_XXX 的定义和 g_encodingCodePages 的索引顺序是一直的。
// 即g_encodingCodePages[ED_XXX].nEncodingId = ED_XXX
EncodingCodePage    g_encodingCodePages[] = {
    { ED_SYSDEF, kCFStringEncodingWindowsLatin1, "", "", "systemdefault" },
    { ED_UNICODE, kCFStringEncodingUTF16LE, "unicode", "", "Unicode" },
    { ED_UNICODE_BIG_ENDIAN, kCFStringEncodingUTF16BE, "Unicode (Big-Endian)", "", "Unicode (Big-Endian)" },
    { ED_UTF8, kCFStringEncodingUTF8, "utf-8", "", "Unicode (UTF-8)" },
    { ED_ARABIC, kCFStringEncodingWindowsArabic, "windows-1256", "arabic", "Arabic" },
    { ED_BALTIC_WINDOWS, kCFStringEncodingWindowsBalticRim, "windows-1257", "baltic", "Baltic (Windows)" },
    { ED_CENTRAL_EUROPEAN_WINDOWS, kCFStringEncodingWindowsLatin1, "windows-1250", "", "Central European (Windows)" },
    { ED_GB2312, kCFStringEncodingGB_18030_2000, "gb2312", "gb2312", "Chinese Simplified (GB2312)" },
    { ED_BIG5, kCFStringEncodingDOSChineseTrad, "big5", "big5", "Chinese Traditional (Big5)" },
    { ED_CYRILLIC_WINDOWS, kCFStringEncodingWindowsCyrillic, "windows-1251", "", "Cyrillic (Windows)" },
    { ED_GREEK_WINDOWS, kCFStringEncodingWindowsGreek, "windows-1253", "greek", "Greek (Windows)" },
    { ED_HEBREW_WINDOWS, kCFStringEncodingWindowsHebrew, "windows-1255", "hebrew", "Hebrew (Windows)" },
    { ED_JAPANESE_SHIFT_JIS, kCFStringEncodingDOSJapanese, "shift_jis", "shiftjis", "Japanese (Shift-JIS)" },
    { ED_KOREAN, kCFStringEncodingDOSKorean, "ks_c_5601-1987", "hangeul", "Korean" },
    { ED_LATIN9_ISO, kCFStringEncodingWindowsLatin1, "iso-8859-15", "", "Latin 9 (ISO)" },
    { ED_THAI, kCFStringEncodingDOSThai, "windows-874", "Thai", "Thai (Windows)" },
    { ED_TURKISH_WINDOWS, kCFStringEncodingWindowsLatin5, "windows-1254", "turkish", "Turkish (Windows)" },
    { ED_VIETNAMESE, kCFStringEncodingWindowsVietnamese, "windows-1258", "vietnamese", "Vietnamese (Windows)" },
    { ED_WESTERN_EUROPEAN_WINDOWS, kCFStringEncodingWindowsLatin2, "Windows-1252", "", "Western European (Windows)" },
    { ED_EASTERN_EUROPEAN_WINDOWS, kCFStringEncodingWindowsLatin1, "Windows-1250", "easteurope", "Eastern European (Windows)" },
    { ED_RUSSIAN_WINDOWS, kCFStringEncodingWindowsCyrillic, "Windows-1251", "russian", "Russian (Windows)" },
};

int getCharEncodingCount() { return CountOf(g_encodingCodePages); }

bool initSetDefaultCharEncoding() {
    EncodingCodePage &ecp = getCharEncodingByID(ED_SYSDEF);
    NSLocale *local = [NSLocale currentLocale];
    NSString *lang = [local objectForKey:NSLocaleLanguageCode];
    NSString *countryCode = [local objectForKey: NSLocaleCountryCode];

    // Compare country code
    if ([countryCode isEqual: @"CN"] || [countryCode isEqual: @"SG"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_GB2312).cfStringEncoding;
    } else if ([countryCode isEqual: @"TW"] || [countryCode isEqual: @"HK"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_BIG5).cfStringEncoding;
    } else if ([countryCode isEqual: @"JP"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_JAPANESE_SHIFT_JIS).cfStringEncoding;
    } else if ([lang isEqual: @"ara"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_ARABIC).cfStringEncoding;
    } else if ([lang isEqual: @"bat"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_BALTIC_WINDOWS).cfStringEncoding;
    } else if ([lang isEqual: @"gre"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_GREEK_WINDOWS).cfStringEncoding;
    } else if ([lang isEqual: @"heb"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_HEBREW_WINDOWS).cfStringEncoding;
    } else if ([lang isEqual: @"tha"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_THAI).cfStringEncoding;
    } else if ([lang isEqual: @"tur"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_TURKISH_WINDOWS).cfStringEncoding;
    } else if ([lang isEqual: @"vie"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_VIETNAMESE).cfStringEncoding;
    } else if ([lang isEqual: @"rus"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_RUSSIAN_WINDOWS).cfStringEncoding;
    } else if ([lang isEqual: @"kor"]) {
        ecp.cfStringEncoding = getCharEncodingByID(ED_KOREAN).cfStringEncoding;
    }
    return true;
}

EncodingCodePage &GetSysDefaultCharEncoding() {
    return g_encodingCodePages[ED_SYSDEF];
}

NSString *initNSString(const char *str, int size, int encodingID) {
    NSString *temp;
    string strIn;
    if (str[size] != '\0') {
        strIn.append(str, size);
        str = strIn.c_str();
    }

    temp = [[NSString alloc] initWithBytes:str length:size encoding:CFStringConvertEncodingToNSStringEncoding(
        getCharEncodingByID((CharEncodingType)encodingID).cfStringEncoding)];
    if (!temp) {
        temp = [NSString stringWithCString:str encoding:(NSStringEncoding)NSISOLatin1StringEncoding];
    }

    return temp;
}

int mbcsToUtf8(const char *str, int size, string &strOut, int encodingID) {
    if (size == -1) {
        size = (int)strlen(str);
    }

    strOut.clear();

    if (encodingID == ED_UTF8) {
        if (size == -1) {
            strOut.assign(str);
        } else {
            strOut.assign(str, size);
        }
        return (int)strOut.size();
    }

    NSString *temp = initNSString(str, size, encodingID);
    if (temp) {
        strOut.assign([temp UTF8String]);
    } else {
        return 0;
    }

    return (int)strOut.size();
}

int utf8ToMbcs(const char *str, int size, string &strOut, int encodingID) {
    if (size == -1) {
        size = (int)strlen(str);
    }

    if (encodingID == ED_UTF8) {
        strOut.assign(str, size);
        return (int)strOut.size();
    }

    NSString *temp = [NSString stringWithUTF8String:str];
    if (temp) {
        strOut.resize(size + 5);
        NSUInteger length = 0;
        [temp getBytes:(char *)strOut.c_str() maxLength:strOut.size() usedLength:&length encoding:CFStringConvertEncodingToNSStringEncoding(getCharEncodingByID((CharEncodingType)encodingID).cfStringEncoding) options:0 range:NSMakeRange(0, [temp length]) remainingRange:nullptr];
        strOut.resize(length);
    } else {
        strOut.assign(str, size);
    }

    return (int)strOut.size();
}

static bool _isCharEncodingInitialized_ = initSetDefaultCharEncoding();
