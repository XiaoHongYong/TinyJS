//
//  StringPool.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/24.
//

#include "StringPool.hpp"


/** 使用下面的 python 脚本生成此数组
x = 0; i = 0; a = []
for m in range(16):
    b = []
    for k in range(4):
        b.append(hex(0xffffffffffffffff - x))
        x |= 1 << i
        i += 1
    a.append(', '.join(b))

print(',\n'.join(a) + ',')
 */
uint64_t OCCUPIED_FLAGS_HEAD[] = {
    0xffffffffffffffffL, 0xfffffffffffffffeL, 0xfffffffffffffffcL, 0xfffffffffffffff8L,
    0xfffffffffffffff0L, 0xffffffffffffffe0L, 0xffffffffffffffc0L, 0xffffffffffffff80L,
    0xffffffffffffff00L, 0xfffffffffffffe00L, 0xfffffffffffffc00L, 0xfffffffffffff800L,
    0xfffffffffffff000L, 0xffffffffffffe000L, 0xffffffffffffc000L, 0xffffffffffff8000L,
    0xffffffffffff0000L, 0xfffffffffffe0000L, 0xfffffffffffc0000L, 0xfffffffffff80000L,
    0xfffffffffff00000L, 0xffffffffffe00000L, 0xffffffffffc00000L, 0xffffffffff800000L,
    0xffffffffff000000L, 0xfffffffffe000000L, 0xfffffffffc000000L, 0xfffffffff8000000L,
    0xfffffffff0000000L, 0xffffffffe0000000L, 0xffffffffc0000000L, 0xffffffff80000000L,
    0xffffffff00000000L, 0xfffffffe00000000L, 0xfffffffc00000000L, 0xfffffff800000000L,
    0xfffffff000000000L, 0xffffffe000000000L, 0xffffffc000000000L, 0xffffff8000000000L,
    0xffffff0000000000L, 0xfffffe0000000000L, 0xfffffc0000000000L, 0xfffff80000000000L,
    0xfffff00000000000L, 0xffffe00000000000L, 0xffffc00000000000L, 0xffff800000000000L,
    0xffff000000000000L, 0xfffe000000000000L, 0xfffc000000000000L, 0xfff8000000000000L,
    0xfff0000000000000L, 0xffe0000000000000L, 0xffc0000000000000L, 0xff80000000000000L,
    0xff00000000000000L, 0xfe00000000000000L, 0xfc00000000000000L, 0xf800000000000000L,
    0xf000000000000000L, 0xe000000000000000L, 0xc000000000000000L, 0x8000000000000000L,
};

/** 使用下面的 python 脚本生成此数组
x = 0; i = 0; a = []
for m in range(16):
    b = []
    for k in range(4):
        x |= 1 << i
        i += 1
        b.append(hex(x))
    a.append(', '.join(b))

print(',\n'.join(a) + ',')
 */
uint64_t OCCUPIED_FLAGS_TAIL[] = {
    0x1, 0x3, 0x7, 0xf,
    0x1f, 0x3f, 0x7f, 0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff,
    0x1fff, 0x3fff, 0x7fff, 0xffff,
    0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
    0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
    0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
    0x1ffffffff, 0x3ffffffff, 0x7ffffffff, 0xfffffffff,
    0x1fffffffff, 0x3fffffffff, 0x7fffffffff, 0xffffffffff,
    0x1ffffffffff, 0x3ffffffffff, 0x7ffffffffff, 0xfffffffffff,
    0x1fffffffffff, 0x3fffffffffff, 0x7fffffffffff, 0xffffffffffff,
    0x1ffffffffffff, 0x3ffffffffffff, 0x7ffffffffffff, 0xfffffffffffff,
    0x1fffffffffffff, 0x3fffffffffffff, 0x7fffffffffffff, 0xffffffffffffff,
    0x1ffffffffffffff, 0x3ffffffffffffff, 0x7ffffffffffffff, 0xfffffffffffffff,
    0x1fffffffffffffff, 0x3fffffffffffffff, 0x7fffffffffffffff, 0xffffffffffffffffL,
};

StringPool::StringPool(uint16_t id, uint32_t size) {
    prev = next = nullptr;

    _id = id;
    _last = _buf = new uint8_t[size];
    _end = _buf + size;
}

StringPool::~StringPool() {
    delete _buf;
}

StringPoolList::StringPoolList() {
    _head = nullptr;
    _tail = nullptr;
    _count = 0;
}

void StringPoolList::append(StringPool *node) {
    assert(isHealthy());
    assert(!exists(node));

    _count++;

    if (_tail == nullptr) {
        assert(_head == nullptr);
        _head = _tail = node;
        node->next = nullptr;
        node->prev = nullptr;
        return;
    }

    node->next = nullptr;
    node->prev = _tail;
    _tail->next = node;
    _tail = node;

    assert(isHealthy());
}

void StringPoolList::remove(StringPool *node) {
    assert(node != nullptr);
    assert(exists(node));
    assert(isHealthy());

    _count--;

    StringPool *prev = node->prev;
    StringPool *next = node->next;
    if (prev == nullptr) {
        // Remove head
        assert(_head == node);
        _head = next;
        if (_head) {
            _head->prev = nullptr;
        }
    } else {
        prev->next = next;
    }

    if (next == nullptr) {
        // Remove tail
        assert(_tail == node);
        _tail = prev;
    } else {
        next->prev = prev;
    }

    node->next = nullptr;
    node->prev = nullptr;

    assert(isHealthy());
}

void StringPoolList::moveToTail(StringPool *node) {
    remove(node);
    append(node);
}

StringPool *StringPoolList::popFront() {
    assert(isHealthy());

    if (_head == nullptr) {
        return nullptr;
    } else {
        _count--;

        StringPool *p = _head;
        _head = _head->next;
        if (_head == nullptr) {
            _tail = nullptr;
        } else {
            _head->prev = nullptr;
        }

        assert(isHealthy());
        p->next = nullptr;
        return p;
    }
}

bool StringPoolList::exists(StringPool *node) {
    for (auto p = _head; p != nullptr; p = p->next) {
        if (p == node) {
            return true;
        }
    }

    return false;
}

bool StringPoolList::isHealthy() {
    if (_tail == nullptr || _head == nullptr) {
        assert(_head == _tail);
    } else {
        assert(_head->prev == nullptr);
        assert(_tail->next == nullptr);

        StringPool *p = _head;
        while (p->next != nullptr) {
            assert(p->next->prev == p);
            p = p->next;
        }

        assert(p == _tail);
    }

    return true;
}
