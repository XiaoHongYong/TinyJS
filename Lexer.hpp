//
//  Lexer.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#ifndef Lexer_hpp
#define Lexer_hpp

#include "../Utils/Utils.h"
#include "VirtualMachineTypes.hpp"


class ResourcePool;

enum TokenType : uint8_t {
    TK_ERR,
    TK_EOF,

    TK_LET,
    TK_OF,
    TK_FROM,
    TK_AS,
    TK_STATIC,
    TK_YIELD,
    TK_ASYNC,
    TK_IMPLEMENTS,
    TK_PACKAGE,

    // 之前的关键字可以作为变量名
    _TK_END_VAR_KEYWORD,

    TK_AWAIT,
    TK_CLASS,
    TK_BREAK,
    TK_CONTINUE,
    TK_DEBUGGER,
    TK_DEFAULT,
    TK_DELETE,
    TK_TRUE,
    TK_FALSE,
    TK_NULL,
    TK_INSTANCEOF,
    TK_VOID,
    TK_TYPEOF,
    TK_FUNCTION,
    TK_IF,
    TK_TRY,
    TK_CATCH,
    TK_FINALLY,
    TK_VAR,
    TK_CONST,
    TK_WITH,
    TK_FOR,
    TK_DO,
    TK_WHILE,
    TK_SWITCH,
    TK_RETURN,
    TK_THROW,
    TK_ELSE,
    TK_CASE,
    TK_NEW,
    TK_EXPORT,
    TK_EXTENDS,
    TK_IMPORT,
    TK_SUPER,
    TK_IN,

    _TK_END_KEYWORD,
    
    // 非 Keyword 的 token
    TK_STRING,
    TK_NUMBER,
    TK_REGEX,
    TK_NAME,

    TK_ASSIGN,
    TK_ASSIGN_X,
    TK_CONDITIONAL,
    TK_NULLISH,
    TK_LOGICAL_OR,
    TK_LOGICAL_AND,
    TK_BIT_OR,
    TK_BIT_XOR,
    TK_BIT_AND,
    TK_EQUALITY,
    TK_RATIONAL,
    TK_SHIFT,
    TK_ADD,
    TK_MUL,
    TK_EXP,
    TK_UNARY_PREFIX,
    TK_POSTFIX,
    TK_OPEN_BRACKET,
    TK_OPEN_BRACE,
    TK_OPEN_PAREN,
    TK_COMMA,
    TK_DOT,
    TK_SEMI_COLON,
    TK_COLON,
    TK_CLOSE_BRACKET,
    TK_CLOSE_BRACE,
    TK_CLOSE_PAREN,
    TK_ELLIPSIS,
    TK_OPTIONAL_DOT,
    TK_ARROW,
    TK_TEMPLATE_NO_SUBSTITUTION,
    TK_TEMPLATE_HEAD,
    TK_TEMPLATE_MIDDLE,
    TK_TEMPLATE_TAIL
};

enum ParseError {
    PE_OK                   = 0,
    PE_SYNTAX_ERROR,
    PE_TYPE_ERROR,
    PE_RANGE_ERROR,
    PE_REFERECNE_ERROR,
};

cstr_t parseErrorToString(ParseError err);

/**
 * 在代码解析阶段可抛出异常，不能在执行 bytecode 阶段抛出异常.
 */
class ParseException : public std::exception {
public:
    ParseException(ParseError err, cstr_t format, ...);

    ParseError              error;
    string                  message;
    
};

struct Token {
    TokenType               type;
    OpCode                  opr;        // 当 Token 为 TK_ASSIGN_X 和二元操作符是，对应的 OpCode.
    bool                    newLineBefore;
    uint32_t                line, col;
    uint32_t                len;
    uint8_t                 *buf;

    // 当 type 为 TK_NUMBER 时有效
    double                  number;
};

bool operator ==(const Token &token, const char *name);

inline SizedString tokenToSizedString(const Token &token) {
    return SizedString(token.buf, token.len);
}

inline bool isKeyword(TokenType type) {
    return type >= TK_EOF && type < _TK_END_KEYWORD;
}

inline bool canKeywordBeVarName(TokenType type) {
    return type >= TK_EOF && type < _TK_END_VAR_KEYWORD;
}

inline bool canTokenBeMemberName(TokenType type) {
    return (type >= TK_EOF && type < _TK_END_KEYWORD) || type == TK_NAME
        || type == TK_NUMBER || type == TK_STRING;
}

uint8_t *parseNumber(uint8_t *start, uint8_t *end, double &retValue);
uint8_t *parseNumber(const SizedString &str, double &retValue);

class JSLexer {
public:
    JSLexer(ResourcePool *resPool, const char *buf, size_t len);

protected:
    void _readToken();
    void _peekToken();

    //
    // 词法解析相关的内部 API
    //
    bool _isUncs2WhiteSpace(uint8_t code);
    void _readString(uint8_t quote);
    void _readNumber();
    void _readRegexp();
    void _readName();
    void  _readInTemplateMid();
    void _skipLineComment();
    void _skipMultilineComment();

    void _allowRegexp() { _prevTokenType = TK_EOF; }
    
    SizedString _escapeString(const SizedString &str);

    void _parseError(ParseError err, cstr_t format, ...);

protected:
    ResourcePool            *_resPool;
    AllocatorPool           &_pool;

    Token                   _curToken, _nextToken;

    const char              *_fileName;
    uint8_t                 *_buf, *_bufPos, *_bufEnd;
    int                     _line, _col;
    bool                    _newLineBefore;

    TokenType               _prevTokenType;
    uint8_t                 *_prevTokenEndPos;

};

#endif /* Lexer_hpp */
