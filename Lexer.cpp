//
//  Lexer.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "Lexer.hpp"
#include <math.h>
#include "AST.hpp"

uint8_t *parseNumber(uint8_t *start, uint8_t *end, double &retValue) {
    retValue = 0;
    if (start >= end) {
        return end;
    }

    if (start + 1 < end && *start == '0' && start[1] != '.') {
        int64_t n = 0;
        start++;
        auto ch = *start;
        if ((ch == 'x' || ch == 'X') && start + 1 < end) {
            // Read hex numbers
            start++;
            while (start < end) {
                ch = *start;
                if (isDigit(ch)) {
                    n = n * 16 + ch - '0';
                } else if (ch >= 'a' && ch <= 'f') {
                    n = n * 16 + (ch - 'a' + 10);
                } else if (ch >= 'A' && ch <= 'F') {
                    n = n * 16 + (ch - 'A' + 10);
                } else {
                    break;
                }
                start++;
            }
            retValue = n;
        } else if ((ch == 'b' || ch == 'B') && start + 1 < end) {
            // Read binary numbers
            start++;
            while (start < end) {
                ch = *start;
                if (ch == '0') {
                    n = n * 2;
                } else if (ch == '1') {
                    n = n * 2 + 1;
                } else {
                    break;
                }
                start++;
            }
            retValue = n;
        } else {
            // Read octal numbers
            if ((ch == 'o' || ch == 'O') && start + 1 < end) {
                start++;
            }
            while (start < end) {
                ch = *start;
                if ('0' <= ch && ch <= '7') {
                    n = n * 8 + ch - '0';
                    start++;
                } else {
                    break;
                }
            }
            retValue = n;
        }
    } else {
        // Number
        int64_t n = 0;
        while (start < end && isDigit(*start)) {
            n = n * 10 + *start - '0';
            start++;
        }

        retValue = n;
        if (start + 1 < end && *start == '.') {
            start++;
            n = 0;
            int64_t factor = 1;

            // Numbers after .
            while (start < end) {
                if (!isDigit(*start)) {
                    break;
                }

                n = n * 10 + *start - '0';
                factor *= 10;
                start++;
            }

            retValue += (double)n / factor;
        }

        if (start + 1 < end && (*start == 'e' || *start == 'E')) {// E,e
            start++;

            bool nagative = false;
            if (*start == '-') {
                nagative = true;
                start++;
            } else if (*start == '+') {
                start++;
            }

            n = 0;
            while (start < end && isDigit(*start)) {
                n = n * 10 + *start - '0';
                start++;
            }

            if (nagative) {
                retValue /= pow(10, n);
            } else {
                retValue *= pow(10, n);
            }
        }
    }

    return start;
}

uint8_t *parseNumber(const SizedString &str, double &retValue) {
    return parseNumber(str.data, str.data + str.len, retValue);
}

bool jsStringToNumber(const SizedString &str, double &retValue) {
    auto tmp = str;
    tmp.trim();
    if (tmp.len == 0) {
        retValue = 0;
        return true;
    }

    auto p = parseNumber(tmp, retValue);
    if (p != tmp.data + tmp.len) {
        retValue = NAN;
        return false;
    }

    return true;
}

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
    return (code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z') || code == '_' || code == '$' || code >= 0xaa;
}

struct Keyword {
    cstr_t                  name;
    TokenType               token;
};

//
// !!! KEYWORDS 必须按照从小到大的顺序排序，因为后面会使用二分查找来搜索.
//
Keyword KEYWORDS[] = {
    { "break", TK_BREAK },
    { "case", TK_CASE },
    { "catch", TK_CATCH },
    { "const", TK_CONST },
    { "continue", TK_CONTINUE },
    { "debugger", TK_DEBUGGER },
    { "default", TK_DEFAULT },
    { "delete", TK_DELETE },
    { "do", TK_DO },
    { "else", TK_ELSE },
    { "false", TK_FALSE },
    { "finally", TK_FINALLY },
    { "for", TK_FOR },
    { "function", TK_FUNCTION },
    { "if", TK_IF },
    { "in", TK_IN },
    { "instanceof", TK_INSTANCEOF },
    { "let", TK_LET },
    { "new", TK_NEW },
    { "null", TK_NULL },
    { "return", TK_RETURN },
    { "switch", TK_SWITCH },
    { "throw", TK_THROW },
    { "true", TK_TRUE },
    { "try", TK_TRY },
    { "typeof", TK_TYPEOF },
    { "var", TK_VAR },
    { "void", TK_VOID },
    { "while", TK_WHILE },
    { "with", TK_WITH },
};

