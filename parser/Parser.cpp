//
//  Parser.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "Parser.hpp"
#include "VirtualMachine.hpp"
#include "Statement.hpp"


static SizedString NAME_ASYNC("async");
static SizedString NAME_TARGET("target");
static SizedString NAME_OF("of");
static SizedString NAME_EVAL("eval");


inline bool isTokenNameEqual(const Token &token, SizedString &name) {
    return token.type == TK_NAME && tokenToSizedString(token).equal(name);
}

JSParser::JSParser(VMRuntimeCommon *rtc, ResourcePool *resPool, const char *buf, size_t len) : JSLexer(resPool, buf, len) {
    _runtimeCommon = rtc;
    if (rtc) {
        _nextStringIdx = (uint32_t)rtc->stringValues.size();
        _nextDoubleIdx = (uint32_t)rtc->doubleValues.size();
    } else {
        _nextStringIdx = 0;
        _nextDoubleIdx = 0;
    }

    _headIdRefs = nullptr;
    _curFunction = nullptr;
    _curFuncScope = nullptr;
    _curScope = nullptr;
}

Function *JSParser::parse(Scope *parent, bool isExpr) {
    _headIdRefs = nullptr;

    if (parent) {
        _curScope = parent;
        _curFuncScope = parent->function->scope;
        _curFunction = parent->function;
    } else {
        _curFunction = nullptr;
        _curFuncScope = nullptr;
        _curScope = nullptr;
    }

    _readToken();

    auto function = _enterFunction(_curToken, true);

    if (isExpr) {
        function->astNodes.push_back(PoolNew(_pool, JsStmtReturnValue)(_expectMultipleExpression()));
    } else {
        while (_curToken.type != TK_EOF) {
            function->astNodes.push_back(_expectStatment());
        }
    }

    _checkExpressionObjects();

    _leaveFunction();

    // 先简单优化一下 scope 的层次, 在后面查找标识符的时候会快点
    _reduceScopeLevels(function);

    // function 是代码片段，其中的变量和子函数并非真的在 function 函数内，
    // 而是在父函数中，所以依照父函数的地址分配存储空间
    _relocateIdentifierInParentFunction(function, parent->function);

    // 建立标识符的引用、作用域关系
    _buildExprIdentifiers();

    // 分析标识符地址
    _allocateIdentifierStorage(function->scope, 0);

    return function;
}

IJsNode *JSParser::_expectStatment() {
    _checkExpressionObjects();

    switch (_curToken.type) {
        case TK_STRING: {
            _peekToken();
            if (_nextToken.type == TK_SEMI_COLON || _nextToken.newLineBefore || _nextToken.type == TK_CLOSE_BRACE) {
                // 是一个字符串 statement, 需判断是不是指令?
                if (tokenToSizedString(_curToken).equal("use strict")) {
                    _curFunction->isStrictMode = true;
                } else {
                    // 忽略其他的字符串语句.
                }
                _readToken();
                _expectSemiColon();
                return PoolNew(_pool, JsStmtEmpty)();
            }
            return _expectExpressionStmt();
        }
        case TK_NAME: {
            _peekToken();
            if (_nextToken.type == TK_COLON) {
                return _expectLabelStmt();
            }
            return _expectExpressionStmt();
        }
        case TK_OPEN_BRACE:
            return _expectBlock();
        case TK_SEMI_COLON:
            // Empty Statement
            _readToken();
            return PoolNew(_pool, JsStmtEmpty);

        case TK_BREAK:
            _readToken();
            return _expectBreak();

        case TK_CONTINUE:
            _readToken();
            return _expectContinue();

        case TK_DEBUGGER: {
            _readToken();
            _expectSemiColon();
            return PoolNew(_pool, JsStmtDebugger)();
        }

        case TK_DO: {
            _readToken();
            _enterBreakContinueArea();
            auto stmt = _expectStatment();
            _leaveBreakContinueArea();
            _expectToken(TK_WHILE);
            auto cond = _expectParenCondition();
            _expectSemiColon();
            return PoolNew(_pool, JsStmtDoWhile)(cond, stmt);
        }
        case TK_WHILE: {
            _readToken();
            auto cond = _expectParenCondition();
            _enterBreakContinueArea();
            auto stmt = _expectStatment();
            _leaveBreakContinueArea();
            return PoolNew(_pool, JsStmtWhile)(cond, stmt);
        }
        case TK_FOR:
            _readToken();
            return _expectForStatment();

        case TK_FUNCTION: {
            return _expectFunctionDeclaration();
        }

        case TK_IF: {
            _readToken();
            auto cond = _expectParenCondition();
            auto stmtTrue = _expectStatment();
            if (_curToken.type == TK_ELSE) {
                _readToken();
                auto stmtFalse = _expectStatment();
                return PoolNew(_pool, JsStmtIf)(cond, stmtTrue, stmtFalse);
            } else {
                return PoolNew(_pool, JsStmtIf)(cond, stmtTrue, nullptr);
            }
        }

        case TK_RETURN: {
            _readToken();
            if (_curToken.type == TK_SEMI_COLON) {
                _readToken();
            } else if (_canInsertSemicolon()) {
            } else {
                auto expr = _expectMultipleExpression();
                _expectSemiColon();
                return PoolNew(_pool, JsStmtReturnValue)(expr);
            }
            return PoolNew(_pool, JsStmtReturn)();
        }

        case TK_SWITCH:
            _readToken();
            return _expectSwitchStmt();

        case TK_THROW: {
            _readToken();
            if (_curToken.newLineBefore) {
                _parseError("Illegal newline after throw");
            }
            auto expr = _expectMultipleExpression();
            _expectSemiColon();
            return PoolNew(_pool, JsStmtThrow)(expr);
        }

        case TK_TRY:
            _readToken();
            return _expectTryStmt();

        case TK_LET: {
            _peekToken();
            if (_nextToken.type != TK_NAME && _nextToken.type != TK_OPEN_BRACKET && _nextToken.type != TK_OPEN_BRACE) {
                // let 作为变量名处理
                _curToken.type = TK_NAME;
                return _expectStatment();
            }
            // 接着执行，不 break
        }
        case TK_VAR:
        case TK_CONST: {
            auto declareType = _curToken.type;
            _readToken();
            auto node = _expectVariableDeclarationList(declareType);
            _expectSemiColon();
            return node;
        }

        case TK_WITH: {
            _readToken();
            // enterScope 必须要包含 expr, 因为其 with 的表达式会优先在此 scope 中搜索
            _enterScope();

            auto expr = _expectParenCondition();
            _curScope->hasWith = true;
            auto stmt = PoolNew(_pool, JsStmtWith)(expr, _expectStatment(), _curScope);

            _leaveScope();
            return stmt;
        }

        default:
            return _expectExpressionStmt();
    }
}

