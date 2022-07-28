//
//  StringPool.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/24.
//

#include "StringPool.hpp"


StringPool::StringPool(uint16_t id, uint32_t size) {
    prev = next = nullptr;

    _id = id;
    _last = _buf = new uint8_t[size];
    _end = _buf + size;
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
