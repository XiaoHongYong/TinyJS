//
//  ByteCodeStream.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/18.
//

#include "ByteCodeStream.hpp"
#include "Lexer.hpp"


void ByteCodeStream::writeBreakAddress() {
    _stackBreakContineAreas.back().breakAddrs.push_back(writeReservedAddress());
}

void ByteCodeStream::writeContinueAddress() {
    auto &bca = _stackBreakContineAreas.back();
    if (!bca.allowContinue) {
        // 应该已经在语法阶段就报告了此错误.
        assert(0);
    }

    writeAddress(bca.continueAddr);
}

void ByteCodeStream::enterBreakContinueArea(bool allowContinue) {
    _stackBreakContineAreas.push_back(BreakContinueArea(address(), allowContinue));
}

void ByteCodeStream::leaveBreakContinueArea() {
    auto addr = address();

    for (auto p : _stackBreakContineAreas.back().breakAddrs) {
        *p = addr;
    }

    _stackBreakContineAreas.pop_back();
}
