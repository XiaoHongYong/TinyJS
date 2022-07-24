//
//  ByteCodeStream.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/6/16.
//

#include "InstructionOutputStream.hpp"
#include "AST.hpp"


void InstructionLabel::convertToByteCode(BinaryOutputStream &stream) {
    address = (uint32_t)stream.size();
}

void InstructionLabelFuture::convertToByteCode(BinaryOutputStream &stream) {
    *address = (uint32_t)stream.size();
}

void InstructionAddress::convertToByteCode(BinaryOutputStream &stream) {
    stream.writeUint32(label->address);
}

void InstructionAddressFuture::convertToByteCode(BinaryOutputStream &stream) {
    label->address = (uint32_t *)stream.writeReserved(sizeof(uint32_t));
}

void InstructionEnterScope::convertToByteCode(BinaryOutputStream &stream) {
}

void InstructionLeaveScope::convertToByteCode(BinaryOutputStream &stream) {
}

void InstructionIdentifierAddress::convertToByteCode(BinaryOutputStream &stream) {
    auto declare = idRef->declare;
    stream.writeUint8(declare->varStorageType);
    stream.writeUint8(declare->scopeDepth);
    stream.writeUint16(declare->storageIndex);
}

void InstructionPushIdentifier::convertToByteCode(BinaryOutputStream &stream) {
    auto declare = idRef->declare;
    if (declare->varStorageType == VST_ARGUMENT) {
        stream.writeUint8(OP_PUSH_ID_PARENT_ARGUMENT);
        stream.writeUint8(declare->scopeDepth);
        stream.writeUint16(declare->storageIndex);
    } else if (declare->varStorageType == VST_SCOPE_VAR || declare->varStorageType == VST_FUNCTION_VAR) {
        if (idRef->scope == declare->scope) {
            stream.writeUint8(OP_PUSH_ID_LOCAL_SCOPE);
        } else {
            stream.writeUint8(OP_PUSH_ID_PARENT_SCOPE);
            stream.writeUint8(declare->scopeDepth);
        }
        stream.writeUint16(declare->storageIndex);
    } else if (declare->varStorageType == VST_GLOBAL_VAR) {
        stream.writeUint8(OP_PUSH_ID_GLOBAL);
        stream.writeUint16(declare->storageIndex);
    } else {
        assert(declare->varStorageType == VST_NOT_SET);
        assert(declare->isFuncName);
        if (declare->isFuncName) {
            if (idRef->scope == declare->scope) {
                stream.writeUint8(OP_PUSH_ID_LOCAL_FUNCTION);
            } else {
                stream.writeUint8(OP_PUSH_ID_PARENT_FUNCTION);
                stream.writeUint8(declare->scopeDepth);
            }
        }
        stream.writeUint16(declare->storageIndex);
    }
}

/**
 * 为了优化函数调用性能，如果
 */
void InstructionFunctionCallPush::convertToByteCode(BinaryOutputStream &stream) {
    auto declare = functionName->declare;
    if (declare->varStorageType == VST_ARGUMENT) {
        stream.writeUint8(OP_PUSH_ID_PARENT_ARGUMENT);
        stream.writeUint8(declare->scopeDepth);
        stream.writeUint16(declare->storageIndex);
    } else if (!declare->isModified && declare->isFuncName) {
        // 调用的函数未被修改，并且是函数名，减少创建函数对象和压栈的操作.
        isDirectFunctionCall = true;
    } else {
        // stream.writeUint8(OP_PUSH_IDENTIFIER);
        if (declare->scope->parent == nullptr) {
            stream.writeUint8(OP_PUSH_ID_GLOBAL);
        } else if (functionName->scope == declare->scope) {
            stream.writeUint8(OP_PUSH_ID_LOCAL_SCOPE);
        } else {
            stream.writeUint8(OP_PUSH_ID_PARENT_SCOPE);
            stream.writeUint8(declare->scopeDepth);
        }
        stream.writeUint16(declare->storageIndex);
    }
}

