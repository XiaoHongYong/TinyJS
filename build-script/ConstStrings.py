# coding=utf-8

'''
此脚本会创建出 2 个文件
* ConstStrings.hpp: 包含了常量字符串的各种声明和定义
* ConstStrings.cpp：包含了常量字符串的实现
'''

import os

HEADER_TEMPLATE = '''/**
 * 此文件由 {} 自动创建，请不要手动修改
 */

#ifndef ConstStrings_hpp
#define ConstStrings_hpp

{}

enum JsValueStringIndex {{
{}
}};

{}

void addConstStrings(class VMRuntimeCommon *rtc);

#endif // ConstStrings_hpp
'''

CPP_TEMPLATE = '''/**
 * 此文件由 {} 自动创建，请不要手动修改
 */

#include "VMRuntime.hpp"


{}

void addConstStrings(VMRuntimeCommon *rtc) {{
    JsValue tmp;

{}
}}

'''

CONST_STRINGS = [
    [ 'Empty', '' ],
    [ 'Undefined', 'undefined' ],
    [ 'Null', 'null' ],
    [ 'True', 'true' ],
    [ 'False', 'false' ],
    [ 'This', 'this' ],
    [ 'Arguments', 'arguments' ],
    [ 'Length', 'length' ],
    [ 'ToString', 'toString' ],
    [ '__proto__', '__proto__' ],
    [ 'Prototype', 'prototype' ],
    [ 'FunctionNativeCode', 'function String() { [native code] }' ],
    [ 'Error', 'Error' ],
    [ 'message', 'message' ],
    [ 'stack', 'stack' ],
]

def write_file(fn, content):
    fn = os.path.abspath(fn)
    print('Write {}, length: {}'.format(fn, len(content)))

    with open(fn, 'wb') as fp:
        fp.write(content)

def build(fn_header, fn_cpp):
    EXTERN_SS_XXS = []
    SS_XXS = []
    enum_JsValueStringIndices = []
    jsStringValues = []
    pushConstStringValues = []

    for name, value in CONST_STRINGS:
        upper_name = name.upper()
        EXTERN_SS_XXS.append('extern SizedString SS_{};'.format(upper_name));
        SS_XXS.append('SizedString SS_{} = makeCommonString("{}");'.format(upper_name, value));
        enum_JsValueStringIndices.append('    JS_STRING_IDX_{},'.format(upper_name));
        jsStringValues.append('const JsValue JsStringValue{} = JsValue(JDT_STRING, JS_STRING_IDX_{});'.format(name, upper_name))

        pushConstStringValues.append('    tmp = rtc->pushStringValue(SS_{}); assert(tmp.value.index == JS_STRING_IDX_{});'.format(upper_name, upper_name))

    this_py_file = os.path.basename(__file__)

    write_file(fn_header, HEADER_TEMPLATE.format(this_py_file,
        '\n'.join(EXTERN_SS_XXS),
        '\n'.join(enum_JsValueStringIndices),
        '\n'.join(jsStringValues)))

    write_file(fn_cpp, CPP_TEMPLATE.format(this_py_file,
        '\n'.join(SS_XXS),
        '\n'.join(pushConstStringValues)))

if __name__ == '__main__':
    work_dir = os.path.abspath(os.path.dirname(__file__))

    build(os.path.join(work_dir, '../ConstStrings.hpp'),
        os.path.join(work_dir, '../ConstStrings.cpp'))
