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

// 仅仅运行 runUnittest 即可
void runUnittest(int &argc, const char *argv[]);

// 也可以分为两个步骤来独立运行.
void initUnittest(int &argc, const char *argv[]);
void runAllUnittest();

std::string getUnittestTempDir();

#endif /* unittest_hpp */