IJsNode *JSParser::_expectExpressionStmt() {
    auto expr = _expectMultipleExpression();
    _expectSemiColon();

    return PoolNew(_pool, JsStmtExpr)(expr);
}

IJsNode *JSParser::_expectLabelStmt() {
    assert(0);
    return nullptr;
}

IJsNode *JSParser::_expectBlock() {
    _enterScope();

    _expectToken(TK_OPEN_BRACE);

    auto stms = PoolNew(_pool, JsStmtBlock)(_resPool, _curScope);

    while (_curToken.type != TK_CLOSE_BRACE) {
        if (_curToken.type == TK_EOF) {
            _parseError("Unexpected end of input");
            break;
        }

        stms->push(_expectStatment());
    }
    _readToken();

    _leaveScope();

    return stms;
}

IJsNode *JSParser::_expectBreak() {
    if (_stackBreakContinueAreas.empty()) {
        _parseError("Illegal break statement");
    }

    return PoolNew(_pool, JsStmtBreak);
}

IJsNode *JSParser::_expectContinue() {
    if (_stackBreakContinueAreas.empty()) {
        _parseError("Illegal break statement");
    }

    if (!_stackBreakContinueAreas.back()) {
        _parseError("Illegal continue statement: no surrounding iteration statement");
    }
    return PoolNew(_pool, JsStmtContinue);
}

IJsNode *JSParser::_tryForInStatment() {
    auto type = _curToken.type;
    bool isIn = false;
    IJsNode *var = nullptr, *varOrg = nullptr;
    size_t countVars = 1;

    if (type == TK_VAR || type == TK_LET || type == TK_CONST) {
        // for (var
        _readToken();

        auto node =_expectVariableDeclarationList(type, true);
        assert(node->type == NT_VAR_DECLARTION_LIST);
        countVars = node->count();
        var = node->nodes[0];

        varOrg = node;
    } else {
        varOrg = _expectMultipleExpression(false);
        if (varOrg->type == NT_COMMA_EXPRESSION) {
            auto node = (JsCommaExprs *)varOrg;
            countVars = node->count();
            var = node->nodes[0];
        } else {
            // 单个 expression
            var = varOrg;
        }
    }

    if (_curToken.type != TK_IN && !isTokenNameEqual(_curToken, NAME_OF)) {
        return varOrg;
    }
    isIn = _curToken.type == TK_IN;
    _readToken();

    // for in/of
    if (countVars > 1) {
        if (isIn) {
            _parseError("Invalid left-hand side in for-in loop: Must have a single binding");
        } else {
            _parseError("Invalid left-hand side in for-loop");
        }
    }

    if (var->type == NT_ASSIGN ||
        (var->type == NT_ASSIGN_WITH_STACK_TOP && ((JsExprAssignWithStackTop *)var)->defaultValue())) {
        _parseError("for-of loop variable declaration may not have an initializer.");
    }

    switch (var->type) {
        case NT_ASSIGN_WITH_STACK_TOP:
        case NT_IDENTIFIER:
        case NT_MEMBER_DOT:
        case NT_MEMBER_INDEX:
        case NT_ARRAY:
        case NT_OBJECT:
            var->setBeingAssigned();
            break;
        default:
            _parseError("Invalid left-hand side in for-loop");
            break;
    }

    auto obj = _expectMultipleExpression();
    _expectToken(TK_CLOSE_PAREN);

    _allowRegexp();
    auto stmt = _expectStatment();

    return PoolNew(_pool, JsStmtForIn)(var, obj, stmt, isIn, _curScope);
}

IJsNode *JSParser::_expectForStatment() {
    // TODO: support await

    _enterScope();

    IJsNode *init = nullptr, *cond = nullptr, *finalExpr = nullptr;

    _expectToken(TK_OPEN_PAREN);
    if (_curToken.type != TK_SEMI_COLON) {
        init = _tryForInStatment();
        if (init->type == NT_FOR_IN) {
            _leaveScope();
            return init;
        }
    }

    // for (a;b;c)
    _expectToken(TK_SEMI_COLON);

    // 退出条件
    if (_curToken.type != TK_SEMI_COLON) {
        cond = _expectMultipleExpression();
    }
    _expectToken(TK_SEMI_COLON);

    if (_curToken.type != TK_CLOSE_PAREN) {
        finalExpr = _expectMultipleExpression();
    }
    _expectToken(TK_CLOSE_PAREN);

    _allowRegexp();
    _enterBreakContinueArea();
    auto stmt = _expectStatment();
    _leaveBreakContinueArea();

    stmt = PoolNew(_pool, JsStmtFor)(init, cond, finalExpr, stmt, _curScope);
    _leaveScope();

    return stmt;
}

IJsNode *JSParser::_expectSwitchStmt() {
    auto cond = _expectParenCondition();
    _enterScope();

    auto stmt = PoolNew(_pool, JsStmtSwitch)(_resPool, cond);
    JsSwitchBranch *curBranch = nullptr;

    _enterBreakContinueArea(false);

    _expectToken(TK_OPEN_BRACE);
    while (_curToken.type != TK_CLOSE_BRACE) {
        if (_curToken.type == TK_EOF) {
            _parseError("Unexpected token: %d", _curToken.type);
        }

        if (_curToken.type == TK_CASE) {
            _readToken();
            auto expr = _expectMultipleExpression();
            _expectToken(TK_COLON);

            curBranch = PoolNew(_pool, JsSwitchBranch)(_resPool, expr);
            stmt->push(curBranch);
        } else if (_curToken.type == TK_DEFAULT) {
            _readToken();
            _expectToken(TK_COLON);

            if (stmt->defBranch) {
                _parseError("More than one default clause in switch statement");
            }
            curBranch = stmt->defBranch = PoolNew(_pool, JsSwitchBranch)(_resPool, nullptr);
            stmt->push(curBranch);
        } else {
            curBranch->push(_expectStatment());
        }
    }
    _readToken(); // '}'

    _leaveBreakContinueArea();
    _leaveScope();
    return stmt;
}

