//
//  Parser.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#ifndef Parser_hpp
#define Parser_hpp


#include "AST.hpp"
#include <functional>
#include <unordered_map>
#include "InstructionOutputStream.hpp"


class VMRuntimeCommon;

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


class VarInitTree {
public:
    using Callback = std::function<bool (Function *function, bool)>;

    VarInitTree() { needPopStackTop = false; }
    ~VarInitTree();

    VarInitTree *newChild();
    void setCallback(const Callback &f) { callback = f; }
    void setNeedPopStackTop() { needPopStackTop = true; }

    void generateInitCode(Function *function, bool initExpr);

protected:
    // 展开Array 和 Object 之后，需要弹出栈顶的数据
    bool                            needPopStackTop;

    Callback                        callback;
    list<VarInitTree *>             children;

};

/**
 * JSParser 负责解析 JavaScript 代码为线性语法树结构.
 */
class JSParser : public JSLexer {
public:
    JSParser(VMRuntimeCommon *rtc, ResourcePool *resPool, const char *buf, size_t len);

    Function *parse(Scope *parent, bool isExpr);

protected:
    enum FunctionType {
        FT_DECLARATION,
        FT_EXPRESSION,
        FT_MEMBER_FUNCTION,
        FT_GETTER,
        FT_SETTER,
    };

    //
    // 语法解析相关的内部 API
    //
    void _expectStatment();
    void _expectExpressionStmt();
    void _expectLabelStmt();
    void _expectBlock();
    void _expectBreakContinue(TokenType type);
    void _expectForStatment();
    void _expectSwitchStmt();
    void _expectTryStmt();
    int _expectArgumentsList(InstructionOutputStream &instructions);
    void _expectVariableDeclarationList(TokenType declareType);
    void _expectVariableDeclaration(TokenType declareType, VarInitTree *parentVarInitTree);
    void _expectArrayAssignable(TokenType declareType, VarInitTree *parentVarInitTree);
    void _expectObjectAssignable(TokenType declareType, VarInitTree *parentVarInitTree);

    void _assignParameter(Function *function, InstructionOutputStream &instructions, int index);
    void _assignIdentifier(Function *function, InstructionOutputStream &instructions, const Token &name);

    Function *_expectFunction(InstructionOutputStream &instructions, FunctionType type, bool ignoreFirstToken = true);
    int _expectFormalParameters();
    void _expectParameterDeclaration(uint16_t index, VarInitTree *parentVarInitTree);
    void _expectExpression(InstructionOutputStream &instructions, Precedence pred = PRED_NONE, bool enableComma = false, bool enableIn = true);
    void _expectParenCondition();
    int _expectRawTemplateCall(InstructionOutputStream &instructions, VecInts &indices);
    void _expectArrowFunction(InstructionOutputStream &instructions);
    void _expectParenExpression(InstructionOutputStream &instructions);
    void _expectObjectLiteralExpression(InstructionOutputStream &instructions);

    IdentifierRef *_newIdentifierRef(const Token &token, Function *function);

    void _expectToken(TokenType expected);
    void _expectSemiColon();

    bool _canInsertSemicolon() {
        // !isStrict
        return _curToken.newLineBefore || _curToken.type == TK_EOF || _curToken.type == TK_CLOSE_BRACE;
    }

    bool _isArrowFunction();
    void _ignoreExpression();

    void _reduceScopeLevels(Function *function);
    void _buildIdentifierRefs();
    void _allocateIdentifierStorage(Scope *scope, int registerIndex);

    int _getStringIndex(const SizedString &str);
    int _getDoubleIndex(double value);
    int _getStringIndex(const Token &token) { return _getStringIndex(SizedString(token.buf, token.len)); }

    int _getRawStringsIndex(const VecInts &indices);

    Function *_enterFunction(const Token &tokenStart, bool isArrowFunction = false);
    void _leaveFunction();
    void _enterScope();
    void _leaveScope();

protected:
    using MapStringToIdx = std::unordered_map<SizedString, int, SizedStringHash, SizedStrCmpEqual>;
    using MapDoubleToIdx = std::unordered_map<double, int>;

    VMRuntimeCommon             *_runtimeCommon;
    uint32_t                    _nextStringIdx, _nextDoubleIdx;

    IdentifierRef               *_headIdRefs;
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
