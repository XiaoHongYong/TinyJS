//
//  ByteCodeStream.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/18.
//

#ifndef ByteCodeStream_hpp
#define ByteCodeStream_hpp

#include "utils/BinaryStream.h"
#include "interpreter/VirtualMachineTypes.hpp"


class ByteCodeStream : public BinaryOutputStream {
public:
    inline void writeOpCode(OpCode code) { writeUInt8(code); }
    inline uint32_t *writeReservedAddress() { auto addr = (uint32_t *)writeReserved(sizeof(uint32_t)); *addr = 0; return addr; }
    inline void writeAddress(VMAddress addr) { writeUInt32((VMAddress)addr); }
    inline VMAddress address() { return (uint32_t)size(); }

    void writeBreakAddress();
    void writeContinueAddress();

    void enterBreakContinueArea(bool allowContinue = true);
    void leaveBreakContinueArea();

    struct BreakContinueArea {
        bool                        allowContinue;
        VMAddress                   continueAddr;
        vector<VMAddress*>          breakAddrs;

        BreakContinueArea(VMAddress continueAddr, bool allowContinue) : continueAddr(continueAddr), allowContinue(allowContinue) { }
    };

protected:
    list<BreakContinueArea>         _stackBreakContineAreas;

};

#endif /* ByteCodeStream_hpp */
