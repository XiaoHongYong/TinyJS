
//
//  InstructionOutputStream.hpp
//
//  Created by henry_xiao on 2022/6/16.
//

#ifndef InstructionOutputStream_hpp
#define InstructionOutputStream_hpp

#include "../Utils/Utils.h"
#include "VirtualMachineTypes.hpp"


class Scope;
class IdentifierRef;
class InstructionBase;
class ResourcePool;


using VecInstructions = std::vector<InstructionBase *>;


class InstructionBase {
public:
    uint32_t                offset;

    InstructionBase(uint32_t offset) : offset(offset) { }

    virtual void convertToByteCode(BinaryOutputStream &stream) = 0;

};

class InstructionLabel : public InstructionBase {
public:
    uint32_t                address;

    InstructionLabel(uint32_t offset) : InstructionBase(offset) { address = 0; }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

/**
 * 未来的跳转标签
 */
class InstructionLabelFuture : public InstructionBase {
public:
    uint32_t                *address;

    InstructionLabelFuture(uint32_t offset) : InstructionBase(offset) { address = nullptr; }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionAddress : public InstructionBase {
public:
    InstructionAddress(InstructionLabel *label, uint32_t offset) : InstructionBase(offset), label(label) { }
    InstructionLabel        *label;

    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

/**
 * 写入未来的地址（跳转地址还没有确定）
 */
class InstructionAddressFuture : public InstructionBase {
public:
    InstructionLabelFuture  *label;
    bool                    mustExist;

    InstructionAddressFuture(uint32_t offset, bool mustExist = true) : InstructionBase(offset), mustExist(mustExist) { label = nullptr; }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionEnterScope : public InstructionBase {
public:
    Scope                   *scope;

    InstructionEnterScope(Scope *scope, uint32_t offset) : InstructionBase(offset), scope(scope) { }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionLeaveScope : public InstructionBase {
public:
    Scope                   *scope;

    InstructionLeaveScope(Scope *scope, uint32_t offset) : InstructionBase(offset), scope(scope) { }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionIdentifierAddress : public InstructionBase {
public:
    IdentifierRef           *idRef;

    InstructionIdentifierAddress(IdentifierRef *ref, uint32_t offset) : InstructionBase(offset), idRef(ref) { }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionPushIdentifier : public InstructionBase {
public:
    IdentifierRef           *idRef;

    InstructionPushIdentifier(IdentifierRef *ref, uint32_t offset) : InstructionBase(offset), idRef(ref) { }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionFunctionCallPush : public InstructionBase {
public:
    IdentifierRef           *functionName;
    bool                    isDirectFunctionCall;

    InstructionFunctionCallPush(IdentifierRef *functionName, uint32_t offset) : InstructionBase(offset), functionName(functionName) { isDirectFunctionCall = false; }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};

class InstructionFunctionCall : public InstructionBase {
public:
    InstructionFunctionCallPush *push;

    InstructionFunctionCall(InstructionFunctionCallPush *push, uint32_t offset) : InstructionBase(offset), push(push) { }
    virtual void convertToByteCode(BinaryOutputStream &stream) override;

};


class InstructionOutputStream {
public:
    enum {
        BUFF_SIZE = 1024 * 4,
    };

    InstructionOutputStream(ResourcePool *resPool);

    InstructionLabel *tagLabel() {
        auto label = PoolNew(_pool, InstructionLabel)(_offset);
        _instructions.push_back(label);
        return label;
    }

    void tagLabel(InstructionAddressFuture *addr) {
        auto label = PoolNew(_pool, InstructionLabelFuture)(_offset);
        _instructions.push_back(label);
        addr->label = label;
    }

    InstructionAddressFuture *writeAddressFuture(bool addrMustExist = true) {
        auto addr = PoolNew(_pool, InstructionAddressFuture)(_offset, addrMustExist);
        _instructions.push_back(addr);
        return addr;
    }

    void writeAddress(InstructionLabel *label) {
        auto labelAddr = PoolNew(_pool, InstructionAddress)(label, _offset);
        _instructions.push_back(labelAddr);
    }

    void writeIdentifierAddress(IdentifierRef *idRef) {
        auto item = PoolNew(_pool, InstructionIdentifierAddress)(idRef, _offset);
        _instructions.push_back(item);
    }

    void writePushIdentifier(IdentifierRef *idRef) {
        auto item = PoolNew(_pool, InstructionPushIdentifier)(idRef, _offset);
        _instructions.push_back(item);
    }
    
    InstructionFunctionCallPush *writeFunctionCallPush(IdentifierRef *idRef) {
        auto item = PoolNew(_pool, InstructionFunctionCallPush)(idRef, _offset);
        _instructions.push_back(item);
        return item;
    }

    void writeFunctionCall(InstructionFunctionCallPush *push) {
        auto item = PoolNew(_pool, InstructionFunctionCall)(push, _offset);
        _instructions.push_back(item);
    }

    void writeEnterScope(Scope *scope) {
        auto item = PoolNew(_pool, InstructionEnterScope)(scope, _offset);
        _instructions.push_back(item);
    }

    void writeLeaveScope(Scope *scope) {
        auto item = PoolNew(_pool, InstructionLeaveScope)(scope, _offset);
        _instructions.push_back(item);
    }

    void writeOpCode(uint8_t opcode) {
        if (_last + 1 > _end) {
            newBuffer();
        }

        *_last++ = opcode;
        _offset++;
    }

    void writeUint8(uint8_t c) {
        if (_last + 1 > _end) {
            newBuffer();
        }

        *_last++ = c;
        _offset++;
    }

    void writeUint16(uint16_t n) {
        if (_last + 2 > _end) {
            newBuffer();
        }

        *(uint16_t *)_last = n;
        _last += 2;
        _offset += 2;
    }

    void writeUint32(uint32_t n) {
        if (_last + 4 > _end) {
            newBuffer();
        }

        *(uint32_t *)_last = n;
        _last += 4;
        _offset += 4;
    }

    void writeUint64(uint64_t n) {
        if (_last + 8 > _end) {
            newBuffer();
        }

        *(uint64_t *)_last = n;
        _last += 8;
        _offset += 8;
    }

    void write(const uint8_t *data, size_t len);
    void writeBranch(InstructionOutputStream *branch);
    void convertToByteCode(BinaryOutputStream &stream);
    void finish();

protected:
    void newBuffer();
    void freeBuffers();

protected:
    ResourcePool                *_resPool;
    AllocatorPool               &_pool;
    VecInstructions             _instructions;

    StreamBuffer                *_header;
    StreamBuffer                *_curBuffer;
    uint8_t                     *_last;
    uint8_t                     *_end;
    uint32_t                    _offset;

};


#endif /* InstructionOutputStream_hpp */