IJsNode *JSParser::_expectTryStmt() {
    IJsNode *stmtTry = nullptr, *exprCatch = nullptr, *stmtCatch = nullptr, *stmtFinal = nullptr;
    Scope *scopeCatch = nullptr;

    stmtTry = _expectBlock();

    if (_curToken.type == TK_CATCH) {
        _readToken();

        _enterScope();
        if (_curToken.type == TK_OPEN_PAREN) {
            _readToken();

            scopeCatch = _curScope;

            // 声明异常变量
            exprCatch = _expectVariableDeclaration(TK_LET, true);
            exprCatch->setBeingAssigned();

            _expectToken(TK_CLOSE_PAREN);
        }
        stmtCatch = _expectBlock();
        _leaveScope();
    }

    if (_curToken.type == TK_FINALLY) {
        _readToken();

        stmtFinal = _expectBlock();
    }

    if (!stmtCatch && !stmtFinal) {
        _parseError("Missing catch or finally after try");
    }

    return PoolNew(_pool, JsStmtTry)(stmtTry, exprCatch, stmtCatch, scopeCatch, stmtFinal);
}

/**
 * 匹配参数列表，返回参数的个数.
 */
void JSParser::_expectArgumentsList(VecJsNodes &args) {
    _readToken();

    int count = 0;
    while (_curToken.type != TK_CLOSE_PAREN) {
        if (count != 0) {
            _expectToken(TK_COMMA);
            if (_curToken.type == TK_CLOSE_PAREN)
                break;
        }

        bool spreadArgs = _curToken.type == TK_ELLIPSIS;
        if (spreadArgs) {
            _readToken();
        }

        auto e = _expectExpression();
        if (spreadArgs) {
            // TODO: spread args 会影响 stack 的 count.
            e = PoolNew(_pool, JsNodeSpreadArgument)(e);
        }
        args.push_back(e);
        count++;
    }

    _expectToken(TK_CLOSE_PAREN);
}

/**
 * @initFromStackTop 用于表示，在生成代码时，是否需要从栈顶赋值给变量.
 *   var [a, b]; // initFromStackTop 为 false
 *   var [a, b] = []; // initFromStackTop 为 true
 */
JsNodeVarDeclarationList *JSParser::_expectVariableDeclarationList(TokenType declareType, bool initFromStackTop) {
    auto expr = PoolNew(_pool, JsNodeVarDeclarationList)(_resPool);

    while (true) {
        auto node = _expectVariableDeclaration(declareType, initFromStackTop);
        expr->push(node);
        if (_curToken.type != TK_COMMA) {
            break;
        }
        _readToken();
    }

    expr->setBeingAssigned();

    return expr;
}

/**
 * 解析变量声明
 */
IJsNode *JSParser::_expectVariableDeclaration(TokenType declareType, bool initFromStackTop) {
    IJsNode *left = nullptr, *right = nullptr;

    switch (_curToken.type) {
        case TK_NAME:
            if (declareType == TK_VAR) {
                _curFuncScope->addVarDeclaration(_curToken);
            } else {
                _curScope->addVarDeclaration(_curToken, declareType == TK_CONST);
            }
            left = _newExprIdentifier(_curToken);
            _readToken();
            break;
        case TK_OPEN_BRACKET: {
            _readToken();
            left = _expectArrayAssignable(declareType);
            break;
        }
        case TK_OPEN_BRACE: {
            _readToken();
            left = _expectObjectAssignable(declareType);
            _expectToken(TK_CLOSE_BRACE);
            break;
        }
        default:
            assert(0);
            break;
    }

    bool hasInitExpr = false;
    if (_curToken.type == TK_ASSIGN) {
        _readToken();

        hasInitExpr = true;
        right = _expectExpression(PRED_NONE, false);
    }

    if (declareType == TK_CONST && !(initFromStackTop || hasInitExpr)) {
        _parseError("Missing initializer in const declaration");
    }

    if (initFromStackTop) {
        return PoolNew(_pool, JsExprAssignWithStackTop)(left, right);
    } else {
        if (right != nullptr) {
            return PoolNew(_pool, JsExprAssign)(left, right, true);
        } else {
            if (left->type == NT_IDENTIFIER) {
                // JsExprIdentifier 并未被赋值，引用，需要从 _headIdRefs 中删除
                ((JsExprIdentifier *)left)->noAssignAndRef = true;

                assert(_headIdRefs == left);
                _headIdRefs = ((JsExprIdentifier *)left)->next;
            }
            return left;
        }
    }
}

IJsNode *JSParser::_expectArrayAssignable(TokenType declareType) {
    auto arr = PoolNew(_pool, JsExprArray)(_resPool);

    int index = 0;
    while (true) {
        if (_curToken.type == TK_COMMA) {
            // 可以有多个 ',' 连着
            _readToken();
            index++;
        }

        if (_curToken.type == TK_CLOSE_BRACKET) {
            // 提前结束
            _readToken();
            break;
        }

        // 声明的变量 initFromStackTop 一定为 true. 因为 ArrayAssignable 在语法上要求必须有初始化值.
        arr->push(_expectVariableDeclaration(declareType, true));

        index++;
    }

    return arr;
}

IJsNode *JSParser::_expectObjectAssignable(TokenType declareType) {
    return nullptr;
}

void JSParser::_expectToken(TokenType expected) {
    if (_curToken.type != expected) {
        _parseError("Unexpected token: %d, expected: %d.", _curToken.type, expected);
    } else {
        _readToken();
    }
}

void JSParser::_expectSemiColon() {
    if (_curToken.type == TK_SEMI_COLON) {
        _readToken();
    } else if (!_canInsertSemicolon()) {
        _parseError("SemiColon is expected, unexpected token: %d", _curToken.type);
    }
}

IJsNode *JSParser::_expectFunctionDeclaration() {
    auto parentScope = _curScope;
    auto child = _enterFunction(_curToken);

    _readToken();

    if (_curToken.type == TK_MUL) {
        _readToken();
        child->isGenerator = true;
    }

    if (_curToken.type != TK_NAME) {
        _expectToken(TK_NAME);
    }

    child->name = tokenToSizedString(_curToken);
    parentScope->addFunctionDeclaration(_curToken, child);
    _readToken();

    child->params = _expectFormalParameters();

    // function body
    _expectToken(TK_OPEN_BRACE);
    while (_curToken.type != TK_CLOSE_BRACE) {
        if (_curToken.type == TK_EOF) {
            _parseError("Unexpected end of input");
            break;
        }

        child->astNodes.push_back(_expectStatment());
    }
    _readToken();

    _leaveFunction();

    return child;
}