struct KeywordCompareLess {
    bool operator()(const Keyword &left, const SizedString &right) {
        const uint8_t *p1 = (uint8_t *)left.name;
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

static cstr_t PARSE_ERROR_NAMES[] = {
    "PE_OK",
    "Error: ",
    "SyntaxError: ",
    "TypeError: ",
    "RangeError: ",
    "ReferenceError: ",
};

cstr_t parseErrorToString(JsErrorType err) {
    assert(err >= 0 && err < CountOf(PARSE_ERROR_NAMES));

    return PARSE_ERROR_NAMES[err];
}

ParseException::ParseException(JsErrorType err, cstr_t format, ...) {
    error = err;
    CStrPrintf strf;

    va_list        args;

    va_start(args, format);
    strf.vprintf(format, args);
    va_end(args);

    assert(err > 0 && err < CountOf(PARSE_ERROR_NAMES));
    message.assign(strf.c_str());
}

JSLexer::JSLexer(ResourcePool *resPool, const char *buf, size_t len) : _resPool(resPool), _pool(resPool->pool) {
    _fileName = nullptr;
    _buf = (uint8_t *)buf;
    _bufEnd = _buf + len;
    _bufPos = _buf;

    _line = 0;
    _col = 0;
    _newLineBefore = true;

    _prevTokenType = TK_SEMI_COLON; // 缺省的前一个 token 为 ';'
    _prevTokenEndPos = _buf;

    memset(&_curToken, 0, sizeof(_curToken));
    memset(&_nextToken, 0, sizeof(_nextToken));
    _curToken.type = _nextToken.type = TK_ERR;
}

void JSLexer::_readToken() {
    if (_nextToken.type != TK_ERR) {
        _prevTokenType = _curToken.type;
        _prevTokenEndPos = _curToken.buf + _curToken.len;

        _curToken = _nextToken;
        _nextToken.type = TK_ERR;
        return;
    }

    if (_bufPos >= _bufEnd) {
        _curToken.type = TK_EOF;
        return;
    }

    uint8_t code = *_bufPos++;
    _newLineBefore = false;
    while (_bufPos < _bufEnd && (code == 32 || (9 <= code && code <= 13) || (code > 0x80 && _isUncs2WhiteSpace(code)))) {
        if (code == '\n') { // \n
            _newLineBefore = true;
            _line++;
            _col = 0;
        }
        code = *_bufPos++;
    }

    _prevTokenType = _curToken.type;
    _prevTokenEndPos = _curToken.buf + _curToken.len;

    _curToken.newLineBefore = _newLineBefore;
    _curToken.buf = _bufPos - 1;
    _curToken.line = _line;
    _curToken.col = _col;

    switch (code) {
        case '"':
        case '\'':
            _readString(code);
            return; // _readString 已经重新设置了 _curToken.buf, 不能沿用后面的逻辑
        case '.':
            code = *_bufPos;
            if (isDigit(code)) {
                _readNumber();
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
                ++_bufPos;
                code = *_bufPos;
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

            _parseError(PE_SYNTAX_ERROR, "Template string is NOT closed.");
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
            if ((code >= 'a' && code <= 'z') || (code >= 'A' && code <= 'Z') || code == '_' || code == '$' || code >= 0xaa) {
                _readName();

                SizedString str(_curToken.buf, (size_t)(_bufPos - _curToken.buf));
                auto p = lower_bound(KEYWORDS, KEYWORDS + CountOf(KEYWORDS), str, KeywordCompareLess());
                if (p < KEYWORDS + CountOf(KEYWORDS)) {
                    if (str.equal(p->name)) {
                        _curToken.type = p->token;
                        break;
                    }
                }

                _curToken.type = TK_NAME;
            } else if (code >= '0' && code <= '9') {
                _readNumber();
            } else {
                if (_bufPos >= _bufEnd) {
                    _curToken.type = TK_EOF;
                    return;
                }
                _parseError(PE_SYNTAX_ERROR,  "Invalid or unexpected token: %d(%c)", code, code);
            }
            break;
    }

    _curToken.len = (int)(_bufPos - _curToken.buf);
}

void JSLexer::_peekToken() {
    if (_nextToken.type == TK_ERR) {
        auto prevTokenType = _prevTokenType;
        auto prevTokenEndPos = _prevTokenEndPos;
        auto tmp = _curToken;
        _readToken();
        _nextToken = _curToken;
        _curToken = tmp;
        _prevTokenType = prevTokenType;
        _prevTokenEndPos = prevTokenEndPos;
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
            _parseError(PE_SYNTAX_ERROR, "Invalid or unexpected token");
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

void JSLexer::_readNumber() {
    _bufPos--;
    _bufPos = parseNumber(_bufPos, _bufEnd, _curToken.number);

    _curToken.type = TK_NUMBER;

    if (isIdentifierStart(*_bufPos))
        _parseError(PE_SYNTAX_ERROR, "unexpected number ending.");
}

void JSLexer::_readRegexp() {
    bool inBracket = false;
    uint8_t ch;

    do {
        ch = *_bufPos++;
        if (!ch || ch == '\n') {
            _parseError(PE_SYNTAX_ERROR, "Invalid regular expression: missing /");
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

    _parseError(PE_SYNTAX_ERROR, "Unexpected template string ending.");
}

void JSLexer::_skipLineComment() {
    while (_bufPos < _bufEnd) {
        if (*_bufPos++ == '\n') {
            _newLineBefore = true;
            break;
        }
    }

    auto tmpPrevTokenEndPos = _prevTokenEndPos;
    auto tmpNewLineBefore = _newLineBefore;
    _readToken();
    _curToken.newLineBefore |= tmpNewLineBefore;
    _prevTokenEndPos = tmpPrevTokenEndPos;
}

void JSLexer::_skipMultilineComment() {
    while (_bufPos < _bufEnd) {
        if (*_bufPos == '\n') {
            _newLineBefore = true;
            _line++;
            _col = 0;
        }

        if (*_bufPos++ == '*' && *_bufPos == '/') {
            _bufPos++;
            auto tmpPrevTokenEndPos = _prevTokenEndPos;
            auto tmpNewLineBefore = _newLineBefore;
            _readToken();
            _curToken.newLineBefore |= tmpNewLineBefore;
            _prevTokenEndPos = tmpPrevTokenEndPos;
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

void JSLexer::_parseError(JsErrorType err, cstr_t format, ...) {
    CStrPrintf strf;

    va_list        args;

    va_start(args, format);
    strf.vprintf(format, args);
    va_end(args);

    throw ParseException(err, "%s", strf.c_str());
}