void InstructionFunctionCall::convertToByteCode(BinaryOutputStream &stream) {
    if (push->isDirectFunctionCall) {
        auto functionName = push->functionName;
        auto function = functionName->declare->value.function;
        auto parent = function->scope->parent->function;

        stream.writeUint8(OP_DIRECT_FUNCTION_CALL);
        stream.writeUint8(parent->scope->depth);
        stream.writeUint16(function->index);
    } else {
        stream.writeUint8(OP_FUNCTION_CALL);
    }
}

InstructionOutputStream::InstructionOutputStream(ResourcePool *resPool) : _resPool(resPool), _pool(resPool->pool) {
    _header = _curBuffer = _resPool->allocStreamBuffer();

    _last = (uint8_t *)_curBuffer + sizeof(StreamBuffer);
    _end = (uint8_t *)_curBuffer + BUFF_SIZE;

    _curBuffer->pos = _curBuffer->last = _last;
    _curBuffer->end = _end;

    _offset = 0;
}

StreamBuffer *writeStreamBuffer(BinaryOutputStream &stream, StreamBuffer *buf, uint8_t *&pos, uint32_t count) {
    while (buf && count) {
        assert(pos >= buf->pos && pos < buf->last);
        if (count < buf->last - pos) {
            stream.write(pos, count);
            pos += count;
            break;
        } else {
            // 当前 buf 用完
            uint32_t n = (uint32_t)(buf->last - pos);
            count -= n;
            stream.write(pos, n);
            buf = buf->next;
            if (buf) {
                pos = buf->pos;
            } else {
                pos = nullptr;
            }
        }
    }

    return buf;
}

void InstructionOutputStream::write(const uint8_t *data, size_t len) {
    while (len > 0) {
        uint32_t avail = (uint32_t)(_end - _last);
        if (avail < len) {
            memcpy(_last, data, avail);
            _last += avail;
            data += avail;
            len -= avail;

            newBuffer();
        } else {
            memcpy(_last, data, len);
            _last += len;
            break;
        }
    }
}

void InstructionOutputStream::writeBranch(InstructionOutputStream *branch) {
    branch->_curBuffer->last = branch->_last;

    // 复制数据
    for (auto p = branch->_header; p != nullptr; p = p->next) {
        write(p->pos, size_t(p->last - p->pos));
    }

    // 添加 branch 的 _instructions，但是都需要偏移 _offset
    for (auto item : branch->_instructions) {
        item->offset += _offset;
        _instructions.push_back(item);
    }
    branch->_instructions.clear();

    _offset += branch->_offset;

    // 释放 branch
    branch->freeBuffers();
}

void InstructionOutputStream::convertToByteCode(BinaryOutputStream &stream) {
    StreamBuffer *nextBuf = _header;
    uint8_t *pos = _header->pos;
    uint32_t lastOffset = 0;
    
    for (auto item : _instructions) {
        if (item->offset > lastOffset) {
            nextBuf = writeStreamBuffer(stream, nextBuf, pos, item->offset - lastOffset);
            lastOffset = item->offset;
        }
        item->convertToByteCode(stream);
    }

    writeStreamBuffer(stream, nextBuf, pos, _offset - lastOffset);
}

void InstructionOutputStream::finish() {
    _curBuffer->last = _last;
    if (_end - _last >= sizeof(StreamBuffer) + 20) {
        // 释放剩余的空间
        _curBuffer->end = _last;

        StreamBuffer *buf = (StreamBuffer *)_last;
        buf->end = _end;
        buf->pos = buf->last = _last + sizeof(StreamBuffer);
        buf->next = nullptr;

        _resPool->freeStreamBuffer(buf);
    }
}

void InstructionOutputStream::newBuffer() {
    _curBuffer->last = _last;

    _curBuffer->next = _resPool->allocStreamBuffer();
    _curBuffer = _curBuffer->next;

    _last = _curBuffer->last;
    _end = _curBuffer->end;
}

void InstructionOutputStream::freeBuffers() {
    for (auto p = _header; p != nullptr; ) {
        write(p->pos, size_t(p->last - p->pos));
        auto tmp = p;
        p = p->next;
        tmp->next = nullptr;
        _resPool->freeStreamBuffer(tmp);
    }

    _header = nullptr;
    _curBuffer = nullptr;
    _last = nullptr;
    _end = nullptr;
    _offset = 0;
    assert(_instructions.empty());
}