IJsNode *JSParser::_expectFunctionExpression(uint32_t functionFlags, bool ignoreFirstToken) {
    auto child = _enterFunction(_curToken);

    if (ignoreFirstToken) {
        _readToken();
    }

    if (functionFlags & FT_ASYNC) {
        child->isAsync = true;
    }

    if (functionFlags & FT_GENERATOR) {
        child->isGenerator = true;
    }

    if ((functionFlags & (FT_GETTER | FT_SETTER)) && _curToken.type == TK_MUL) {
        _readToken();
        assert(!child->isGenerator);
        child->isGenerator = true;
    }

    if ((functionFlags & FT_EXPRESSION) && _curToken.type == TK_NAME) {
        child->name = tokenToSizedString(_curToken);
        // TODO: 函数表达式的名字在函数的作用域内
        // child->addVarDeclaration(_curToken);
        _readToken();
    }

    if (functionFlags & FT_MEMBER_FUNCTION) {
        child->isMemberFunction = true;
    }

    child->params = _expectFormalParameters();
    auto count = child->params ? child->params->count() : 0;
    if ((functionFlags & FT_GETTER) && count > 0) {
        _parseError("Getter must not have any formal parameters.");
        return child;
    } else if ((functionFlags & FT_SETTER) && count != 1) {
        _parseError("Setter must have exactly one formal parameter.");
        return child;
    }

    // function body
    _expectToken(TK_OPEN_BRACE);
    while (_curToken.type != TK_CLOSE_BRACE) {
        if (_curToken.type == TK_EOF) {
            _parseError("Unexpected end of input");
            break;
        }

        child->astNodes.push_back(_expectStatment());
    }
    _readToken();

    _leaveFunction();

    return PoolNew(_pool, JsFunctionExpr)(child);
}

IJsNode *JSParser::_expectClassDeclaration(bool isClassExpr) {
    assert(0);
    return nullptr;
}

JsNodeParameters *JSParser::_expectFormalParameters() {
    _expectToken(TK_OPEN_PAREN);

    auto params = PoolNew(_pool, JsNodeParameters)(_resPool);
    int index = 0;
    while (_curToken.type != TK_CLOSE_PAREN) {
        if (index > 0)
            _expectToken(TK_COMMA);

        if (_curToken.type == TK_ELLIPSIS) {
            // ...id_name
            _readToken();
            _expectToken(TK_NAME);

            params->push(PoolNew(_pool, JsNodeRestParameter)(_newExprIdentifier(_curToken), index));

            _curFuncScope->addVarDeclaration(_curToken);
            _readToken();
            if (_curToken.type != TK_CLOSE_PAREN) {
                _parseError("SyntaxError: Rest parameter must be last formal parameter");
            }
            break;
        } else {
            params->push(_expectParameterDeclaration(index));
        }
        index++;
    }

    params->setBeingAssigned();

    _expectToken(TK_CLOSE_PAREN);
    return params;
}

IJsNode *JSParser::_expectParameterDeclaration(uint16_t index) {
    IJsNode *left = nullptr;

    switch (_curToken.type) {
        case TK_NAME:
            _curFuncScope->addArgumentDeclaration(_curToken, index);
            left = _newExprIdentifier(_curToken);
            _readToken();
            break;
        case TK_OPEN_BRACKET: {
            _readToken();
            left = _expectArrayAssignable(TK_VAR);
            break;
        }
        case TK_OPEN_BRACE: {
            _readToken();
            left = _expectObjectAssignable(TK_VAR);
            _expectToken(TK_CLOSE_BRACE);
            break;
        }
        default:
            assert(0);
            break;
    }

    if (_curToken.type == TK_ASSIGN) {
        _readToken();

        // 将表达式的指令保存起来
        auto defaultVal = _expectExpression();
        if (left->type != NT_IDENTIFIER) {
            auto expr = PoolNew(_pool, JsExprAssignWithStackTop)(left, defaultVal);
            return PoolNew(_pool, JsNodeAssignWithParameter)(expr, index);
        } else {
            return PoolNew(_pool, JsNodeUseDefaultParameter)(left, defaultVal, index);
        }
    } else {
        if (left->type == NT_IDENTIFIER) {
            // JsExprIdentifier 并未被赋值，引用，需要从 _headIdRefs 中删除
            assert(_headIdRefs == left);
            _headIdRefs = ((JsExprIdentifier *)left)->next;
            return left;
        } else {
            // array, object 需要扩展参数
            return PoolNew(_pool, JsNodeAssignWithParameter)(left, index);
        }
    }
}

