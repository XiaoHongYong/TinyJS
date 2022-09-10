//
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
    IJsNode *_expectBreakContinue(TokenType type);
    IJsNode *_expectForStatment();
    IJsNode *_tryForInStatment();
    IJsNode *_expectSwitchStmt();
    IJsNode *_expectTryStmt();
    void _expectArgumentsList(VecJsNodes &args);
    JsNodeVarDeclarationList *_expectVariableDeclarationList(TokenType declareType, bool initFromStackTop = false);
    IJsNode *_expectVariableDeclaration(TokenType declareType, bool initFromStackTop = false);
    IJsNode *_expectArrayAssignable(TokenType declareType);
    IJsNode *_expectObjectAssignable(TokenType declareType);

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
    IJsNode *_expectArrowFunction(IJsNode *parenExpr);

    JsExprIdentifier *_newExprIdentifier(const Token &token);

    void _expectToken(TokenType expected);
    void _expectSemiColon();

    bool _canInsertSemicolon() {
        // !isStrict
        return _curToken.newLineBefore || _curToken.type == TK_EOF || _curToken.type == TK_CLOSE_BRACE;
    }

    void _reduceScopeLevels(Function *function);
    void _relocateIdentifierInParentFunction(Function *codeBlock, Function *parent);
    void _buildExprIdentifiers();
    void _allocateIdentifierStorage(Scope *scope, int registerIndex);

    int _getStringIndex(const SizedString &str);
    int _getDoubleIndex(double value);
    int _getStringIndex(const Token &token) { return _getStringIndex(SizedString(token.buf, token.len)); }

    int _getRawStringsIndex(const VecInts &indices);

    Function *_enterFunction(const Token &tokenStart, bool isCodeBlock = false, bool isArrowFunction = false);
    void _leaveFunction();
    void _enterScope();
    void _leaveScope();

    // 检查 {x=y} 这样的不合乎语法的表达式
    void checkExpressionObjects();

protected:
    using MapStringToIdx = std::unordered_map<SizedString, int, SizedStringHash, SizedStrCmpEqual>;
    using MapDoubleToIdx = std::unordered_map<double, int>;

    VMRuntimeCommon             *_runtimeCommon;
    uint32_t                    _nextStringIdx, _nextDoubleIdx;

    // 保存了被引用到的 Identifier 实例，声明的不能添加到此列表中
    JsExprIdentifier            *_headIdRefs;

    vector<JsExprObject *>        _checkingExprObjs;

    MapStringToIdx              _strings;
    MapDoubleToIdx              _doubles;
    std::vector<VecInts>        _v2Ints;

    VecScopes                   _stackScopes;
    VecFunctions                _stackFunctions;

    Function                    *_curFunction;
    Scope                       *_curFuncScope;
    Scope                       *_curScope;

};

#endif /* Parser_hpp */
