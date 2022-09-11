//
//  Statements.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/14.
//

#include "Statement.hpp"
#include "Parser.hpp"


void JsStmtForIn::convertToByteCode(ByteCodeStream &stream) {
    writeEnterScope(scope, stream);

    obj->convertToByteCode(stream);
    stream.writeOpCode(isIn ? OP_ITERATOR_IN_CREATE : OP_ITERATOR_OF_CREATE);

    auto addrLoopStart = stream.address();
    stream.writeOpCode(isIn ? OP_ITERATOR_NEXT_KEY : OP_ITERATOR_NEXT_VALUE);
    auto addrLoopEnd = stream.writeReservedAddress();

    // var 需要转换为从堆栈上的赋值
    var->convertAssignableToByteCode(nullptr, stream);

    stream.writeOpCode(OP_POP_STACK_TOP);

    if (stmt) {
        stream.enterBreakContinueArea();
        stmt->convertToByteCode(stream);
        stream.leaveBreakContinueArea();
    }

    stream.writeOpCode(OP_JUMP);
    stream.writeAddress(addrLoopStart);
    *addrLoopEnd = stream.address();

    writeLeaveScope(scope, stream);
}

void JsStmtSwitch::convertToByteCode(ByteCodeStream &stream) {
    bool isAllConsts = true;
    for (auto branch : branches) {
        if (branch->exprCase && branch->exprCase->type > _NT_CONST_EXPR_END) {
            isAllConsts = false;
            break;
        }
    }

    // 将条件压栈
    cond->convertToByteCode(stream);

    stream.enterBreakContinueArea(false);

    auto countCases = (uint32_t)branches.size() - (defBranch ? 1 : 0);

    if (isAllConsts && countCases >= 3) {
        // 可以优化为二分查找
        SwitchJump switchJump;

        switchJump.stmtSwitch = this;
        switchJump.caseJumps = (CaseJump *)resPool->pool.allocate(sizeof(CaseJump) * countCases);
        switchJump.caseJumpsEnd = switchJump.caseJumps + countCases;

        stream.writeOpCode(OP_SWITCH_CASE_FAST_CMP_JUMP);
        stream.writeUInt16((uint16_t)resPool->switchCaseJumps.size());

        auto pcj = switchJump.caseJumps;

        for (auto branch : branches) {
            if (branch->exprCase) {
                pcj->caseConds = jsValueUndefined;
                pcj->addr = stream.address();
                pcj++;
            } else {
                switchJump.defaultAddr = stream.address();
            }
            branch->convertToByteCode(stream);
        }

        assert(resPool->switchCaseJumps.size() < 0xFFFF);
        resPool->switchCaseJumps.push_back(switchJump);
    } else {
        // 只能逐个比较
        for (auto branch : branches) {
            // 生成分支条件比较代码
            if (branch->exprCase) {
                // 是 case 分支
                branch->exprCase->convertToByteCode(stream);
                stream.writeOpCode(OP_SWITCH_CASE_CMP_JUMP);
                branch->addrBranch = stream.writeReservedAddress();
            }
        }

        // 压栈的 switchCond 需要弹出来
        stream.writeOpCode(OP_POP_STACK_TOP);

        VMAddress *endAddr = nullptr;
        if (defBranch) {
            // 如果一个都不满足，则跳到 defBranch
            stream.writeOpCode(OP_JUMP);
            assert(defBranch->addrBranch == nullptr);
            defBranch->addrBranch = stream.writeReservedAddress();
        } else {
            // 跳转到结束
            stream.writeOpCode(OP_JUMP);
            endAddr = stream.writeReservedAddress();
        }

        for (auto branch : branches) {
            // 生成 case branch 的代码.
            *branch->addrBranch = stream.address();
            branch->convertToByteCode(stream);
            assert(branch->addrBranch != nullptr);
        }

        if (endAddr) {
            *endAddr = stream.address();
        }
    }

    stream.leaveBreakContinueArea();
}

void JsStmtSwitch::buildCaseJumps(VMRuntime *runtime, uint16_t poolIndex, SwitchJump *switchJump) {
    auto pcj = switchJump->caseJumps;

    for (auto branch : branches) {
        if (branch->exprCase) {
            pcj->caseConds = convertConstExprToJsValue(runtime, poolIndex, branch->exprCase);
            pcj++;
        }
    }
}
