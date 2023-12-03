//
//  StringParser.hpp
//  TinyJS
//
//  Created by henry_xiao on 2023/9/8.
//

#ifndef StringParser_hpp
#define StringParser_hpp

#include "StringView.h"
#include "UtilsTypes.h"


class ParserRangeError : public std::out_of_range {
public:
    ParserRangeError(const string &desc) : std::out_of_range(desc) { }

};

class ParserFormatError : public std::logic_error {
public:
    ParserFormatError(const string &desc) : std::logic_error(desc) { }

};

class StringParser {
public:
    StringParser(const StringView &text);
    StringParser(cstr_t text, size_t len);

    void forward(int n);
    void backward(int n);
    void setPosition(int n);
    int position();

    void expect(char c);
    void expect(const StringView &str);
    void expectAnyCharOf(const StringView &str);
    void expectSpaces();
    // Using case insensitive compare
    void expectI(const StringView &str);

    bool is(char c, bool forward = true);
    bool is(const StringView &str, bool forward = true);
    bool isI(const StringView &str, bool forward = true);

    void ignoreSpaces();

    StringView readTill(char c);
    StringView readTill(const StringView &str);

protected:
    cstr_t                  _text = nullptr;
    int                     _pos = 0, _len = 0;

};

#endif /* StringParser_hpp */
