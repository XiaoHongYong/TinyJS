//
//  Lexer.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#include "parser/Lexer.hpp"


#if UNIT_TEST

#include "utils/unittest.h"


StringView makeParseNumberString(const char *str) {
    static uint8_t buf[256];
    size_t len = strlen(str);
    memcpy(buf, str, len);
    buf[len] = '1';
    buf[len + 1] = '0';
    buf[len + 2] = '2';

    return StringView(buf, len);
}

TEST(JsLexer, parserNumber) {
    StringView str;
    double v;
    uint8_t *p;

    str = makeParseNumberString("9.999999999999999");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 9.999999999999999);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0);
    ASSERT_EQ(p, str.data);

    str = makeParseNumberString("1");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("0");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("2a");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 2);
    ASSERT_EQ(p, str.data + 1);

    str = makeParseNumberString("0x");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0);
    ASSERT_EQ(p, str.data + 1);

    str = makeParseNumberString("0Xf");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0xf);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("0xf1");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0xf1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("0xg");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0);
    ASSERT_EQ(p, str.data + 1);

    str = makeParseNumberString("0b");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0);
    ASSERT_EQ(p, str.data + 1);

    str = makeParseNumberString("0b1");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("0B10");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 2);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("0b13");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 1);
    ASSERT_EQ(p, str.data + str.len - 1);

    str = makeParseNumberString("077");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 077);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("078");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 7);
    ASSERT_EQ(p, str.data + str.len - 1);

    str = makeParseNumberString(".");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0);
    ASSERT_EQ(p, str.data);

    str = makeParseNumberString(".1");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0.1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("1.");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 1);
    ASSERT_EQ(p, str.data + 1);

    str = makeParseNumberString("0.1");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 0.1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("11.22");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 11.22);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("11.22e3");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 11.22e3);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("11.22f");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 11.22);
    ASSERT_EQ(p, str.data + str.len - 1);

    str = makeParseNumberString("11.22e+3");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 11.22e3);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("11.22e-3");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 11.22e-3);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("11.22e");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 11.22);
    ASSERT_EQ(p, str.data + str.len - 1);

    str = makeParseNumberString("1e-");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("1e+");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 1);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("999999999999998200000");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 9999999999999982e5);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("9.999999999999982e+67");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 9.999999999999982e+67);
    ASSERT_EQ(p, str.data + str.len);

    str = makeParseNumberString("5e-324");
    p = parseNumber(str, v);
    ASSERT_EQ(v, 5e-324);
    ASSERT_EQ(p, str.data + str.len);
}

#endif