IJsNode *JSParser::_expectExpression(Precedence pred, bool enableIn) {
    IJsNode *expr = nullptr;

    // 先匹配基本的表达式
    switch (_curToken.type) {
        case TK_NAME: {
            auto name = _curToken;
            _readToken();
            if (_curToken.type == TK_ARROW) {
                // Arrow function
                _readToken();

                auto childFunction = _enterFunction(name, false, true);
                _curFuncScope->addArgumentDeclaration(name, 0);

                if (_curToken.type == TK_OPEN_BRACE) {
                    childFunction->astNodes.push_back(_expectBlock());
                } else {
                    childFunction->astNodes.push_back(PoolNew(_pool, JsStmtReturnValue)(_expectExpression()));
                }
                _leaveFunction();

                expr = PoolNew(_pool, JsFunctionExpr)(childFunction);
            } else {
                expr = _newExprIdentifier(name);
            }
            break;
        }
        case TK_OPEN_BRACE:
            expr = _expectObjectLiteralExpression();
            break;
        case TK_OPEN_BRACKET:
            expr = _expectArrayLiteralExpression();
            break;
        case TK_OPEN_PAREN:
            expr = _expectParenExpression();
            if (_curToken.type == TK_ARROW) {
                expr = _expectArrowFunction(expr);
            }
            break;
        case TK_TEMPLATE_NO_SUBSTITUTION:
        case TK_STRING: {
            if (_curToken.len == 1) {
                expr = PoolNew(_pool, JsExprChar)(_curToken.buf[0]);
            } else {
                expr = PoolNew(_pool, JsExprString)(_getStringIndex(_curToken));
            }
            _readToken();
            break;
        }
        case TK_NUMBER: {
            int32_t n = (int32_t)_curToken.number;
            if (_curToken.number == n) {
                expr = PoolNew(_pool, JsExprInt32)(n);
            } else {
                expr = PoolNew(_pool, JsExprNumber)(_getDoubleIndex(_curToken.number));
            }
            _readToken();
            break;
        }
        case TK_TRUE: {
            expr = PoolNew(_pool, JsExprBoolTrue)();
            _readToken();
            break;
        }
        case TK_FALSE: {
            expr = PoolNew(_pool, JsExprBoolFalse)();
            _readToken();
            break;
        }
        case TK_NULL: {
            expr = PoolNew(_pool, JsExprNull)();
            _readToken();
            break;
        }
        case TK_REGEX: {
            expr = PoolNew(_pool, JsExprRegExp)(_getStringIndex(_curToken));
            _readToken();
            break;
        }
        case TK_FUNCTION:
            expr = _expectFunctionExpression(FT_EXPRESSION);
            break;
        case TK_CLASS:
            _expectClassDeclaration(true);
            break;
        case TK_NEW: {
            _readToken();
            if (_curToken.type == TK_DOT) {
                _readToken();
                if (isTokenNameEqual(_curToken, NAME_TARGET)) {
                    expr = PoolNew(_pool, JsExprNewTarget);
                } else {
                    _parseError("Unexpected token.");
                }
                break;
            }

            auto exprNew = PoolNew(_pool, JsExprNew)(_resPool, _expectExpression(PRED_LEFT_HAND_EXPR));
            if (_curToken.type == TK_OPEN_PAREN) {
                _expectArgumentsList(exprNew->args);
            }
            expr = exprNew;
            break;
        }
        case TK_DELETE: {
            _readToken();
            expr = PoolNew(_pool, JsExprDelete)(_expectExpression(PRED_UNARY_PREFIX));
            break;
        }
        case TK_VOID: {
            _readToken();
            expr = PoolNew(_pool, JsExprUnaryPrefix)(_expectExpression(PRED_UNARY_PREFIX), OP_VOID);
            break;
        }
        case TK_TYPEOF: {
            _readToken();
            expr = PoolNew(_pool, JsExprUnaryPrefix)(_expectExpression(PRED_UNARY_PREFIX), OP_TYPEOF);
            break;
        }
        case TK_UNARY_PREFIX: {
            // !, ~
            auto opcode = _curToken.opr;
            _readToken();
            expr = PoolNew(_pool, JsExprUnaryPrefix)(_expectExpression(PRED_UNARY_PREFIX), opcode);
            break;
        }
        case TK_POSTFIX: {
            // ++, --
            bool increase = _curToken.buf[0] == '+';
            _readToken();
            expr = PoolNew(_pool, JsExprPrefixXCrease)(_expectExpression(PRED_UNARY_PREFIX), increase);
            break;
        }
        case TK_ADD: {
            // +, -
            bool isNegative = _curToken.buf[0] == '-';
            _readToken();
            expr = _expectExpression(PRED_UNARY_PREFIX);
            if (isNegative) {
                expr = PoolNew(_pool, JsExprUnaryPrefix)(expr, OP_PREFIX_NEGATE);
            } else {
                expr = PoolNew(_pool, JsExprUnaryPrefix)(expr, OP_PREFIX_PLUS);
            }
            break;
        }
        case TK_TEMPLATE_HEAD:
            // templateLiteral(stream);
            break;
        default:
            _parseError("Unexpected token: %.*s", _curToken.len, _curToken.buf);
            break;
    }

    // 匹配 member dot, member index, function call
    bool tryNext = true;
    while (tryNext) {
        switch (_curToken.type) {
            case TK_OPEN_BRACKET: {
                // Member index expression
                _readToken();
                expr = PoolNew(_pool, JsExprMemberIndex)(expr, _expectMultipleExpression());
                _expectToken(TK_CLOSE_BRACKET);
                break;
            }
            case TK_OPTIONAL_DOT: {
               // Optional member dot expression
               _readToken();
               if (_curToken.type == TK_NAME || isKeyword(_curToken.type)) {
                   expr = PoolNew(_pool, JsExprMemberDot)(expr, _getStringIndex(_curToken), true);
                   _readToken();
               } else {
                   _expectToken(TK_NAME);
               }
               break;
           }
            case TK_DOT: {
                // Member dot expression
                _readToken();
                if (_curToken.type == TK_NAME || isKeyword(_curToken.type)) {
                    expr = PoolNew(_pool, JsExprMemberDot)(expr, _getStringIndex(_curToken));
                    _readToken();
                } else {
                    _expectToken(TK_NAME);
                }
                break;
            }
            case TK_OPEN_PAREN: {
                // Arguments expression
                if (pred >= PRED_POSTFIX) {
                    return expr;
                }

                if (expr->type == NT_IDENTIFIER && ((JsExprIdentifier *)expr)->name.equal(NAME_EVAL)) {
                    _curScope->setHasEval();
                }

                auto funcCall = PoolNew(_pool, JsExprFunctionCall)(_resPool, expr);
                _expectArgumentsList(funcCall->args);
                expr = funcCall;
                break;
            }
            case TK_TEMPLATE_NO_SUBSTITUTION: {
                if (pred >= PRED_POSTFIX) {
                    return expr;
                }

                VecInts indices;
                indices.push_back(_getStringIndex(_curToken));
                indices.push_back(_getStringIndex(_escapeString(tokenToSizedString(_curToken))));

                expr = PoolNew(_pool, JsExprTemplateFunctionCall)(_resPool, expr, _getRawStringsIndex(indices));
                break;
            }
            case TK_TEMPLATE_HEAD: {
                if (pred >= PRED_POSTFIX) {
                    return expr;
                }

                expr = _expectRawTemplateCall(expr);
                break;
            }
            default:
                tryNext = false;
                break;
        }
    }

    switch (_curToken.type) {
        case TK_POSTFIX: {
            // Postfix expression
            if (pred >= PRED_POSTFIX) {
                return expr;
            }

            expr = PoolNew(_pool, JsExprPostfix)(expr, _curToken.buf[0] == '+');
            _readToken();
            break;
        }
        case TK_ASSIGN: {
            if (pred > PRED_ASSIGNMENT) {
                return expr;
            }

            _readToken();
            expr = PoolNew(_pool, JsExprAssign)(expr, _expectExpression(PRED_ASSIGNMENT, enableIn));
            break;
        }
        case TK_ASSIGN_X: {
            if (pred > PRED_ASSIGNMENT) {
                return expr;
            }

            auto opr = _curToken.opr;
            _readToken();
            expr = PoolNew(_pool, JsExprAssignX)(expr, _expectExpression(PRED_ASSIGNMENT, enableIn), opr);
            break;
        }
        default:
            break;
    }

    while (true) {
        switch (_curToken.type) {
            case TK_CONDITIONAL: {
                if (pred >= PRED_ADD)
                    return expr;
                _readToken();
                auto exprTrue = _expectExpression();
                _expectToken(TK_COLON);
                auto exprFalse = _expectExpression();
                expr = PoolNew(_pool, JsExprConditional)(expr, exprTrue, exprFalse);
                break;
            }
            case TK_NULLISH: {
                if (pred >= PRED_NULLISH)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprNullish)(expr, _expectExpression(PRED_NULLISH));
                break;
            }
            case TK_LOGICAL_OR: {
                if (pred >= PRED_LOGICAL_OR)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprLogicalOr)(expr, _expectExpression(PRED_LOGICAL_OR));
                break;
            }
            case TK_LOGICAL_AND: {
                if (pred >= PRED_LOGICAL_AND)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprLogicalAnd)(expr, _expectExpression(PRED_LOGICAL_AND));
                break;
            }
            case TK_BIT_OR: {
                if (pred >= PRED_BIT_OR)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_BIT_OR), OP_BIT_OR);
                break;
            }
            case TK_BIT_XOR: {
                if (pred >= PRED_BIT_XOR)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_BIT_XOR), OP_BIT_XOR);
                break;
            }
            case TK_BIT_AND: {
                if (pred >= PRED_BIT_AND)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_BIT_AND), OP_BIT_AND);
                break;
            }
            case TK_EQUALITY: {
                if (pred >= PRED_EQUALITY)
                    return expr;
                auto opcode = _curToken.opr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_EQUALITY), opcode);
                break;
            }
            case TK_RATIONAL: {
                if (pred >= PRED_RATIONAL)
                    return expr;
                auto opcode = _curToken.opr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_RATIONAL), opcode);
                break;
            }
            case TK_IN:
                if (pred >= PRED_RATIONAL || !enableIn)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_RATIONAL), OP_IN);
                break;

            case TK_INSTANCEOF:
                if (pred >= PRED_RATIONAL || !enableIn)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_RATIONAL), OP_INSTANCE_OF);
                break;

            case TK_SHIFT: {
                if (pred >= PRED_SHIFT)
                    return expr;
                auto opcode = _curToken.opr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_SHIFT), opcode);
                break;
            }
            case TK_ADD: {
                if (pred >= PRED_ADD)
                    return expr;
                auto opcode = _curToken.opr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_ADD), opcode);
                break;
            }
            case TK_MUL: {
                if (pred >= PRED_MUL)
                    return expr;
                auto opcode = _curToken.opr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_MUL), opcode);
                break;
            }
            case TK_EXP: {
                if (pred >= PRED_EXP)
                    return expr;
                _readToken();
                expr = PoolNew(_pool, JsExprBinaryOp)(expr, _expectExpression(PRED_EXP), OP_EXP);
                break;
            }
            default:
                return expr;
        }
    }
}

