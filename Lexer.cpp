//
//  Lexer.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "Lexer.hpp"
#include <math.h>
#include "AST.hpp"


bool operator ==(const Token &token, const char *name) {
    return token.buf[0] == name[0] && tokenToSizedString(token).equal(name);
}

bool isRegexAllowed(TokenType token) {
    if (token == TK_CLOSE_PAREN || token == TK_POSTFIX || token == TK_NAME ||
            token == TK_STRING || token == TK_CLOSE_BRACKET || token == TK_CLOSE_PAREN)
        return false;

    return true;
}

bool isIdentifierStart(uint8_t code) {
    return (code >= 'a' && code <= 'z') || (code >= 'a' && code <= 'z') || code == '_' || code == '$' || code >= 0xaa;
}

cstr_t KEYWORDS[] = { "break", "case", "catch", "const", "continue", "debugger", "default",
    "delete", "do", "else", "false", "finally", "for", "function", "if", "in", "instanceof", "new",
    "null", "return", "switch", "throw", "true", "try", "typeof", "var", "void", "while", "with" };

TokenType KEYWORDS_TYPE[] = { TK_BREAK, TK_CASE, TK_CATCH, TK_CONST, TK_CONTINUE, TK_DEBUGGER, TK_DEFAULT,
    TK_DELETE, TK_DO, TK_ELSE, TK_FALSE, TK_FINALLY, TK_FOR, TK_FUNCTION, TK_IF, TK_IN, TK_INSTANCEOF, TK_NEW,
    TK_NULL, TK_RETURN, TK_SWITCH, TK_THROW, TK_TRUE, TK_TRY, TK_TYPEOF, TK_VAR, TK_VOID, TK_WHILE, TK_WITH };

struct KeywordCompareLess {
    bool operator()(cstr_t left, const SizedString &right) {
        const uint8_t *p1 = (uint8_t *)left;
        const uint8_t *p2 = right.data, *p2End = right.data + right.len;

        for (; *p1 && p2 < p2End; p1++, p2++) {
            if (*p1 == *p2) {
                continue;
            }
            return *p1 < *p2;
        }

        return p2 != p2End; // x < xy
    }
};

JSLexer::JSLexer(ResourcePool *resPool, const char *buf, size_t len) : _resPool(resPool) {
    _error = PE_OK;

    _fileName = nullptr;
    _buf = (uint8_t *)buf;
    _bufEnd = _buf + len;
    _bufPos = _buf;

    _line = 0;
    _col = 0;
    _newLineBefore = true;

    _prevTokenType = TK_SEMI_COLON; // 缺省的前一个 token 为 ';'

    memset(&_curToken, 0, sizeof(_curToken));
    memset(&_nextToken, 0, sizeof(_nextToken));
    _curToken.type = _nextToken.type = TK_ERR;
}

