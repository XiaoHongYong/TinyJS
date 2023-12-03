//
//  StringParser.cpp
//  TinyJS
//
//  Created by henry_xiao on 2023/9/8.
//

#include <cstdarg>
#include "StringParser.hpp"
#include "StringEx.h"


StringParser::StringParser(const StringView &text) : _text((char *)text.data), _len(text.len) {
}

StringParser::StringParser(cstr_t text, size_t len) : _text(text), _len((int)len) {
}

void StringParser::forward(int n) {
    _pos += n;
    if (_pos < 0 || _pos > _len) {
        throw ParserRangeError(stringPrintf("Forward out-of-range, position: %d, forward: %d", _pos, n));
    }
}

void StringParser::backward(int n) {
    _pos -= n;
    if (_pos < 0 || _pos > _len) {
        throw ParserRangeError(stringPrintf("Backward out-of-range, position: %d, backward: %d", _pos, n));
    }
}

void StringParser::setPosition(int n) {
    if (n < 0 || n > _len) {
        throw ParserRangeError(stringPrintf("SetPosition out-of-range, length: %d, position: %d", _len, n));
    }

    _pos = n;
}

int StringParser::position() {
    return _pos;
}

void StringParser::expect(char c) {
    if (_pos >= _len) {
        throw ParserFormatError(stringPrintf("Expected character '%c', but reach to string end.", c).c_str());
    }

    if (_text[_pos] != c) {
        throw ParserFormatError(stringPrintf("Expected character '%c', actual: '%c', at pos: %d.", c, _text[_pos], _pos).c_str());
    }

    _pos++;
}

void StringParser::expect(const StringView &str) {
    if (_pos + str.len > _len) {
        throw ParserFormatError(stringPrintf("Expected string '%.*s', but reach to string end.", str.len, str.data).c_str());
    }

    if (memcmp(str.data, _text + _pos, str.len) != 0) {
        throw ParserFormatError(stringPrintf("Expected string '%.*s' at position: %d.", str.len, str.data, _len).c_str());
    }

    _pos += + str.len;
}

void StringParser::expectAnyCharOf(const StringView &str) {
    if (_pos >= _len) {
        throw ParserFormatError("Expected a character, but reach to string end.");
    }

    if (str.strchr(_text[_pos]) == -1) {
        throw ParserFormatError(stringPrintf("Expected any character in '%.*s', actual: '%c', at pos: %d.", str.len, str.data, _text[_pos], _pos).c_str());
    }

    _pos++;
}

void StringParser::expectSpaces() {
    if (_pos >= _len) {
        throw ParserFormatError(stringPrintf("Expected space, but reach to string end.").c_str());
    }

    if (!isSpace(_text[_pos])) {
        throw ParserFormatError(stringPrintf("Expected spaces at position: %d", _pos).c_str());
    }

    while (_pos < _len && isSpace(_text[_pos])) {
        _pos++;
    }
}

void StringParser::expectI(const StringView &str) {
    if (_pos + str.len > _len) {
        throw ParserFormatError(stringPrintf("Expected stringI '%.*s', but reach to string end.", str.len, str.data).c_str());
    }

    if (strncasecmp((char *)str.data, _text + _pos, str.len) != 0) {
        throw ParserFormatError(stringPrintf("Expected stringI '%.*s' at position: %d.", str.len, str.data, _pos).c_str());
    }

    _pos += + str.len;
}

bool StringParser::is(char c, bool forward) {
    if (_pos < _len) {
        if (_text[_pos] == c) {
            if (forward) {
                _pos++;
            }
            return true;
        }
    }

    return false;
}

bool StringParser::is(const StringView &str, bool forward) {
    if (_pos + str.len < _len) {
        if (strncmp((char *)str.data, _text + _pos, str.len) == 0) {
            if (forward) {
                _pos += + str.len;
            }
            return true;
        }
    }

    return false;
}

bool StringParser::isI(const StringView &str, bool forward) {
    if (_pos + str.len < _len) {
        if (strncasecmp((char *)str.data, _text + _pos, str.len) == 0) {
            if (forward) {
                _pos += + str.len;
            }
            return true;
        }
    }

    return false;
}

void StringParser::ignoreSpaces() {
    while (_pos < _len && isSpace(_text[_pos])) {
        _pos++;
    }
}

StringView StringParser::readTill(char c) {
    for (int i = _pos; i < _len; i++) {
        if (_text[i] == c) {
            StringView s(_text + _pos, i - _pos);
            _pos = i + 1;
            return s;
        }
    }

    throw ParserFormatError(stringPrintf("Not found character: %c, from position: %d", c, _pos));
}

StringView StringParser::readTill(const StringView &str) {
    char c = str.data[0];

    for (int i = _pos; i < _len; i++) {
        if (_text[i] == c && strncmp(_text + i, (char *)str.data, str.len) == 0) {
            StringView s(_text + _pos, i - _pos);
            _pos = i + str.len;
            return s;
        }
    }

    throw ParserFormatError(stringPrintf("Not found string: %.*s, from position: %d", str.len, str.data, _pos));
}


#if UNIT_TEST

#include "../TinyJS/utils/unittest.h"
#include "../TinyJS/utils/FileApi.h"

