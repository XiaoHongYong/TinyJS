﻿//
//  RunJavaScript.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/8.
//

#include "interpreter/VirtualMachine.hpp"


#if UNIT_TEST

#include "utils/unittest.h"


class StringStreamConsole : public IConsole {
public:
    virtual void log(const StringView &message) override {
        printf("%.*s\n", message.len, message.data);
        stream.write(message);
        stream.writeUInt8('\n');
    }

    virtual void info(const StringView &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    virtual void warn(const StringView &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    virtual void error(const StringView &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    StringView getOutput() { return stream.toStringView(); }

protected:
    BinaryOutputStream          stream;

};

uint8_t *ignoreSpace(const uint8_t *text, const uint8_t *end) {
    while (text < end && isspace(*text)) text++;
    return (uint8_t *)text;
}

/**
 * 找到字符串 [pos, len] 能在 orgRight 中唯一出现一次的长度. 用于打印出错的位置.
 */
uint32_t uniqueLen(const StringView &orgRight, const char *pos) {
    uint32_t maxlen = (uint32_t)(orgRight.data + orgRight.len - pos);
    uint32_t len = min((uint32_t)100, maxlen);
    if (strlen((char *)pos) == 0) {
        return len;
    }

    StringView org(orgRight);
    StringView pt(pos, len);

    // 去掉第一个相同的部分
    auto n = org.strstr(pt);
    assert(n != -1);
    org.shrink(n + (int)len);

    for (int i = 0; i < 10; i++) {
        auto n = org.strstr(pt);
        if (n == -1) break;
        pt.len = min(maxlen, pt.len + 30);
    }

    return pt.len;
}

bool compareTextIgnoreSpace(const StringView &leftS, const StringView &rightS) {
    const char *left = leftS.data, *right = rightS.data;
    auto leftEnd = left + leftS.len, rightEnd = right + rightS.len;

    while (true) {
        auto p1 = left;
        auto p2 = right;

        while (left != leftEnd && right != rightEnd && *left == *right) {
            left++; right++;
        }
        left = ignoreSpace(left, leftEnd);
        right = ignoreSpace(right, rightEnd);

        if (p1 == left && p2 == right) {
            break;
        }
    }

    if (left != leftEnd || right != rightEnd) {
        auto len = uniqueLen(rightS, right);
        printf("NOT EQUAL, at:\nCurrent(%d):  %.100s\nExpected(%d): %.*s\n", (int)(left - leftS.data), left, (int)(right - rightS.data), len, right);
        return false;
    }

    return true;
}

bool runJavascript(const string &code, string &output) {

    JsVirtualMachine vm;

    auto runtime = vm.defaultRuntime();
    auto console = new StringStreamConsole();
    runtime->setConsole(console);

    vm.run(code.c_str(), code.size(), runtime);

    // 循环至少 3 次以上，因为后执行的 task 可能会添加新的 task.
    for (int i = 0; i < 3; i++) {
        if (runtime->onRunTasks()) {
            i = 0;
        } else {
            Sleep(1);
        }
    }

    auto msg = console->getOutput();
    output.assign((const char *)msg.data, msg.len);

    auto countAllocated = runtime->countAllocated();
    auto countFreed = runtime->garbageCollect();
    printf("** CountFreed: %d, CountAllocated: %d\n", countFreed, countAllocated);
    if (countAllocated != countFreed) {
        printf("** NOT FREED **\n");
    }

    return true;
}

void splitTestCodeAndOutput(string textOrg, VecStrings &vCodeOut, VecStrings &vOutputOut) {
    StringView text(textOrg);
    while (true) {
        StringView code, remain;
        if (text.split("/* OUTPUT", code, remain)) {
            if (remain.startsWith("-FIXED")) {
                remain.shrink(6);
            }

            StringView output;
            if (!remain.split("*/", output, remain)) {
                throw "Invalid output format";
            }

            if (output.startsWith("-DISABLED")) {
                code = "";
                output = "";
            }
            vCodeOut.push_back(code.toString());
            vOutputOut.push_back(output.trim().toString());
            text = remain;
        } else {
            text = text.trim();
            if (text.len > 0) {
                printf("NO OUTPUT is specified: %.*s\n", text.len, text.data);
            }
            assert(text.len == 0);
            break;
        }
    }
}

string removeNewLine(const string &str) {
    string tmp = str;
    strrep(tmp, "\n", " ");
    return tmp;
}

TEST(RunJavaScript, outputCheck) {
    FileFind finder;

    string path = "test-cases/check_output/";
    if (isDirExist(path.c_str())) {
        ASSERT_TRUE(finder.openDir(path.c_str()));
    } else {
        path = "TinyJS/test-cases/check_output/";
        ASSERT_TRUE(finder.openDir(path.c_str()));
    }

    char buf[256];
    getcwd(buf, sizeof(buf));

    printf("Current Working Directory: %s\n", buf);

    while (finder.findNext()) {
        string fn = path + finder.getCurName();
        // fn = path + "try.js";
        const int TEST_START = 0;
        if (endsWith(fn.c_str(), ".js") && !startsWith(fn.c_str(), ".")) {
            printf("== Run Js: %s\n", fn.c_str());

            string text;
            readFile(fn.c_str(), text);

            VecStrings vCodes, vOutputs;
            splitTestCodeAndOutput(text, vCodes, vOutputs);
            assert(!vCodes.empty());
            assert(vCodes.size() == vOutputs.size());

            for (size_t i = TEST_START; i < vCodes.size(); i++) {
                string &code = vCodes[i], &outputExpected = vOutputs[i], output;

                runJavascript(code.c_str(), output);

                // printf("   Current:   %s\n", output.c_str());
                // printf("   Expected: %s\n", outputExpected.c_str());

                printf("   Code(%d): %.*s\n", (int)i, (int)70, removeNewLine(code).c_str());
                printf("   Current:   %.*s\n", (int)60, removeNewLine(output).c_str());
                printf("   Expected: %.*s\n", (int)60, removeNewLine(outputExpected).c_str());
                auto result = compareTextIgnoreSpace(output, outputExpected);
                if (!result) {
                    FAIL() << "File: " + fn;
                }
                printf("   Part: %d Passed\n", (int)i);
            }

            printf("   Run Js: %s\n", fn.c_str());
            printf("   Passed\n");
        }
    }
}

#endif
