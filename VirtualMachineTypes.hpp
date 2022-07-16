//
//  VirtualMachineTypes.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/6/5.
//

#ifndef VirtualMachineTypes_hpp
#define VirtualMachineTypes_hpp

#include "../Utils/SizedString.h"
#include "../Utils/BinaryStream.h"


class IJsObject;

#define OP_CODE_DEFINES \
    OP_ITEM(OP_INVALID, "not_used"), \
    OP_ITEM(OP_RETURN_VALUE, ""), \
    OP_ITEM(OP_RETURN, ""), \
    OP_ITEM(OP_DEBUGGER, ""), \
    OP_ITEM(OP_THROW, ""), \
    OP_ITEM(OP_JUMP, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_TRUE, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_FALSE, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_UNDEFINED, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_NOT_UNDEFINED, "address:u32"), \
    OP_ITEM(OP_FUNCTION_CALL, "not_used"), \
    OP_ITEM(OP_MEMBER_FUNCTION_CALL, "count_args:u16"), \
    OP_ITEM(OP_DIRECT_FUNCTION_CALL, "scope_depth:u8, function_idx:u16, count_args:u16"), \
    OP_ITEM(OP_PREPARE_RAW_STRING_TEMPLATE_CALL, "raw_string_idx:u16, count_exprs:u16"), \
    \
    OP_ITEM(OP_ENTER_SCOPE, "scope_idx:u16"), \
    OP_ITEM(OP_LEAVE_SCOPE, ""), \
    \
    OP_ITEM(OP_POP_STACK_TOP, ""), \
    OP_ITEM(OP_PUSH_STACK_TOP, ""), \
    \
    OP_ITEM(OP_PUSH_IDENTIFIER, "not_used"), \
    OP_ITEM(OP_PUSH_ID_LOCAL_ARGUMENT, "argument_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_ARGUMENT, "scope_depth:u8, argument_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_GLOBAL, "var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_LOCAL_SCOPE, "var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_SCOPE, "scope_depth:u8, var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_GLOBAL_BY_NAME, ""), \
    OP_ITEM(OP_PUSH_ID_LOCAL_FUNCTION, "function_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_FUNCTION, "scope_depth:u8, function_idx:u16"), \
    OP_ITEM(OP_PUSH_STRING, "string_idx:u32"), \
    OP_ITEM(OP_PUSH_COMMON_STRING, "string_idx:u16"), /*系统常用的 string，不会涉及资源回收*/\
    OP_ITEM(OP_PUSH_INT32, "int_number:i32"), \
    OP_ITEM(OP_PUSH_DOUBLE, "value_idx:u32"), \
    OP_ITEM(OP_PUSH_FUNCTION_EXPR, "scope_depth:u8, function_idx:u32"), \
    \
    OP_ITEM(OP_PUSH_MEMBER_INDEX, ""), \
    OP_ITEM(OP_PUSH_MEMBER_INDEX_INT, "index:u32"), \
    OP_ITEM(OP_PUSH_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_PUSH_MEMBER_DOT_OPTIONAL, "property_string_idx:u32"), \
    \
    OP_ITEM(OP_PUSH_THIS_MEMBER_INDEX, ""), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_INDEX_INT, "index:u32"), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_DOT_OPTIONAL, "property_string_idx:u32"), \
    \
    OP_ITEM(OP_ASSIGN_LOCAL_ARGUMENT, "argument_idx:u16"), \
    OP_ITEM(OP_ASSIGN_IDENTIFIER, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_ASSIGN_MEMBER_INDEX, ""), \
    OP_ITEM(OP_ASSIGN_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_ID_PRE, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_INCREMENT_ID_POST, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_DECREMENT_ID_PRE, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_DECREMENT_ID_POST, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_INCREMENT_MEMBER_DOT_PRE, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_MEMBER_DOT_POST, "property_string_idx:u32"), \
    OP_ITEM(OP_DECREMENT_MEMBER_DOT_PRE, "property_string_idx:u32"), \
    OP_ITEM(OP_DECREMENT_MEMBER_DOT_POST, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_MEMBER_INDEX_PRE, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_MEMBER_INDEX_POST, "property_string_idx:u32"), \
    OP_ITEM(OP_DECREMENT_MEMBER_INDEX_PRE, "property_string_idx:u32"), \
    OP_ITEM(OP_DECREMENT_MEMBER_INDEX_POST, "property_string_idx:u32"), \
    \
    OP_ITEM(OP_ADD, ""), \
    OP_ITEM(OP_SUB, ""), \
    OP_ITEM(OP_MUL, ""), \
    OP_ITEM(OP_DIV, ""), \
    OP_ITEM(OP_MOD, ""), \
    OP_ITEM(OP_EXP, ""), \
    OP_ITEM(OP_INEQUAL_STRICT, ""), \
    OP_ITEM(OP_INEQUAL, ""), \
    OP_ITEM(OP_EQUAL_STRICT, ""), \
    OP_ITEM(OP_EQUAL, ""), \
    \
    OP_ITEM(OP_CONDITIONAL, ""), \
    OP_ITEM(OP_NULLISH, ""), \
    OP_ITEM(OP_LOGICAL_OR, ""), \
    OP_ITEM(OP_LOGICAL_AND, ""), \
    OP_ITEM(OP_BIT_OR, ""), \
    OP_ITEM(OP_BIT_XOR, ""), \
    OP_ITEM(OP_BIT_AND, ""), \
    OP_ITEM(OP_RATIONAL, ""), \
    OP_ITEM(OP_SHIFT, ""), \
    OP_ITEM(OP_UNARY, ""), \
    OP_ITEM(OP_POST_FIX, ""), \
    \
    OP_ITEM(OP_LOGICAL_NOT, ""), \
    OP_ITEM(OP_BIT_NOT, ""), \
    OP_ITEM(OP_LEFT_SHIFT, ""), \
    OP_ITEM(OP_RIGHT_SHIFT, ""), \
    OP_ITEM(OP_UNSIGNED_RIGHT_SHIFT, ""), \
    OP_ITEM(OP_LESS_THAN, ""), \
    OP_ITEM(OP_LESS_EQUAL_THAN, ""), \
    OP_ITEM(OP_GREATER_THAN, ""), \
    OP_ITEM(OP_GREATER_EQUAL_THAN, ""), \
    OP_ITEM(OP_IN, ""), \
    OP_ITEM(OP_INSTANCE_OF, ""), \
    \
    OP_ITEM(OP_NEW, ""), \
    OP_ITEM(OP_NEW_TARGET, ""), \


