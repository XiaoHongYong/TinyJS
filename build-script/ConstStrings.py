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

enum jsStringValueIndex {{
    __JS_STRING_IDX_INVALID__,
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
    # name_prefix, value
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
    [ "Configurable", "configurable" ],
    [ "Enumerable", "enumerable" ],
    [ "Writable", "writable" ],
    [ "Value", "value" ],
    [ "ValueOf", "valueOf" ],
    [ "Get", "get" ],
    [ "Set", "set" ],
    [ 'Name', 'name' ],
    [ 'Caller', 'caller' ],
    [ 'Constructor', 'constructor' ],
    [ 'Object', 'object' ],
    [ 'Function', 'function' ],
    [ 'Number', 'number' ],
    [ 'String', 'string' ],
    [ 'Boolean', 'boolean' ],
    [ 'Bigint', 'bigint' ],
    [ 'Symbol', 'symbol' ],
    [ 'Index', 'index' ],
    [ 'LastIndex', 'lastIndex' ],
    [ 'Groups', 'groups' ],
    [ 'Input', 'input' ],
    [ 'Infinity', 'Infinity' ],
    [ 'NaN', 'NaN' ],
    [ 'InvalidDate', 'Invalid Date' ],
    [ 'dotAll', 'dotAll' ],
    [ 'flags', 'flags' ],
    [ 'global', 'global' ],
    [ 'hasIndices', 'hasIndices' ],
    [ 'ignoreCase', 'ignoreCase' ],
    [ 'multiline', 'multiline' ],
    [ 'source', 'source' ],
    [ 'sticky', 'sticky' ],
    [ 'unicode', 'unicode' ],
    [ 'resolve', 'resolve' ],
    [ 'reject', 'reject' ],
    [ 'then', 'then' ],
    # [ '', '' ],
]

def write_file_cmp(fn, content):
    fn = os.path.abspath(fn)
    print('Write {}, length: {}'.format(fn, len(content)))

    with open(fn, 'rb') as fp:
        if content == fp.read():
            print('No changes of file: {}, ignore wrtting'.format(fn))
            return

    with open(fn, 'wb') as fp:
        fp.write(content)

def build():
    EXTERN_SS_XXS = []
    SS_XXS = []
    enum_jsStringValueIndices = []
    jsStringValues = []
    pushConstStringValues = []
    names = set()

    for name, value in CONST_STRINGS:
        upper_name = name.upper()
        if upper_name in names:
            raise Exception('{} exists already in const string: CONST_STRINGS'.format(name))
        names.add(upper_name)

        EXTERN_SS_XXS.append('extern StringView SS_{};'.format(upper_name));
        SS_XXS.append('StringView SS_{} = makeCommonString("{}");'.format(upper_name, value));
        enum_jsStringValueIndices.append('    JS_STRING_IDX_{},'.format(upper_name));
        jsStringValues.append('const JsValue jsStringValue{} = JsValue(JDT_STRING, JS_STRING_IDX_{});'.format(name, upper_name))

        pushConstStringValues.append('    tmp = rtc->pushStringValue(SS_{}); assert(tmp.value.index == JS_STRING_IDX_{});'.format(upper_name, upper_name))

    this_py_file = os.path.basename(__file__)

    work_dir = os.path.abspath(os.path.dirname(__file__))
    fn_header = os.path.join(work_dir, '../generated/ConstStrings.hpp')
    fn_cpp = os.path.join(work_dir, '../generated/ConstStrings.cpp')

    write_file_cmp(fn_header, HEADER_TEMPLATE.format(this_py_file,
        '\n'.join(EXTERN_SS_XXS),
        '\n'.join(enum_jsStringValueIndices),
        '\n'.join(jsStringValues)))

    write_file_cmp(fn_cpp, CPP_TEMPLATE.format(this_py_file,
        '\n'.join(SS_XXS),
        '\n'.join(pushConstStringValues)))

if __name__ == '__main__':
    build()
