#include "UtilsTypes.h"
#include "XCharSeparatedValues.h"
#include "StringEx.h"


#define CHAR_ESCAPE         '\\'

XCharSeparatedValues::XCharSeparatedValues(char chSeparator) {
    m_chSeparator = chSeparator;
}

XCharSeparatedValues::~XCharSeparatedValues() {
}

void XCharSeparatedValues::split(cstr_t szStr, VecStrings &vValues) {
    vValues.clear();

    const char *p = szStr;

    string strValue;

    // read until m_chSeparator or end of line.
    while (*p) {
        if (*p == CHAR_ESCAPE) {
            p++;
        } else if (*p == m_chSeparator) {
            vValues.push_back(strValue);
            strValue.clear();
            p++;
            continue;
        }

        strValue += *p;
        p++;
    }

    vValues.push_back(strValue);
}

void XCharSeparatedValues::split(cstr_t szStr, MapStrings &mapNameValues, char chAssign) {
    mapNameValues.clear();

    VecStrings vValues;

    split(szStr, vValues);

    for (size_t i = 0; i < vValues.size(); i++) {
        string key, value;
        strSplit(vValues[i].c_str(), chAssign, key, value);
        mapNameValues[key] = value;
    }
}

void XCharSeparatedValues::addValue(cstr_t szValue) {
    if (m_strWriting.size() > 0) {
        m_strWriting += m_chSeparator;
    }

    cstr_t szBegin = szValue;
    cstr_t p = szBegin;
    while (*p) {
        if (*p == m_chSeparator || *p == CHAR_ESCAPE || *p == '\r' || *p == '\n') {
            m_strWriting.append(szBegin, (size_t)(p - szBegin));
            m_strWriting += CHAR_ESCAPE;
            m_strWriting += *p;
            szBegin = p + 1;
        }
        p++;
    }

    m_strWriting.append(szBegin);
}

void XCharSeparatedValues::addValue(int value) {
    if (m_strWriting.size() > 0) {
        m_strWriting += m_chSeparator;
    }

    char szBuff[32];

    itoa(value, szBuff);
    m_strWriting += szBuff;
}

void XCharSeparatedValues::addNameValue(cstr_t szName, cstr_t szValue, char chAssign) {
    string str = szName;
    str += chAssign;
    str += szValue;

    addValue(str.c_str());
}

#if UNIT_TEST

#include "unittest.h"


TEST(SepValues, testCSV) {
    CColonSeparatedValues csv;
    CColonSeparatedValues csv2;

    {
        VecStrings vValues;
        cstr_t szText = "1:\\:2:33:";

        csv.split(szText, vValues);
        trimStr(vValues);

        cstr_t szValue[] = { "1", ":2", "33", "", };
        ASSERT_TRUE(vValues.size() == CountOf(szValue));
        for (int i = 0; i < CountOf(szValue); i++) {
            ASSERT_TRUE(vValues[i] == szValue[i]);
            csv2.addValue(szValue[i]);
        }

        ASSERT_TRUE(strcmp(csv2.c_str(), szText) == 0);
    }

    {
        VecStrings vValues;
        cstr_t szText = "";

        csv2.clear();

        csv.split(szText, vValues);
        trimStr(vValues);

        cstr_t szValue[] = { "" };
        ASSERT_TRUE(vValues.size() == CountOf(szValue));
        for (int i = 0; i < CountOf(szValue); i++) {
            ASSERT_TRUE(vValues[i] == szValue[i]);
            csv2.addValue(szValue[i]);
        }

        ASSERT_TRUE(strcmp(csv2.c_str(), szText) == 0);
    }
}

#endif // UNIT_TEST