void JSLexer::_readToken() {
    if (_nextToken.type != TK_ERR) {
        _curToken = _nextToken;
        _nextToken.type = TK_ERR;
        return;
    }

    uint8_t code = *_bufPos++;
    _newLineBefore = false;
    while (code == 32 || (9 <= code && code <= 13) || (code > 0x80 && _isUncs2WhiteSpace(code))) {
        if (code == '\n') { // \n
            _newLineBefore = true;
            _line++;
            _col = 0;
        }
        code = *_bufPos++;
    }

    _prevTokenType = _curToken.type;

    _curToken.newLineBefore = _newLineBefore;
    _curToken.type = TK_ERR;
    _curToken.buf = _bufPos - 1;
    _curToken.line = _line;
    _curToken.col = _col;

    if (_bufPos >= _bufEnd) {
        _curToken.type = TK_EOF;
        return;
    }

    switch (code) {
        case '"':
        case '\'':
            _readString(code);
            return; // _readString 已经重新设置了 _curToken.buf, 不能沿用后面的逻辑
        case '.':
            code = *_bufPos;
            if (isDigit(code)) {
                _readNumber('.');
            } else if (code == '.' && _bufPos[1] == '.') {
                _curToken.type = TK_ELLIPSIS;
            } else {
                _curToken.type = TK_DOT;
            }
            break;
        case '/':
            code = *_bufPos;
            switch (code) {
                case '/':
                    _bufPos++;
                    _skipLineComment();
                    return;
                case '*':
                    _bufPos++;
                    _skipMultilineComment();
                    return;
            }

            if (!isRegexAllowed(_prevTokenType)) {
                _curToken.opr = OP_DIV;
                if (code == '=') {
                    _bufPos++;
                    _curToken.type = TK_ASSIGN_X;
                } else {
                    _curToken.type = TK_MUL;
                }
                return;
            }

            _readRegexp();
            break;
        case '!':
            code = *_bufPos;
            if (code == '=') {
                code = *_bufPos++;
                _curToken.type = TK_EQUALITY;
                if (code == '=') {
                    // !==
                    _bufPos++;
                    _curToken.opr = OP_INEQUAL_STRICT;
                } else { // !=
                    _curToken.opr = OP_INEQUAL;
                }
            } else {
                // !
                _curToken.type = TK_UNARY_PREFIX;
                _curToken.opr = OP_LOGICAL_NOT;
            }
            break;
        case '%':
            code = *_bufPos;
            _curToken.opr = OP_MOD;
            if (code == '=') {
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
            } else {
                _curToken.type = TK_MUL;
            }
            break;
        case '&':
            code = *_bufPos;
            if (code == '&') {
                // &&
                _bufPos++;
                _curToken.type = TK_LOGICAL_AND;
                _curToken.opr = OP_LOGICAL_AND;
            } else if (code == '=') {
                // &=
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
                _curToken.opr = OP_BIT_AND;
            } else {
                // &
                _curToken.type = TK_BIT_AND;
                _curToken.opr = OP_BIT_AND;
            }
            break;
        case '*':
            code = *_bufPos;
            if (code == '*') {
                _bufPos++;
                if (*_bufPos == '=') {
                    // **=
                    _bufPos++;
                    _curToken.type = TK_ASSIGN_X;
                    _curToken.opr = OP_EXP;
                } else {
                    // **
                    _curToken.type = TK_EXP;
                    _curToken.opr = OP_EXP;
                }
            } else if (code == '=') {
                // *=
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
                _curToken.opr = OP_MUL;
            } else {
                // *
                _curToken.type = TK_MUL;
                _curToken.opr = OP_MUL;
            }
            break;
        case '+':
            code = *_bufPos;
            if (code == '+') {
                // ++
                _bufPos++;
                _curToken.type = TK_POSTFIX;
            } else if (code == '=') {
                // +=
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
                _curToken.opr = OP_ADD;
            } else {
                // +
                _curToken.type = TK_ADD;
                _curToken.opr = OP_ADD;
            }
            break;
        case '-':
            code = *_bufPos;
            if (code == '-') {
                // --
                _bufPos++;
                _curToken.type = TK_POSTFIX;
            } else if (code == '=') {
                // -=
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
                _curToken.opr = OP_SUB;
            } else {
                // -
                _curToken.type = TK_ADD;
                _curToken.opr = OP_SUB;
            }
            break;
        case '<':
            code = *_bufPos;
            if (code == '<') {
                _bufPos++;
                if (*_bufPos == '=') {
                    // <<=
                    _bufPos++;
                    _curToken.type = TK_ASSIGN_X;
                    _curToken.opr = OP_LEFT_SHIFT;
                } else {
                    // <<
                    _curToken.type = TK_SHIFT;
                    _curToken.opr = OP_LEFT_SHIFT;
                }
            } else {
                if (code == '=') {
                    // <=
                    _bufPos++;
                    _curToken.type = TK_RATIONAL;
                    _curToken.opr = OP_LESS_EQUAL_THAN;
                } else { // <
                    _curToken.type = TK_RATIONAL;
                    _curToken.opr = OP_LESS_THAN;
                }
            }
            break;
        case '>':
            code = *_bufPos;
            if (code == '>') {
                _bufPos++;
                code = *_bufPos;
                if (code == '=') {
                    // >>=
                    _bufPos++;
                    _curToken.type = TK_ASSIGN_X;
                    _curToken.opr = OP_RIGHT_SHIFT;
                } else if (code == '>') {
                    if (*_bufPos == '=') {
                        // >>>=
                        _bufPos++;
                        _curToken.type = TK_ASSIGN_X;
                        _curToken.opr = OP_UNSIGNED_RIGHT_SHIFT;
                    } else {
                        // >>>
                        _curToken.type = TK_SHIFT;
                        _curToken.opr = OP_UNSIGNED_RIGHT_SHIFT;
                    }
                } else {
                    // >>
                    _curToken.type = TK_SHIFT;
                    _curToken.opr = OP_RIGHT_SHIFT;
                }
            } else {
                _curToken.type = TK_RATIONAL;
                if (code == '=') {
                    // >=
                    _bufPos++;
                    _curToken.opr = OP_GREATER_EQUAL_THAN;
                } else { // >
                    _curToken.opr = OP_GREATER_THAN;
                }
            }
            break;
        case '=':
            code = *_bufPos;
            if (code == '=') {
                _bufPos++;
                if (*_bufPos == '=') {
                    // ===
                    _bufPos++;
                    _curToken.opr = OP_EQUAL_STRICT;
                } else { // ==
                    _curToken.opr = OP_EQUAL;
                }
                _curToken.type = TK_EQUALITY;
            } else {
                if (code == '>') {
                    // =>
                    _bufPos++;
                    _curToken.type = TK_ARROW;
                } else {
                    // =
                    _curToken.type = TK_ASSIGN;
                }
            }
            break;
        case '?':
            if (*_bufPos == '.' && isDigit(_bufPos[1])) {
                // ?.
                _bufPos++;
                _curToken.type = TK_OPTIONAL_DOT;
            } else {
                // ?
                _curToken.type = TK_CONDITIONAL;
                _curToken.opr = OP_CONDITIONAL;
            }
            break;
        case '^':
            if (*_bufPos == '=') {
                // ^=
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
            } else {
                // ^
                _curToken.type = TK_BIT_XOR;
            }
            _curToken.opr = OP_BIT_XOR;
            break;
        case '|':
            code = *_bufPos;
            if (code == '=') {
                // |=
                _bufPos++;
                _curToken.type = TK_ASSIGN_X;
                _curToken.opr = OP_BIT_OR;
            } else {
                if (code == '|') {
                    // ||
                    _bufPos++;
                    _curToken.type = TK_LOGICAL_OR;
                    _curToken.opr = OP_LOGICAL_OR;
                } else { // |
                    _curToken.type = TK_BIT_OR;
                    _curToken.opr = OP_BIT_OR;
                }
            }
            break;
        case '`':
            // Ignore '`'
            _curToken.buf++;

            while (_bufPos < _bufEnd) {
                code = *_bufPos++;
                if (code == '\\') {
                    // escape char '\\'
                    _bufPos++;
                } else if (code == '$' && *_bufPos == '{') {
                    // ${
                    _bufPos++;
                    _curToken.len = (int)(_bufPos - _curToken.buf - 1);
                    _curToken.type = TK_TEMPLATE_HEAD;
                    return;
                } else if (code == '`') {
                    // `
                    _curToken.len = (int)(_bufPos - _curToken.buf - 1);
                    _curToken.type = TK_TEMPLATE_NO_SUBSTITUTION;
                    return;
                }
            }

            if (_curToken.type == TK_ERR) {
                _parseError(PE_TEMPLATE_STRING_NOT_CLOSED, "Template string is NOT closed.");
            }
            break;
        case '~':
            _curToken.type = TK_UNARY_PREFIX;
            _curToken.opr = OP_BIT_NOT;
            break;
        case '(': _curToken.type = TK_OPEN_PAREN; break;
        case ')': _curToken.type = TK_CLOSE_PAREN; break;
        case ',': _curToken.type = TK_COMMA; break;
        case ':': _curToken.type = TK_COLON; break;
        case ';': _curToken.type = TK_SEMI_COLON; break;
        case '[': _curToken.type = TK_OPEN_BRACKET; break;
        case ']': _curToken.type = TK_CLOSE_BRACKET; break;
        case '{': _curToken.type = TK_OPEN_BRACE; break;
        case '}': _curToken.type = TK_CLOSE_BRACE; break;
        default:
            if ((code >= 'a' && code <= 'z') || (code >= 'a' && code <= 'z') || code == '_' || code == '$' || code >= 0xaa) {
                _readName();

                SizedString str(_curToken.buf, (size_t)(_bufPos - _curToken.buf));
                auto p = lower_bound(KEYWORDS, KEYWORDS + CountOf(KEYWORDS), str, KeywordCompareLess());
                if (p < KEYWORDS + CountOf(KEYWORDS)) {
                    if (str.equal(*p)) {
                        _curToken.type = KEYWORDS_TYPE[p - KEYWORDS];
                        break;
                    }
                }

                _curToken.type = TK_NAME;
            } else if (code >= '0' && code <= '9') {
                _readNumber(code);
            } else {
                _parseError(PE_UNEXPECTED_CHAR,  "Unexpected char: %d(%c)", code, code);
            }
            break;
    }

    _curToken.len = (int)(_bufPos - _curToken.buf);
}

