//
//  ByteCodeStream.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/18.
//

#ifndef ByteCodeStream_hpp
#define ByteCodeStream_hpp

#include "utils/BinaryStream.h"
#include "VirtualMachineTypes.hpp"


class ByteCodeStream : public BinaryOutputStream {
public:
    inline void writeOpCode(OpCode code) { writeUInt8(code); }
    inline uint32_t *writeReservedAddress() { auto addr = (uint32_t *)writeReserved(sizeof(uint32_t)); *addr = 0; return addr; }
    inline void writeAddress(size_t addr) { writeUInt32((uint32_t)addr); }
    inline uint32_t address() { return (uint32_t)size(); }

};

#endif /* ByteCodeStream_hpp */
