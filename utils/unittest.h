//
//  unittest.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#pragma once

#ifndef unittest_hpp
#define unittest_hpp

#include "gtest/gtest.h"

const std::string &getSourceRootDir();

// 仅仅运行 runUnittest 即可
void runUnittest(int &argc, const char *argv[]);

// 也可以分为两个步骤来独立运行.
void initUnittest(int &argc, const char *argv[]);
void runAllUnittest();

std::string getUnittestTempDir();

inline void assertException(std::function<void()> f, int n) {
    try {
        f();
        ASSERT_EQ(n, -1);
    } catch (std::exception &e) {
    }
}

inline void assertNoException(std::function<void()> f, int n) {
    try {
        f();
    } catch (std::exception &e) {
        ASSERT_EQ(n, -1);
    }
}

#define ASSERT_EXCEPTION(f) assertException(f, __LINE__)
#define ASSERT_NO_EXCEPTION(f) assertNoException(f, __LINE__)

#endif /* unittest_hpp */