void JSLexer::_peekToken() {
    if (_nextToken.type == TK_ERR) {
        auto tmp = _curToken;
        _readToken();
        _nextToken = _curToken;
        _curToken = tmp;
    }
}

bool JSLexer::_isUncs2WhiteSpace(uint8_t code) {
    static uint16_t WHITE_SPACES[] = {160, 6158, 8192, 8193, 8194, 8195, 8196, 8197, 8198, 8199, 8200, 8201, 8202, 8203, 8239, 8287, 12288};

    return std::binary_search(WHITE_SPACES, WHITE_SPACES + CountOf(WHITE_SPACES), code);
}

void JSLexer::_readString(uint8_t quote) {
    uint8_t c;
    bool needEscape = false;
    auto start = _bufPos;

    do {
        c = *_bufPos++;
        if (c == '\n') { // \n
            _parseError(PE_UNENCLOSED_STRING, "Unenclosed string.");
            return;
        } if (c == '\\') {  // '\\'
            // Escape next char.
            needEscape = true;
            ++_bufPos;
        }
    } while (_bufPos < _bufEnd && c != quote);

    SizedString str(start, size_t(_bufPos - start - 1));
    if (needEscape) {
        str = _escapeString(str);
    }
    _curToken.buf = (uint8_t *)str.data;
    _curToken.len = (int)str.len;

    _curToken.type = TK_STRING;
}