IJsNode *JSParser::_expectMultipleExpression(bool enableIn) {
    auto e = _expectExpression(PRED_NONE, enableIn);
    if (_curToken.type != TK_COMMA) {
        return e;
    }

    auto expr = PoolNew(_pool, JsCommaExprs)(_resPool);
    expr->push(e);

    do {
        _readToken();
        expr->push(_expectExpression(PRED_NONE, enableIn));
    } while (_curToken.type == TK_COMMA);

    return expr;
}

IJsNode *JSParser::_expectParenCondition() {
    _expectToken(TK_OPEN_PAREN);

    auto expr = _expectMultipleExpression();

    _expectToken(TK_CLOSE_PAREN);

    return expr;
}

IJsNode *JSParser::_expectRawTemplateCall(IJsNode *func) {
    VecInts indices;

    indices.push_back(_getStringIndex(_curToken));
    indices.push_back(_getStringIndex(_escapeString(tokenToSizedString(_curToken))));

    auto rawStringIdx = _getRawStringsIndex(indices);
    auto expr = PoolNew(_pool, JsExprTemplateFunctionCall)(_resPool, func, rawStringIdx);

    while (true) {
        expr->args.push_back(_expectMultipleExpression());

        if (_curToken.type != TK_CLOSE_BRACE) {
            _expectToken(TK_CLOSE_BRACE);
        }

        _readInTemplateMid();

        indices.push_back(_getStringIndex(_curToken));
        indices.push_back(_getStringIndex(_escapeString(tokenToSizedString(_curToken))));

        if (_curToken.type == TK_TEMPLATE_TAIL) {
            _readToken();
            break;
        }

        _expectToken(TK_TEMPLATE_MIDDLE);
    }

    _v2Ints[rawStringIdx] = indices;

    return expr;
}

IJsNode *JSParser::_expectArrowFunction(IJsNode *parenExpr) {
    assert(0);
    return nullptr;
}

IJsNode *JSParser::_expectParenExpression() {
    _expectToken(TK_OPEN_PAREN);

    auto parenExpr = PoolNew(_pool, JsParenExpr)(_resPool);

    parenExpr->push(_expectExpression());

    while (_curToken.type != TK_CLOSE_PAREN) {
        _expectToken(TK_COMMA);
        parenExpr->push(_expectExpression());
    }
    _expectToken(TK_CLOSE_PAREN);

    return parenExpr;
}

