//
//  RunJavaScript.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/8.
//

#include "../VirtualMachine.hpp"


#if UNIT_TEST

#include "../../Utils/unittest.h"

class StringStreamConsole : public IConsole {
public:
    virtual void log(const SizedString &message) override {
        stream.write(message);
        stream.writeUint8('\n');
    }

    virtual void info(const SizedString &message) override {
        stream.write("[info] ");
        stream.write(message);
        stream.writeUint8('\n');
    }
    
    virtual void warn(const SizedString &message) override {
        stream.write("[warn] ");
        stream.write(message);
        stream.writeUint8('\n');
    }
    
    virtual void error(const SizedString &message) override {
        stream.write("[error] ");
        stream.write(message);
        stream.writeUint8('\n');
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
        printf("NOT EQUAL, at:\nLeft(%d): %.30s\nRight(%d): %.30s\n", (int)(left - orgLeft), left, (int)(right - orgRight), right);
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

bool splitTestCodeAndOutput(string text, string &codeOut, string &outputOut) {
    while (true) {
        string left, right;
        if (strSplit(text.c_str(), "/* OUTPUT", left, right)) {
            codeOut.append(left);
            if (!strSplit(right.c_str(), "*/", left, right)) {
                throw "Invalid output format";
            }
            outputOut.append(left);
            text = right;
        } else {
            codeOut.append(text);
            break;
        }
    }

    return !outputOut.empty();
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
        if (endsWith(fn.c_str(), ".js") && !startsWith(fn.c_str(), ".")) {
            printf("== Run Js: %s\n", fn.c_str());

            string text, code, outputExpected, output;
            readFile(fn.c_str(), text);
            ASSERT_TRUE(splitTestCodeAndOutput(text, code, outputExpected));
            runJavascript(code.c_str(), output);

            printf("   Current:  %.*s\n", (int)60, removeNewLine(output).c_str());
            printf("   Expected: %.*s\n", (int)60, removeNewLine(outputExpected).c_str());
            ASSERT_TRUE(compareTextIgnoreSpace(outputExpected.c_str(), output.c_str()));
            printf("   Passed.\n");
        }
    }
}

#endif