void JSLexer::_readNumber(uint8_t ch) {
    double value = 0;

    if (ch == '0') {
        int64_t n = 0;
        ch = *_bufPos++;
        if (ch == 'x' || ch == 'X') {
            // Read hex numbers
            while (true) {
                ch = *_bufPos++;
                if (isDigit(ch)) {
                    n = n * 16 + ch - '0';
                } else if (ch >= 'a' && ch <= 'f') {
                    n = n * 16 + (ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    n = n * 16 + (ch - 'A' + 10);
                } else {
                    break;
                }
            }
            value = n;
        } else if (ch == 'b' || ch == 'B') {
            // Read binary numbers
            while (true) {
                ch = *_bufPos++;
                if (ch == '0') {
                    n = n * 2;
                } else if (ch == '1') {
                    n = n * 2 + 1;
                } else {
                    break;
                }
            }
            value = n;
        } else {
            // Read octal numbers
            if (ch == 'o' || ch == 'O') {
                _bufPos++;
            }
            while (true) {
                ch = *_bufPos++;
                if ('0' <= ch && ch <= '7') {
                    n = n * 8 + ch - '0';
                } else {
                    break;
                }
            }
            value = n;
        }
    } else {
        // Number
        int64_t n = 0;
        while (isDigit(ch)) {
            n = n * 10 + ch - '0';
            ch = *_bufPos++;
        }

        value = n;
        if (ch == '.') {
            n = 0;
            int64_t factor = 1;

            ch = *_bufPos++;
            // Numbers after .
            while (isDigit(ch)) {
                n = n * 10 + ch - '0';
                factor *= 10;
                ch = *_bufPos++;
            }

            value += (double)n / factor;
        }

        if (ch == 'e' || ch == 'E') {// E,e
            ch = *_bufPos++;

            bool nagative = false;
            if (ch == '-') {
                nagative = true;
                ch = *_bufPos++;
            } else if (ch == '+') {
                ch = *_bufPos++;
            }

            n = 0;
            while (isDigit(ch)) {
                ch = *_bufPos++;
                n = n * 10 + ch - '0';
            }

            if (nagative) {
                value /= pow(10, n);
            } else {
                value *= pow(10, n);
            }
        }
    }

    _curToken.number = value;
    _curToken.type = TK_NUMBER;

    if (isIdentifierStart(ch))
        _parseError(PE_UNEXPECTED_NUMBER_ENDING, "unexpected number ending.");
    _bufPos--; // Seek back one char
}

void JSLexer::_readRegexp() {
    bool inBracket = false;
    uint8_t ch;

    do {
        ch = *_bufPos++;
        if (!ch || ch == '\n') {
            _parseError(PE_UNENCLOSED_REGEX, "Unenclosed regular expression.");
            return;
        }
        if (ch == '[') {
            inBracket = true;
        }
        if (ch == '\\') {
            // Escape next char.
            ++_bufPos;
            continue;
        }
        if (ch == ']') {
            inBracket = false;
        }
    } while (ch != '/' || inBracket);    // / or in []

    // mods
    _readName();
    _curToken.type = TK_REGEX;
}

void JSLexer::_readName() {
    uint8_t code = *_bufPos++;
    while (isIdentifierStart(code) || isDigit(code)) {
        code = *_bufPos++;
    }

    _bufPos--;
}

void JSLexer::_readInTemplateMid() {
    // 读取 template mid 中的内容
    _curToken.buf = _bufPos;

    while (_bufPos < _bufEnd) {
        auto code = *_bufPos;
        _bufPos++;
        if (code == '\\') {
            // escape char '\\'
            _bufPos++;
        } else if (code == '$' && _bufPos[1] == '{') {
            // ${
            _curToken.type = TK_TEMPLATE_MIDDLE;
            _curToken.len = (uint32_t)(_bufPos - _curToken.buf);
            _bufPos++;
            return;
        } else if (code == '`') {
            // `
            _curToken.type = TK_TEMPLATE_TAIL;
            _curToken.len = (uint32_t)(_bufPos - _curToken.buf);
            return;
        }
    }

    _parseError(PE_TEMPLATE_STRING_NOT_CLOSED, "Unexpected template string ending.");
}

void JSLexer::_skipLineComment() {
    while (_bufPos < _bufEnd) {
        if (*_bufPos++ == '\n') {
            _newLineBefore = true;
            break;
        }
    }

    auto tmpNewLineBefore = _newLineBefore;
    auto tmp = _prevTokenType;
    _readToken();
    tmp = _prevTokenType;
    _curToken.newLineBefore |= tmpNewLineBefore;
}

void JSLexer::_skipMultilineComment() {
    while (_bufPos < _bufEnd) {
        if (*_bufPos == '\n') {
            _newLineBefore = true;
            _line++;
            _col = 0;
        }

        if (*_bufPos++ == '*' && *_bufPos == '/') {
            auto tmp = _prevTokenType;
            auto tmpNewLineBefore = _newLineBefore;
            _readToken();
            tmp = _prevTokenType;
            _curToken.newLineBefore |= tmpNewLineBefore;
            return;
        }
    }
}

SizedString JSLexer::_escapeString(const SizedString &str) {
    uint8_t c;
    auto p = str.data;
    auto end = str.data + str.len;
    auto out = (uint8_t *)_resPool->pool.allocate(str.len);
    auto po = out;

    do {
        c = *p++;
        if (c == '\\') {  // '\\'
            // Escape next char.
            *po++ = *p++;
            ++p;
        } else {
            *po++ = c;
        }
    } while (p < end);

    return SizedString(out, size_t(po - out));
}

void JSLexer::_parseError(ParseError err, cstr_t format, ...) {
    if (_error != PE_OK) {
        return;
    }

    _error = err;
    CStrPrintf strf;

    va_list        args;

    va_start(args, format);
    strf.vprintf(format, args);
    va_end(args);

    _errorMessage = strf.c_str();
}
