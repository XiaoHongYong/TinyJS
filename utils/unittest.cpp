//
//  unittest.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#include "unittest.h"
#include "StringEx.h"
#include "CharEncoding.h"


static string g_srcRootDir;

const string &getSourceRootDir() { return g_srcRootDir; }

void initUnittest(int &argc, const char *argv[]) {
#if UNIT_TEST
    testing::InitGoogleTest(&argc, (char **)argv);
#endif

    static cstr_t SRC_ROOT = "--src_root=";

    for (int i = 0; i < argc; i++) {
        if (startsWith(argv[i], SRC_ROOT)) {
            g_srcRootDir.assign(argv[i] + strlen(SRC_ROOT));
            break;
        }
    }
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
    WCHAR path[MAX_PATH] = { 0 };
    GetTempPathW(MAX_PATH, path);
    return ucs2ToUtf8(path);
#else
    return "/tmp/";
#endif
}