#ifdef OP_ITEM
#undef OP_ITEM
#endif
#define OP_ITEM(a, b) a

enum OpCode {
    OP_CODE_DEFINES
};

enum JsDataType : uint8_t {
    JDT_NOT_INITIALIZED = 0,
    JDT_UNDEFINED,
    JDT_NULL,
    JDT_INT32,
    JDT_BOOL,
    JDT_NUMBER,
    JDT_STRING,
    JDT_REGEX,
    JDT_ARRAY,
    JDT_OBJECT,
    JDT_FUNCTION,
    JDT_NATIVE_FUNCTION,
    JDT_NATIVE_MEMBER_FUNCTION,
};

const char *jsDataTypeToString(JsDataType type);

struct JsValue {
    uint8_t                     reserved[2];
    JsDataType                  type;
    bool                        isInResourcePool;
    // bool                        isInSysLib; // 不参与引用释放
    union {
        int32_t                 n32;
        uint32_t                objIndex;
        struct {
            uint16_t            poolIndex;
            uint16_t            index;
        } resourcePool;
    } value;

    JsValue() { *(uint64_t *)this = 0; }
    JsValue(bool b) { *(uint64_t *)this = 0; type = JDT_BOOL; value.n32 = b; }
    JsValue(int32_t n32) { *(uint64_t *)this = 0; type = JDT_INT32; value.n32 = n32; }
    JsValue(JsDataType type, uint32_t objIdx) { *(uint64_t *)this = 0; this->type = type; value.objIndex = objIdx; }
};

inline JsValue makeJsValueOfStringInResourcePool(uint16_t poolIndex, uint16_t index) {
    JsValue r;
    r.type = JDT_STRING;
    r.isInResourcePool = true;
    r.value.resourcePool.poolIndex = poolIndex;
    r.value.resourcePool.index = index;
    return r;
}

/**
 * 存储 double 类型的值
 */
struct JsDouble {
    int8_t                      referIdx; // 用于资源回收时所用
    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    double                      value;
};

/**
 * 存储 string 类型的值
 */
struct JsString {
    int8_t                      referIdx; // 用于资源回收时所用
    uint8_t                     stringPoolIdx; // 为此字符串分配内存的 Pool 是哪个
    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    SizedString                 value;
};

using VecJsValues = std::vector<JsValue>;
using VecJsDoubles = std::vector<JsDouble>;
using VecJsStrings = std::vector<JsString>;
using VecJsObjects = std::vector<IJsObject *>;

// NameAtom 用于将 JavaScript 中的 name string 转换为对应的 name index.
using NameAtom = uint32_t;

bool decodeBytecode(uint8_t *bytecode, int lenBytecode, BinaryOutputStream &stream);

#endif /* VirtualMachineTypes_hpp */
