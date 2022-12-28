//
//  unittest.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#include "unittest.h"


void initUnittest(int &argc, const char *argv[]) {
#if UNIT_TEST
    testing::InitGoogleTest(&argc, (char **)argv);
#endif
}

void runAllUnittest() {
#if UNIT_TEST
    int ret = RUN_ALL_TESTS();

    printf("Quit UT, result: %d.\n", ret);
    if (ret != 0) {
        exit(ret);
    }
#endif
}

void runUnittest(int &argc, const char *argv[]) {
#if UNIT_TEST
    testing::InitGoogleTest(&argc, (char **)argv);

    int ret = RUN_ALL_TESTS();

    printf("Quit UT, result: %d.\n", ret);
    if (ret != 0) {
        exit(ret);
    }
#endif
}

std::string getUnittestTempDir() {
#ifdef _WIN32
    char path[MAX_PATH];
    GetModulePath(path, NULL);
    return path;
#else
    return "/tmp/";
#endif
}
