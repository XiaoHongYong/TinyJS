#pragma once

//
//  Copyright (c) 2021 CrintSoft, Ltd. All rights reserved.
//

#ifndef __AllocatorPool__
#define __AllocatorPool__

#include "UtilsTypes.h"
#include "LinkedString.hpp"


#define alignPtr(p, a)      (uint8_t *) (((uintptr_t) (p) + ((uintptr_t)(a) - 1)) & ~((uintptr_t)(a) - 1))


class AllocatorPool {
private:
    AllocatorPool(const AllocatorPool &);
    AllocatorPool &operator=(const AllocatorPool &);

    struct PoolBlock {
        struct PoolBlock        *next;
        uint8_t                     buf[8];
    };

public:
    AllocatorPool(size_t blockSize = 1024 * 16);
    virtual ~AllocatorPool();

    inline void *allocate(size_t n) {
        if (n >= _max) {
            PoolBlock *b = (PoolBlock *)new uint8_t[n + sizeof(PoolBlock) - sizeof(PoolBlock::buf)];
            b->next = _poolBlockMax;
            _poolBlockMax = b;
            return b->buf;
        }

        if (n <= 4) {
            if (n <= 2) {
                _start = alignPtr(_start, 2);
            } else {
                _start = alignPtr(_start, 4);
            }
        } else {
            alignPtr(_start, 8);
        }

        if (_start + n > _end) {
            PoolBlock *b = (PoolBlock *)new uint8_t[_max + sizeof(PoolBlock) - sizeof(PoolBlock::buf)];
            b->next = _poolBlock;
            _poolBlock = b;
            _start = b->buf;
            _end = _start + _max;
        }
        void *p = _start;
        _start += n;
        return p;
    }

    inline void *allocate(size_t n, size_t alignment) {
        return allocate(n);
    }

    inline void deallocate(void *p, size_t size) {
        if (size > _max) {
            // TODO: Only large memory can be freed
        }
    }

    SizedString duplicate(const SizedString &s) {
        uint8_t *p = (uint8_t *)allocate(s.len);
        memcpy(p, s.data, s.len);

        return SizedString(p, s.len);
    }

    SizedString duplicate(const LinkedString *s) {
        assert(s != nullptr);

        // 计算长度
        uint32_t len = 0;
        auto ps = s;
        do {
            len += ps->len;
            ps = ps->next;
        } while (ps);

        // 复制
        uint8_t *data = (uint8_t *)allocate(len);
        auto p = data;
        ps = s;
        do {
            memcpy(p, ps->data, ps->len);
            p += ps->len;
            ps = ps->next;
        } while (ps);

        return SizedString(data, len);
    }

    char *duplicate(const char *s) {
        auto len = strlen(s) + 1;
        char *p = (char *)allocate(len);
        memcpy(p, s, len);

        return p;
    }

    template<typename T>
    T *duplicate(const T *s, size_t len) {
        T *p = (T *)allocate(len);
        memcpy(p, s, len);

        return p;
    }

    void reset();

    size_t totalSize() {
        size_t size = 0;
        for (auto p = _poolBlock; p != nullptr; p = p->next) {
            size += _max;
        }

        for (auto p = _poolBlockMax; p != nullptr; p = p->next) {
            size += _max; // TODO...
        }

        return size;
    }

protected:
    void freePoolBlockChain(PoolBlock *b) {
        while (b) {
            auto tmp = b;
            b = b->next;
            delete [] (uint8_t *)tmp;
        }
    }

protected:
    PoolBlock                   *_poolBlock;
    PoolBlock                   *_poolBlockMax;
    uint8_t                     *_start, *_end;
    size_t                      _max;

};


template <typename T, typename Pool>
class Allocator_t {
public:
    typedef size_t          size_type;
    typedef T              *pointer;
    typedef const T        *const_pointer;
    typedef T              &reference;
    typedef const T        &const_reference;
    typedef T               value_type;

    Allocator_t(Pool &pool) : _allocatorPool(pool) { }
    ~Allocator_t() { }

    template <class U> struct rebind { typedef  Allocator_t<U, Pool> other; };
    template <class U> Allocator_t(const Allocator_t<U, Pool> &other) : Allocator_t<T, Pool>(other.getPool()) { }

    pointer allocate(size_type n) {
        T *p = static_cast<T *>(_allocatorPool.allocate(n * sizeof(T), 1));
        return p;
    }

    void deallocate(pointer p, size_type n) {
        _allocatorPool.deallocate(p, n * sizeof(T));
    }

    Pool &getPool() const { return _allocatorPool; }

protected:
    Pool                        &_allocatorPool;

};


template <typename T>
class Allocator : public Allocator_t<T, AllocatorPool> {
public:
    Allocator(AllocatorPool &pool) : Allocator_t<T, AllocatorPool>(pool) { }
    Allocator(const Allocator &allocator) : Allocator_t<T, AllocatorPool>(allocator.getPool()) { }
    Allocator(const Allocator_t<const T, AllocatorPool> &allocator) : Allocator_t<T, AllocatorPool>(allocator.getPool()) { }

};


//
//template <class T, class U>
//bool operator==(const Allocator<T>&, const Allocator<U>&) { return true; }
//
//template <class T, class U>
//bool operator!=(const Allocator<T>&, const Allocator<U>&) { return false; }

#ifndef PoolNew
#define PoolNew(pool, type) new (pool.allocate(sizeof(type), 1)) type
#endif

#endif /* defined(__AllocatorPool__) */