TEST(StringParser, position) {
    assertException([]() {
        StringParser parser("abcd");
        parser.forward(5);
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.forward(-1);
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.backward(1);
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.backward(-5);
    }, __LINE__);

    StringParser parser("abcd");
    ASSERT_EQ(parser.position(), 0);
    parser.forward(1);
    ASSERT_EQ(parser.position(), 1);
    parser.forward(2);
    ASSERT_EQ(parser.position(), 3);
    parser.forward(1);
    ASSERT_EQ(parser.position(), 4);
    parser.backward(2);
    ASSERT_EQ(parser.position(), 2);
}

TEST(StringParser, expect) {
    // void expect(char c);
    // void expect(const StringView &str);
    // void expectAnyCharOf(const StringView &str);
    // void expectSpaces();
    // void expectI(const StringView &str);

    assertException([]() {
        StringParser parser("abcd");
        parser.expect('x');
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.expect('A');
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.expect("Ab");
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.expectAnyCharOf("cd");
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.expectAnyCharOf("");
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.expectSpaces();
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.expectI("BC");
    }, __LINE__);

    assertException([]() {
        StringParser parser("");
        parser.expect('x');
    }, __LINE__);

    assertException([]() {
        StringParser parser("");
        parser.expect("Ab");
    }, __LINE__);

    assertException([]() {
        StringParser parser("");
        parser.expectAnyCharOf("cd");
    }, __LINE__);

    assertException([]() {
        StringParser parser("a");
        parser.expect("ab");
    }, __LINE__);

    assertException([]() {
        StringParser parser("");
        parser.expectSpaces();
    }, __LINE__);

    assertException([]() {
        StringParser parser("b");
        parser.expectI("BC");
    }, __LINE__);

    StringParser parser("abc  dXy");
    parser.expectI("");
    ASSERT_EQ(parser.position(), 0);

    parser.expect('a');
    ASSERT_EQ(parser.position(), 1);
    parser.expect("bc");
    ASSERT_EQ(parser.position(), 3);
    parser.expectSpaces();
    ASSERT_EQ(parser.position(), 5);
    parser.expectAnyCharOf("Xd");
    ASSERT_EQ(parser.position(), 6);
    parser.expectI("xY");
    ASSERT_EQ(parser.position(), 8);
}

TEST(StringParser, is) {
    // bool is(char c, bool forward = true);
    // bool is(const StringView &str, bool forward = true);
    // bool isI(const StringView &str, bool forward = true);

    {
        StringParser parser("abcd");
        ASSERT_FALSE(parser.is('A'));
        ASSERT_EQ(parser.position(), 0);
    }

    {
        StringParser parser("abcd");
        ASSERT_FALSE(parser.is("Ab"));
        ASSERT_EQ(parser.position(), 0);
    }

    {
        StringParser parser("abcd");
        ASSERT_FALSE(parser.isI("xAb"));
        ASSERT_EQ(parser.position(), 0);
    }

    {
        StringParser parser("");
        ASSERT_FALSE(parser.is('A'));
        ASSERT_EQ(parser.position(), 0);
    }

    {
        StringParser parser("A");
        ASSERT_FALSE(parser.is("Ab"));
        ASSERT_EQ(parser.position(), 0);
    }

    {
        StringParser parser("xA");
        ASSERT_FALSE(parser.isI("xAb"));
        ASSERT_EQ(parser.position(), 0);
    }

    StringParser parser("abc  dXy");
    ASSERT_TRUE(parser.is(""));
    ASSERT_EQ(parser.position(), 0);

    ASSERT_TRUE(parser.is('a'));
    ASSERT_EQ(parser.position(), 1);
    ASSERT_TRUE(parser.is("bc", false));
    ASSERT_EQ(parser.position(), 1);
    ASSERT_TRUE(parser.is("bc"));
    ASSERT_EQ(parser.position(), 3);
    ASSERT_FALSE(parser.isI("C  D"));
    ASSERT_EQ(parser.position(), 3);
    ASSERT_TRUE(parser.isI("  D", false));
    ASSERT_EQ(parser.position(), 3);
    ASSERT_TRUE(parser.isI("  D"));
    ASSERT_EQ(parser.position(), 6);
}

TEST(StringParser, readTill) {
    // StringView readTill(char c);
    // StringView readTill(const StringView &str);

    assertException([]() {
        StringParser parser("abcd");
        parser.readTill('A');
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.readTill("abcde");
    }, __LINE__);

    assertException([]() {
        StringParser parser("abcd");
        parser.readTill("D");
    }, __LINE__);

    assertException([]() {
        StringParser parser("");
        parser.readTill('A');
    }, __LINE__);

    assertException([]() {
        StringParser parser("");
        parser.readTill("ab");
    }, __LINE__);

    StringView s;
    StringParser parser("abc  dXy");
    s = parser.readTill('b');
    ASSERT_EQ(parser.position(), 2);
    ASSERT_TRUE(s.equal("a"));

    s = parser.readTill("Xy");
    ASSERT_EQ(parser.position(), 8);
    ASSERT_TRUE(s.equal("c  d"));
}

#endif
