#pragma once

#include "UtilsTypes.h"


class XCharSeparatedValues {
public:
    XCharSeparatedValues(char chSeparator);
    virtual ~XCharSeparatedValues();

    void split(cstr_t szStr, VecStrings &vValues);
    void split(cstr_t szStr, MapStrings &mapNameValues, char chAssign = '=');

    void addValue(cstr_t szValue);
    void addValue(int value);

    void addNameValue(cstr_t szName, cstr_t szValue, char chAssign = '=');

    void clear() { m_strWriting.clear(); }

    cstr_t c_str() const { return m_strWriting.c_str(); }
    size_t size() const { return m_strWriting.size(); }

protected:
    char                        m_chSeparator;
    string                      m_strWriting;

};

class CCommaSeparatedValues : public XCharSeparatedValues {
public:
    CCommaSeparatedValues() : XCharSeparatedValues(',') { }

};

class CColonSeparatedValues : public XCharSeparatedValues {
public:
    CColonSeparatedValues() : XCharSeparatedValues(':') { }

};
