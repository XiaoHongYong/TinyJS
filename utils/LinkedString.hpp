//
//  LinkedString.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/15.
//

#ifndef LinkedString_hpp
#define LinkedString_hpp

#pragma once

#include <cstdint>


/**
 * LinkedString 只能被作为指针使用，data 会被扩展超过 4 个字节的
 */
struct LinkedString {
    LinkedString                *next;
    uint32_t                    len;
    uint8_t                     data[4];

    LinkedString() {
        static_assert(sizeof(LinkedString) == sizeof(uint64_t) * 2, "LinkedString should be 16 bytes long");
        auto p = (uint64_t *)this;
        p[0] = 0;
        p[1] = 0;
    }

};

#endif /* LinkedString_hpp */