IJsNode *JSParser::_expectObjectLiteralExpression() {
    _expectToken(TK_OPEN_BRACE);
    bool first = true;

    auto obj = PoolNew(_pool, JsExprObject)(_resPool);

    while (_curToken.type != TK_CLOSE_BRACE) {
        if (first) {
            first = false;
        } else {
            _expectToken(TK_COMMA);
        }
        if (_curToken.type == TK_CLOSE_BRACE) {
            break;
        }

        auto type = _curToken.type;
        auto name = _curToken;
        _peekToken();

        if (isTokenNameEqual(_curToken, NAME_ASYNC)) {
            // async function
            _readToken();
            auto nameIdx = _getStringIndex(_curToken);
            auto expr = _expectFunctionExpression(FT_MEMBER_FUNCTION | FT_ASYNC);

            obj->push(PoolNew(_pool, JsObjectProperty)(nameIdx, expr));
        } else if (type == TK_NAME && (_nextToken.type != TK_COLON && _nextToken.type != TK_OPEN_PAREN) && (name == "get" || name == "set")) {
            // getter, setter
            if (!canTokenBeMemberName(_nextToken.type)) {
                _parseError("Unexpected token: %.*s", _curToken.len, _curToken.buf);
                return nullptr;
            }

            auto nameIdx = _getStringIndex(_nextToken);
            _readToken();

            if (name == "get") {
                auto expr = _expectFunctionExpression(FT_GETTER);
                obj->push(PoolNew(_pool, JsObjectPropertyGetter)(nameIdx, expr));
            } else {
                auto expr = _expectFunctionExpression(FT_SETTER);
                obj->push(PoolNew(_pool, JsObjectPropertySetter)(nameIdx, expr));
            }
        } else if (type == TK_NAME && (_nextToken.type == TK_COMMA || _nextToken.type == TK_CLOSE_BRACE)) {
            // name,
            auto nameIdx = _getStringIndex(_curToken);
            auto expr = _newExprIdentifier(_curToken);
            _readToken();

            if (_curToken.type == TK_ASSIGN) {
                // { x=y } 这种形式
                auto value = _expectExpression();
                obj->push(PoolNew(_pool, JsObjectPropertyShortInit)(nameIdx, expr, value));
            } else {
                obj->push(PoolNew(_pool, JsObjectProperty)(nameIdx, expr));
            }

            _readToken();
        } else if (type == TK_ELLIPSIS) {
            // ...expr
            _readToken();
            auto expr = _expectExpression();
            obj->push(PoolNew(_pool, JsObjectPropertySpread)(expr));
        } else if (type == TK_MUL) {
            // * generator function
            _readToken();
            if (_curToken.type == TK_OPEN_BRACKET) {
                // [ expr ]
                _readToken();
                auto name = _expectMultipleExpression();
                _expectToken(TK_CLOSE_BRACKET);
                auto value = _expectFunctionExpression(FT_MEMBER_FUNCTION | FT_GENERATOR);
                obj->push(PoolNew(_pool, JsObjectPropertyComputed)(name, value));
            } else {
                auto nameIdx = _getStringIndex(_curToken);
                auto expr = _expectFunctionExpression(FT_MEMBER_FUNCTION | FT_GENERATOR);
                obj->push(PoolNew(_pool, JsObjectProperty)(nameIdx, expr));
            }
        } else {
            IJsNode *name = nullptr, *value = nullptr;
            int nameIdx = -1;
            if (type == TK_OPEN_BRACKET) {
                // [ expr ]
                _readToken();
                name = _expectMultipleExpression();
                _expectToken(TK_CLOSE_BRACKET);
            } else {
                if (!canTokenBeMemberName(type)) {
                    _parseError("Unexpected token: %.*s", _curToken.len, _curToken.buf);
                    return nullptr;
                }
                nameIdx = _getStringIndex(_curToken);
                _readToken();
            }

            if (_curToken.type == TK_OPEN_PAREN) {
                // 函数
                value = _expectFunctionExpression(FT_MEMBER_FUNCTION, false);
            } else {
                _expectToken(TK_COLON);
                value = _expectExpression();
            }

            if (name) {
                obj->push(PoolNew(_pool, JsObjectPropertyComputed)(name, value));
            } else {
                obj->push(PoolNew(_pool, JsObjectProperty)(nameIdx, value));
            }
        }
    }

    _expectToken(TK_CLOSE_BRACE);

    _checkingExprObjs.push_back(obj);

    return obj;
}

IJsNode *JSParser::_expectArrayLiteralExpression() {
    _expectToken(TK_OPEN_BRACKET);

    bool first = true;
    auto *arr = PoolNew(_pool, JsExprArray)(_resPool);
    int index = 0;

    while (_curToken.type != TK_CLOSE_BRACKET) {
        if (first) {
            first = false;
        } else {
            _expectToken(TK_COMMA);
        }
        if (_curToken.type == TK_CLOSE_BRACKET) {
            break;
        }

        auto type = _curToken.type;
        if (type == TK_ELLIPSIS) {
            _readToken();
            arr->push(PoolNew(_pool, JsExprArrayItemSpread)(_expectExpression()));
        } else if (type == TK_COMMA) {
            _readToken();
            arr->push(PoolNew(_pool, JsExprArrayItemEmpty));
        } else {
            arr->push(PoolNew(_pool, JsExprArrayItem)(_expectExpression()));
        }
        index++;
    }

    _expectToken(TK_CLOSE_BRACKET);

    return arr;
}

JsExprIdentifier *JSParser::_newExprIdentifier(const Token &token) {
    auto ret = PoolNew(_resPool->pool, JsExprIdentifier)(token, _curScope);
    ret->isUsedNotAsFunctionCall = true;
    ret->next = _headIdRefs;
    _headIdRefs = ret;
    return ret;
}

void _reduceScopeLevels(Scope *scope, Scope *validParent) {
    scope->parent = validParent;
    scope->depth = validParent->depth + 1;

    if (scope->sibling) {
        _reduceScopeLevels(scope->sibling, validParent);
    }

    bool keepScope = !scope->varDeclares.empty() || scope->isFunctionScope || scope->hasWith || scope->hasEval;
    if (keepScope) {
        // 重新添加到 parent
        scope->sibling = validParent->child;
        validParent->child = scope;
    }

    if (scope->child) {
        if (keepScope) {
            validParent = scope;
        }

        auto child = scope->child;
        scope->child = nullptr;
        _reduceScopeLevels(child, validParent);
    }
}

/**
 * 精简 scope 的层次(去掉没有变量声明的 scope)，加快访问速度
 */
void JSParser::_reduceScopeLevels(Function *function) {
    if (function->scope->child) {
        auto child = function->scope->child;
        function->scope->child = nullptr;
        ::_reduceScopeLevels(child, function->scope);
    }
}

void JSParser::_relocateIdentifierInParentFunction(Function *codeBlock, Function *parent) {
    // codeBlock 中的变量和子函数并非真的在 function 函数内，
    // 而是在父函数中，所以依照父函数的地址分配存储空间

    Token token;
    memset(&token, 0, sizeof(token));

    VarStorageType storageType = parent->scope->parent ? VST_FUNCTION_VAR : VST_GLOBAL_VAR;

    // 分配变量地址
    auto functionScope = parent->scope;
    for (auto &item : codeBlock->scope->varDeclares) {
        token.buf = item.first.data;
        token.len = item.first.len;

        auto curId = item.second;
        if (!curId->isScopeVar) {
            // var 变量，分配变量地址
            auto id = functionScope->addVarDeclaration(token);
            if (id->varStorageType == VST_NOT_SET) {
                id->varStorageType = storageType;
                id->storageIndex = functionScope->countLocalVars++;
            } else if (curId->isFuncName && functionScope->parent == nullptr
                       && id->storageIndex < _runtimeCommon->countImmutableGlobalVars) {
                // 不能出现同名的函数
                _parseError("Identifier '%.*s' has already been declared",
                            (int)item.first.len, item.first.data);
            }

            curId->varStorageType = id->varStorageType;
            curId->storageIndex = id->storageIndex;
            curId->scope = id->scope;
        }
    }

    // 分配函数地址
    for (auto f : codeBlock->scope->functionDecls) {
        assert(f->declare->varStorageType == VST_FUNCTION_VAR || f->declare->varStorageType == VST_GLOBAL_VAR);
    }
}

