//
//  StringPool.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/24.
//

#ifndef StringPool_hpp
#define StringPool_hpp

#include "utils/Utils.h"


extern uint64_t OCCUPIED_FLAGS_HEAD[];
extern uint64_t OCCUPIED_FLAGS_TAIL[];
const uint64_t OCCUPIED_ALL = 0xFFFFFFFFFFFFFFFFL;

class StringPool {
public:
    StringPool(uint16_t id, uint32_t size);
    virtual ~StringPool();

    uint8_t *copy(const SizedString &str);
    uint8_t *ptr() { return _last; }
    uint8_t *alloc(uint32_t size) { auto p = _last; _last += size; assert(_last <= _end); return p; }
    uint32_t capacity() const { return (uint32_t)(_end - _last); }
    uint8_t id() const { return _id; }

    void markReferred(const SizedString &s) {
        // 按 4 bytes 对齐
        uint32_t offset = uint32_t(s.data - _buf);
        uint32_t len = ((s.len + offset % 4) + 3) & ~3;

        // 分为 4 bytes 一块
        offset /= 4;
        len /= 4;

        auto start = offset % 64, end = start + len;
        uint64_t flags = OCCUPIED_FLAGS_HEAD[start];
        if (end > 64) {
            // 超过了一个 uint64
            _occupiedFlags[offset / 64] |= flags;

            uint32_t remain = end % 64;
            start = start / 64 + 1;
            end = end / 64 - 1;
            while (start <= end) {
                _occupiedFlags[start] = OCCUPIED_ALL;
            }

            _occupiedFlags[start] |= OCCUPIED_FLAGS_TAIL[remain];
        } else {
            // 还无法占满
            flags &= ~(OCCUPIED_FLAGS_HEAD[end]);
            _occupiedFlags[offset / 64] |= flags;
        }
    }

    StringPool              *prev, *next;

protected:
    friend class StringPoolList;

    uint16_t                _id;
    uint8_t                 *_buf;
    uint8_t                 *_last, *_end;
    uint64_t                *_occupiedFlags;
    uint32_t                _offsets[8];

};

class StringPoolList {
public:
    StringPoolList();

    void append(StringPool *node);
    void remove(StringPool *node);
    void moveToTail(StringPool *node);
    StringPool *popFront();

    StringPool *front() { return _head; }
    uint32_t count() { return _count; }

protected:
    bool exists(StringPool *node);
    bool isHealthy();

    uint32_t                _count;
    StringPool              *_head, *_tail;

};

using VecStringPools = std::vector<StringPool *>;

#endif /* StringPool_hpp */
