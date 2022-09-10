//
//  unittest.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/28.
//

#include "unittest.h"


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
