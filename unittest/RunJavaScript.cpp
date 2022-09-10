//
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
    virtual void log(const SizedString &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    virtual void info(const SizedString &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    virtual void warn(const SizedString &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    virtual void error(const SizedString &message) override {
        stream.write(message);
        stream.writeUInt8('\n');
    }

    SizedString getOutput() { return stream.toSizedString(); }

protected:
    BinaryOutputStream          stream;

};

const char *ignoreSpace(const char *text) {
    while (isspace(*text)) text++;
    return text;
}

bool compareTextIgnoreSpace(const char *left, const char *right) {
    auto orgLeft = left, orgRight = right;

    while (true) {
        auto p1 = left;
        auto p2 = right;

        while (*left != '\0' && *left == *right) {
            left++; right++;
        }
        left = ignoreSpace(left);
        right = ignoreSpace(right);

        if (p1 == left && p2 == right)
            break;
    }

    if (*left != *right) {
        printf("NOT EQUAL, at:\nCurrent(%d):  %.100s\nExpected(%d): %.100s\n", (int)(left - orgLeft), left, (int)(right - orgRight), right);
    }

    return *left == *right;
}

bool runJavascript(const string &code, string &output) {

    JsVirtualMachine vm;

    auto runtime = vm.defaultRuntime();
    auto console = new StringStreamConsole();
    runtime->setConsole(console);

    vm.run(code.c_str(), code.size(), runtime);

    auto msg = console->getOutput();
    output.assign((const char *)msg.data, msg.len);

    return true;
}

void splitTestCodeAndOutput(string text, VecStrings &vCodeOut, VecStrings &vOutputOut) {
    while (true) {
        string left, right;
        if (strSplit(text.c_str(), "/* OUTPUT", left, right)) {
            vCodeOut.push_back(left);

            if (startsWith(right.c_str(), "-FIXED")) {
                right.erase(right.begin(), right.begin() + 6);
            }
            if (!strSplit(right.c_str(), "*/", left, right)) {
                throw "Invalid output format";
            }
            vOutputOut.push_back(left);
            text = right;
        } else {
            trimStr(text);
            if (!text.empty()) {
                printf("NO OUTPUT is specified: %s\n", text.c_str());
            }
            assert(text.empty());
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
    ASSERT_TRUE(finder.openDir(path.c_str()));

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
                auto result = compareTextIgnoreSpace(output.c_str(), outputExpected.c_str());
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

