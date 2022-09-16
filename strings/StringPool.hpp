//
//  StringPool.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/24.
//

#ifndef StringPool_hpp
#define StringPool_hpp

#include "utils/Utils.h"


class StringPool {
public:
    StringPool(uint16_t id, uint32_t size);
    virtual ~StringPool();

    uint8_t *copy(const SizedString &str);
    uint8_t *ptr() { return _last; }
    uint8_t *alloc(uint32_t size) { auto p = _last; _last += size; assert(_last <= _end); return p; }
    uint32_t capacity() const { return (uint32_t)(_end - _last); }
    uint8_t id() const { return _id; }

    StringPool              *prev, *next;

protected:
    friend class StringPoolList;

    uint16_t                _id;
    uint8_t                 *_buf;
    uint8_t                 *_last, *_end;

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
