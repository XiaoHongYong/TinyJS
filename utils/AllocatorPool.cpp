//
//  Copyright (c) 2021 CrintSoft, Ltd. All rights reserved.
//

#include "AllocatorPool.h"


AllocatorPool::AllocatorPool(size_t blockSize) {
    _max = blockSize - sizeof(PoolBlock) + sizeof(PoolBlock::buf);
    _poolBlock = nullptr;
    _poolBlockMax = nullptr;
    _start = nullptr;
    _end = nullptr;
}

AllocatorPool::~AllocatorPool() {
    freePoolBlockChain(_poolBlockMax);
    freePoolBlockChain(_poolBlock);
}

void AllocatorPool::reset() {
    freePoolBlockChain(_poolBlockMax);
    freePoolBlockChain(_poolBlock);

    _poolBlock = nullptr;
    _poolBlockMax = nullptr;
    _start = nullptr;
    _end = nullptr;
}
