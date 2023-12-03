//
//  StringView.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/1.
//

#include "utils/Utils.h"


#if UNIT_TEST

#include "utils/unittest.h"


TEST(StringView, strstr) {
    cstr_t input;

    {
        input = "";
        StringView s(input);
        ASSERT_EQ(s.strstr(""), 0);
        ASSERT_EQ(s.strstr("", -1), 0);
    }

    {
        input = "";
        StringView s(input);
        ASSERT_EQ(s.strstr("abc"), -1);
        ASSERT_EQ(s.strstr("abc", -1), -1);
    }

    {
        input = "abcbc";
        StringView s(input);
        ASSERT_EQ(s.strstr("c"), 2);
        ASSERT_EQ(s.strstr("c", 4), 4);
        ASSERT_EQ(s.strstr("c", 3), 4);
        ASSERT_EQ(s.strstr("c", -1), 2);
        ASSERT_EQ(s.strstr("bc"), 1);
        ASSERT_EQ(s.strstr("bc", 3), 3);
        ASSERT_EQ(s.strstr("bc", 1), 1);
        ASSERT_EQ(s.strstr("bc", 0), 1);
        ASSERT_EQ(s.strstr("abcbc"), 0);
        ASSERT_EQ(s.strstr("abcbc", -1), 0);
        ASSERT_EQ(s.strstr("abcbc", 1), -1);
        ASSERT_EQ(s.strstr("cd"), -1);
        ASSERT_EQ(s.strstr("abc"), 0);
        ASSERT_EQ(s.strstr("abc", 1), -1);
        ASSERT_EQ(s.strstr("a"), 0);
        ASSERT_EQ(s.strstr("a", 2), -1);
    }
}

TEST(StringView, strrstr) {
    cstr_t input;

    {
        input = "";
        StringView s(input);
        ASSERT_EQ(s.strrstr(""), 0);
        ASSERT_EQ(s.strrstr("", -1), 0);
    }

    {
        input = "";
        StringView s(input);
        ASSERT_EQ(s.strrstr("abc"), -1);
        ASSERT_EQ(s.strrstr("abc", -1), -1);
    }

    {
        input = "abcbc";
        StringView s(input);
        ASSERT_EQ(s.strrstr("c"), 4);
        ASSERT_EQ(s.strrstr("c", 4), 4);
        ASSERT_EQ(s.strrstr("c", 3), 2);
        ASSERT_EQ(s.strrstr("bc"), 3);
        ASSERT_EQ(s.strrstr("bc", 3), 3);
        ASSERT_EQ(s.strrstr("bc", 1), 1);
        ASSERT_EQ(s.strrstr("bc", 0), -1);
        ASSERT_EQ(s.strrstr("abcbc"), 0);
        ASSERT_EQ(s.strrstr("abcbc", -1), 0);
        ASSERT_EQ(s.strrstr("abcbc", 1), 0);
        ASSERT_EQ(s.strrstr("cd"), -1);
        ASSERT_EQ(s.strrstr("abc"), 0);
        ASSERT_EQ(s.strrstr("abc", 1), 0);
        ASSERT_EQ(s.strrstr("a"), 0);
        ASSERT_EQ(s.strrstr("a", 2), 0);
    }
}

#endif
