﻿//
//  Parser.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#ifndef Parser_hpp
#define Parser_hpp


#include <functional>
#include <unordered_map>
#include "Expression.hpp"


class VMRuntimeCommon;
class JsNodeVarDeclarationList;

enum Precedence {
    PRED_NONE,
    PRED_ASSIGNMENT,
    PRED_CONDITIONAL,
    PRED_NULLISH,
    PRED_LOGICAL_OR,
    PRED_LOGICAL_AND,
    PRED_BIT_OR,
    PRED_BIT_XOR,
    PRED_BIT_AND,
    PRED_EQUALITY,
    PRED_RATIONAL,
    PRED_SHIFT,
    PRED_ADD,
    PRED_MUL,
    PRED_EXP,
    PRED_UNARY_PREFIX,
    PRED_POSTFIX,
    PRED_LEFT_HAND_EXPR,
};


/**
 * JSParser 负责解析 JavaScript 代码为线性语法树结构.
 *
 * JsParser 是一个临时对象，只在语法分析阶段有效。
 * 需要运行时使用的对象放在 Function 对象中，或者 ResourcePool 中。
 */
class JSParser : public JSLexer {
public:
    JSParser(VMRuntimeCommon *rtc, ResourcePool *resPool, const char *buf, size_t len);

    Function *parse(Scope *parent, bool isExpr);

protected:
    enum FunctionFlags : uint32_t {
        FT_EXPRESSION               = 1,
        FT_MEMBER_FUNCTION          = 1 << 1,
        FT_GETTER                   = 1 << 2,
        FT_SETTER                   = 1 << 3,
        FT_ASYNC                    = 1 << 4,
        FT_GENERATOR                = 1 << 5,
    };

    //
    // 语法解析相关的内部 API
    //
    IJsNode *_expectStatment();
    IJsNode *_expectExpressionStmt();
    IJsNode *_expectLabelStmt();
    IJsNode *_expectBlock();
    IJsNode *_expectBreak();
    IJsNode *_expectContinue();
    IJsNode *_expectForStatment();
    IJsNode *_tryForInStatment();
    IJsNode *_expectSwitchStmt();
    IJsNode *_expectTryStmt();
    void _expectArgumentsList(VecJsNodes &args);
    JsNodeVarDeclarationList *_expectVariableDeclarationList(JsTokenType declareType, bool initFromStackTop = false);
    IJsNode *_expectVariableDeclaration(JsTokenType declareType, bool initFromStackTop = false);
    IJsNode *_expectArrayAssignable(JsTokenType declareType);
    IJsNode *_expectObjectAssignable(JsTokenType declareType);

    IJsNode *_expectFunctionDeclaration();
    IJsNode *_expectFunctionExpression(uint32_t functionFlags, bool ignoreFirstToken = true);
    IJsNode *_expectClassDeclaration(bool isClassExpr = false);
    JsNodeParameters *_expectFormalParameters();
    IJsNode *_expectParameterDeclaration(uint16_t index);
    IJsNode *_expectExpression(Precedence pred = PRED_NONE, bool enableIn = true);
    IJsNode *_expectMultipleExpression(bool enableIn = true);
    IJsNode *_expectObjectLiteralExpression();
    IJsNode *_expectArrayLiteralExpression();
    IJsNode *_expectParenExpression();
    IJsNode *_expectParenCondition();
    IJsNode *_expectRawTemplateCall(IJsNode *func);
    JsNodeParameters *_convertParenExprsToFormalPrameters(JsParenExpr *exprParen);
    IJsNode *_convertExprToVariableDeclaration(IJsNode *expr);
    void _convertToArrayAssignable(JsExprArray *exprArr);
    void _convertToObjectAssignable(JsExprObject *exprObject);

    JsExprIdentifier *_newExprIdentifier(const Token &token);

    void _expectToken(JsTokenType expected);
    void _expectSemiColon();
    void _unexpectToken();

    bool _canInsertSemicolon() {
        // !isStrict
        return _curToken.newLineBefore || _curToken.type == TK_EOF || _curToken.type == TK_CLOSE_BRACE;
    }

    void _reduceScopeLevels(Function *function);
    void _relocateIdentifierInParentFunction(Function *codeBlock, Function *parent);
    void _buildExprIdentifiers();
    void _allocateIdentifierStorage(Scope *scope, int registerIndex);

    int _getStringIndex(const StringView &str);
    int _getDoubleIndex(double value);
    int _getStringIndex(const Token &token) { return _getStringIndex(StringView(token.buf, token.len)); }

    int _getRawStringsIndex(const VecInts &indices);

    Function *_enterFunction(const Token &tokenStart, bool isCodeBlock = false, bool isArrowFunction = false);
    void _leaveFunction();
    void _enterScope();
    void _leaveScope();

    // 检查 {x=y} 这样的不合乎语法的表达式
    void _checkExpressionObjects();

    void _enterBreakContinueArea(bool allowContinue = true);
    void _leaveBreakContinueArea();

protected:
    using MapStringToIdx = std::unordered_map<StringView, int, StringViewHash, SizedStrCmpEqual>;
    using MapDoubleToIdx = std::unordered_map<double, int>;

    VMRuntimeCommon             *_runtimeCommon;
    uint32_t                    _nextStringIdx, _nextDoubleIdx;

    // 保存了被引用到的 Identifier 实例，声明的不能添加到此列表中
    JsExprIdentifier            *_headIdRefs;

    std::vector<JsExprObject *> _checkingExprObjs;

    MapStringToIdx              _strings;
    MapDoubleToIdx              _doubles;
    std::vector<VecInts>        _v2Ints;

    VecScopes                   _stackScopes;
    VecFunctions                _stackFunctions;

    Function                    *_curFunction;
    Scope                       *_curFuncScope;
    Scope                       *_curScope;

    std::list<bool>             _stackBreakContinueAreas;

};

#endif /* Parser_hpp */
