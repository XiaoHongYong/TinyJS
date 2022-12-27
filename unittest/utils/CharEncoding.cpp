//
//  CharEncoding.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/1.
//

#include "utils/CharEncoding.h"


#if UNIT_TEST

#include "utils/unittest.h"


TEST(CharEncoding, utf8ToUtf16Length) {
    cstr_t input;

    input = "𝌆"; // \xf0\x9d\x8c\x86
    ASSERT_EQ(strlen(input), 4);
    ASSERT_EQ(utf8ToUtf16Length((uint8_t *)input, (int)strlen(input)), 2);
}

TEST(CharEncoding, utf8ToUtf16) {
    cstr_t input;
    utf16_t buf[1024];

    input = "𝌆"; // \xf0\x9d\x8c\x86
    ASSERT_EQ(strlen(input), 4);
    ASSERT_EQ(utf8ToUtf16((uint8_t *)input, (int)strlen(input), buf, CountOf(buf)), 2);
    ASSERT_EQ(buf[0], 0xD834);
    ASSERT_EQ(buf[1], 0xDF06);

    ASSERT_EQ(utf8ToUtf16((uint8_t *)input, (int)strlen(input), buf, 2), 2);
    ASSERT_EQ(buf[0], 0xD834);
    ASSERT_EQ(buf[1], 0xDF06);

    // buf 不够空间
    ASSERT_EQ(utf8ToUtf16((uint8_t *)input, (int)strlen(input), buf, 1), 1);
    ASSERT_EQ(buf[0], 0xD834);

    ASSERT_EQ(utf8ToUtf16((uint8_t *)input, (int)strlen(input), buf, 0), 0);
    ASSERT_EQ(buf[0], 0xD834);
}

#endif