void JSParser::_buildExprIdentifiers() {
    for (auto p = _headIdRefs; p; p = p->next) {
        p->scope->addVarReference(p);
        if (p->nameStringIdx == 0) {
            p->nameStringIdx = _getStringIndex(p->name);
        }
    }
}

void JSParser::_allocateIdentifierStorage(Scope *scope, int registerIndex) {
    if (scope->sibling) {
        _allocateIdentifierStorage(scope->sibling, registerIndex);
    }

    auto functionScope = scope->function->scope;

    for (auto &item : scope->varDeclares) {
        auto declare = item.second;
        if (declare->varStorageType != VST_NOT_SET) {
            if (declare->isReferred) {
                if (declare->name.equal(SS_THIS)) {
                    declare->scope->isThisUsed = true;
                } else if (declare->name.equal(SS_ARGUMENTS)) {
                    declare->scope->isArgumentsUsed = true;
                }
            } else if (declare->varStorageType == VST_ARGUMENT && declare->isFuncName) {

            }
            continue;
        }

        if (declare->isFuncName) {
            if (declare->isUsedNotAsFunctionCall && declare->varStorageType != VST_FUNCTION_VAR) {
                // 函数被引用到了（如果仅仅是函数调用，isUsedNotAsFunctionCall 不会为 true），需要分配变量地址
                declare->varStorageType = VST_FUNCTION_VAR;
                declare->storageIndex = functionScope->countLocalVars++;
            }
        } else {
            // 变量
            if (true || declare->isReferredByChild || scope->hasEval || scope->hasWith) {
                // 被子函数引用 或者 有 eval, with，则必须将变量保存到 vars 中
                if (scope->parent == nullptr) {
                    declare->varStorageType = VST_GLOBAL_VAR;
                } else {
                    declare->varStorageType = VST_SCOPE_VAR;
                    declare->storageIndex = scope->countLocalVars++;
                }
            } else {
                // 可直接存储在寄存器中
                declare->varStorageType = VST_REGISTER;
                declare->storageIndex = registerIndex++;
            }
        }
    }

    if (scope->isAllocateFunctionVar()) {
        // 需要给函数分配变量地址
        for (auto f : scope->functionDecls) {
            if (f->declare->varStorageType == VST_NOT_SET) {
                f->declare->varStorageType = VST_FUNCTION_VAR;
                f->declare->storageIndex = functionScope->countLocalVars++;
            }
        }
    } else {
        // 仅仅保留有地址的函数，用于执行时的初始化
        VecFunctions funcVars;
        for (auto f : scope->functionDecls) {
            if (f->declare->varStorageType != VST_NOT_SET) {
                funcVars.push_back(f);
            }
        }
        scope->functionDecls = funcVars;
    }

    // 为子 scope 分配
    if (scope->child) {
        _allocateIdentifierStorage(scope->child, registerIndex);
    }
}

int JSParser::_getDoubleIndex(double value) {
    if (_runtimeCommon) {
        auto idx = _runtimeCommon->findDoubleValue(value);
        if (idx != -1) {
            return idx;
        }
    }

    int idx;
    auto it = _doubles.find(value);
    if (it == _doubles.end()) {
        idx = _nextDoubleIdx++;
        _doubles[value] = idx;

        _resPool->doubles.push_back(value);
    } else {
        idx = (*it).second;
    }

    return idx;
}

int JSParser::_getStringIndex(const SizedString &str) {
    if (_runtimeCommon) {
        auto idx = _runtimeCommon->findStringValue(str);
        if (idx != -1) {
            return idx;
        }
    }

    int idx;
    auto it = _strings.find(str);
    if (it == _strings.end()) {
        idx = _nextStringIdx++;
        _strings[str] = idx;

        _resPool->strings.push_back(str);
    } else {
        idx = (*it).second;
    }

    return idx;
}

int JSParser::_getRawStringsIndex(const VecInts &indices) {
    _v2Ints.push_back(indices);
    return (int)_v2Ints.size() - 1;
}

Function *JSParser::_enterFunction(const Token &tokenStart, bool isCodeBlock, bool isArrowFunction) {
    auto child = PoolNew(_resPool->pool, Function)(_resPool, _curScope, (int16_t)_curFunction->functions.size(), isCodeBlock, isArrowFunction);

    child->line = tokenStart.line;
    child->col = tokenStart.col;
    child->srcCode.data = tokenStart.buf;

    if (!isArrowFunction && !isCodeBlock) {
        // Codeblock, Arrow Function 的 this, arguments 使用的是父函数的.
        // 添加 this, arguments 的声明
        Token name = tokenStart;
        name.buf = (uint8_t *)SS_THIS.data; name.len = SS_THIS.len;
        child->scope->addVarDeclaration(name, false, true);

        name.buf = (uint8_t *)SS_ARGUMENTS.data; name.len = SS_ARGUMENTS.len;
        child->scope->addVarDeclaration(name, false, true);
    }

    _curScope->functions.push_back(child);
    _curFunction->functions.push_back(child);
    assert(_curFunction->functions.size() <= 0xFFFF);

    _stackFunctions.push_back(_curFunction);
    _curFunction = child;

    _stackScopes.push_back(_curScope);
    _stackScopes.push_back(_curFuncScope);
    _curScope = _curFuncScope = _curFunction->scope;

    return child;
}

void JSParser::_leaveFunction() {
    _curFunction->srcCode.len = uint32_t(_prevTokenEndPos - _curFunction->srcCode.data);

    _curFunction = _stackFunctions.back(); _stackFunctions.pop_back();

    _curFuncScope = _stackScopes.back(); _stackScopes.pop_back();
    _curScope = _stackScopes.back(); _stackScopes.pop_back();
}

void JSParser::_enterScope() {
    _stackScopes.push_back(_curScope);
    _curScope = PoolNew(_resPool->pool, Scope)(_resPool, _curFunction, _curScope);
    // writer.writeEnterScope(_curScope);
}

void JSParser::_leaveScope() {
    // writer.writeLeaveScope(_curScope);
    _curScope = _stackScopes.back(); _stackScopes.pop_back();
}

void JSParser::_checkExpressionObjects() {
    if (!_checkingExprObjs.empty()) {
        for (auto item : _checkingExprObjs) {
            item->checkCanBeExpression();
        }

        // 检查过的都删除
        _checkingExprObjs.clear();
    }
}

void JSParser::_enterBreakContinueArea(bool allowContinue) {
    _stackBreakContinueAreas.push_back(allowContinue);
}

void JSParser::_leaveBreakContinueArea() {
    _stackBreakContinueAreas.pop_back();
}
