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
    OP_ITEM(OP_PREPARE_VAR_THIS, ""), \
    OP_ITEM(OP_PREPARE_VAR_ARGUMENTS, ""), \
    OP_ITEM(OP_INIT_FUNCTION_TO_VARS, ""), \
    OP_ITEM(OP_INIT_FUNCTION_TO_ARGS, ""), \
    \
    OP_ITEM(OP_RETURN_VALUE, ""), \
    OP_ITEM(OP_RETURN, ""), \
    OP_ITEM(OP_DEBUGGER, ""), \
    OP_ITEM(OP_THROW, ""), \
    OP_ITEM(OP_JUMP, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_TRUE, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_FALSE, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_NULL_UNDEFINED, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_NOT_NULL_UNDEFINED, "address:u32"), \
    OP_ITEM(OP_PREPARE_RAW_STRING_TEMPLATE_CALL, "raw_string_idx:u16, count_exprs:u16"), \
    OP_ITEM(OP_FUNCTION_CALL, "count_args:u16"), \
    OP_ITEM(OP_MEMBER_FUNCTION_CALL, "count_args:u16"), \
    OP_ITEM(OP_DIRECT_FUNCTION_CALL, "scope_depth:u8, function_idx:u16, count_args:u16"), \
    \
    OP_ITEM(OP_ENTER_SCOPE, "scope_idx:u16"), \
    OP_ITEM(OP_LEAVE_SCOPE, ""), \
    \
    OP_ITEM(OP_POP_STACK_TOP, ""), \
    OP_ITEM(OP_PUSH_STACK_TOP, ""), \
    \
    OP_ITEM(OP_PUSH_IDENTIFIER, "not_used"), \
    OP_ITEM(OP_PUSH_ID_LOCAL_SCOPE, "var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_LOCAL_ARGUMENT, "argument_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_ARGUMENT, "scope_depth:u8, argument_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_GLOBAL, "var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_SCOPE, "scope_depth:u8, var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_GLOBAL_BY_NAME, ""), \
    OP_ITEM(OP_PUSH_ID_LOCAL_FUNCTION, "function_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_FUNCTION, "scope_depth:u8, function_idx:u16"), \
    OP_ITEM(OP_PUSH_STRING, "string_idx:u32"), \
    OP_ITEM(OP_PUSH_COMMON_STRING, "string_idx:u16"), /*系统常用的 string，不会涉及资源回收*/\
    OP_ITEM(OP_PUSH_INT32, "int_number:i32"), \
    OP_ITEM(OP_PUSH_DOUBLE, "value_idx:u32"), \
    OP_ITEM(OP_PUSH_FUNCTION_EXPR, "scope_depth:u8, function_idx:u16"), \
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
    OP_ITEM(OP_ASSIGN_IDENTIFIER, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_ASSIGN_LOCAL_ARGUMENT, "argument_idx:u16"), \
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
    OP_ITEM(OP_NEW, "count_args:u16"), \
    OP_ITEM(OP_NEW_TARGET, ""), \
    \
    OP_ITEM(OP_OBJ_CREATE, ""), \
    OP_ITEM(OP_OBJ_SET_PROPERTY, "property_string_idx:u32"), \
    OP_ITEM(OP_OBJ_SET_COMPUTED_PROPERTY, ""), \
    OP_ITEM(OP_OBJ_SPREAD_PROPERTY, ""), \
    OP_ITEM(OP_OBJ_SET_GETTER, "property_string_idx:u32"), \
    OP_ITEM(OP_OBJ_SET_SETTER, "property_string_idx:u32"), \
    \
    OP_ITEM(OP_TRY_START, "address_catch:u32, address_finally:u32"), \
    OP_ITEM(OP_BEGIN_FINALLY_NORMAL, ""), \
    OP_ITEM(OP_FINISH_FINALLY, ""), \


#ifdef OP_ITEM
#undef OP_ITEM
#endif
#define OP_ITEM(a, b) a

enum OpCode {
    OP_CODE_DEFINES
};

const char *opCodeToString(OpCode code);

enum JsDataType : uint8_t {
    JDT_NOT_INITIALIZED = 0,
    JDT_UNDEFINED,
    JDT_NULL,
    JDT_BOOL,
    JDT_INT32,
    JDT_NUMBER,
    JDT_SYMBOL,
    JDT_CHAR, // 单个字符的 String
    JDT_STRING,
    JDT_OBJECT,
    JDT_REGEX,
    JDT_ARRAY,
    JDT_LIB_OBJECT,
    JDT_FUNCTION,
    JDT_NATIVE_FUNCTION,
};

const char *jsDataTypeToString(JsDataType type);


inline uint32_t makeResourceIndex(uint16_t poolIdx, uint16_t resIdx) { return (poolIdx << 16) | resIdx; }
inline uint16_t getPoolIndexOfResource(uint32_t resIndex) { return resIndex >> 16; }
inline uint16_t getIndexOfResource(uint32_t resIndex) { return resIndex & 0xFFFF; }

struct JsValue {
    uint8_t                     reserved[1];
    JsDataType                  type;
    bool                        isInResourcePool;
    union {
        int32_t                 n32;
        uint32_t                index;
    } value;

    JsValue() { *(uint64_t *)this = 0; }
    JsValue(bool b) { *(uint64_t *)this = 0; type = JDT_BOOL; value.n32 = b; }
    JsValue(int32_t n32) { *(uint64_t *)this = 0; type = JDT_INT32; value.n32 = n32; }
    JsValue(JsDataType type, uint32_t objIdx) { *(uint64_t *)this = 0; this->type = type; value.index = objIdx; }
};

inline bool operator==(const JsValue &a, const JsValue &b) {
    return a.type == b.type && a.isInResourcePool == b.isInResourcePool && a.value.index == b.value.index;
}

inline JsValue makeJsValueOfStringInResourcePool(uint16_t poolIndex, uint16_t index) {
    JsValue r(JDT_STRING, makeResourceIndex(poolIndex, index));
    r.isInResourcePool = true;
    return r;
}

/**
 * 存储 double 类型的值
 */
struct JsDouble {
    JsDouble() { referIdx = 0; nextFreeIdx = 0; value = 0; }
    JsDouble(double v) { referIdx = 0; nextFreeIdx = 0; value = v; }

    int8_t                      referIdx; // 用于资源回收时所用
    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    double                      value;
};

/**
 * 存储 Symbol 类型的值
 */
struct JsSymbol {
    JsSymbol() { referIdx = 0; nextFreeIdx = 0; name = nullptr; }
    JsSymbol(const char *name) { referIdx = 0; nextFreeIdx = 0; this->name = name; }

    int8_t                      referIdx; // 用于资源回收时所用
    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    const char                  *name;
};

//
// 为了压缩 JsString 的存储空间，因此改为 2 字节对齐。
// 后续增加成员时，需仔细考虑对齐
//
#pragma pack(push)
#pragma pack(2)

/**
 * 存储 pool string 类型的值
 */
struct JsPoolString {
    uint8_t                     reserved;
    uint8_t                     poolIdx; // 为此字符串分配内存的 Pool 是哪个
    SizedString                 value;
};

/**
 * 存储 joined string 类型的值
 */
struct JsJoinedString {
    bool                        isStringIdxInResourcePool; // stringIdx 指向的是 ResourcePool?
    bool                        isNextStringIdxInResourcePool; // nextStringIdx 指向的是 ResourcePool?
    uint32_t                    stringIdx;
    uint32_t                    nextStringIdx; // 下一个连接的字符串索引位置
    uint32_t                    len; // 长度大小
};

/**
 * 存储 string 类型的值
 */
struct JsString {
    JsString() { referIdx = 0; nextFreeIdx = 0; isJoinedString = false; }
    JsString(const JsPoolString &poolString) { referIdx = 0; nextFreeIdx = 0; isJoinedString = false; value.poolString = poolString; }
    JsString(const JsJoinedString &joinedString) { referIdx = 0; nextFreeIdx = 0; isJoinedString = true; value.joinedString = joinedString; }

    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    int8_t                      referIdx; // 用于资源回收时所用
    bool                        isJoinedString;
    union Value {
        Value() { }
        JsJoinedString          joinedString;
        JsPoolString            poolString;
    } value;
};

static_assert(sizeof(JsString) == 24, "JsPoolString should be 24 bytes long.");

#pragma pack(pop)

enum JsValueStringIndex {
    JS_STRING_IDX_EMPTY         = 0,
    JS_STRING_IDX_UNDEFINED,
    JS_STRING_IDX_NULL,
    JS_STRING_IDX_TRUE,
    JS_STRING_IDX_FALSE,

    SS_STRING_IDX_THIS,
    SS_STRING_IDX_ARGUMENTS,
    SS_STRING_IDX_LENGTH,
    SS_STRING_IDX_TO_STRING,

    SS_STRING_IDX__PROTO__,
    SS_STRING_IDX_PROTOTYPE,

    SS_STRING_IDX_FUNCTION_NATIVE_CODE,
};

const JsValue JsNotInitializedValue = JsValue();
const JsValue JsNullValue = JsValue(JDT_NULL, 0);
const JsValue JsUndefinedValue = JsValue(JDT_UNDEFINED, 0);

const JsValue JsStringValueEmpty = JsValue(JDT_STRING, JS_STRING_IDX_EMPTY);
const JsValue JsStringValueUndefined = JsValue(JDT_STRING, JS_STRING_IDX_UNDEFINED);
const JsValue JsStringValueNull = JsValue(JDT_STRING, JS_STRING_IDX_NULL);
const JsValue JsStringValueTrue = JsValue(JDT_STRING, JS_STRING_IDX_TRUE);
const JsValue JsStringValueFalse = JsValue(JDT_STRING, JS_STRING_IDX_FALSE);

const JsValue JsStringValueThis = JsValue(JDT_STRING, SS_STRING_IDX_THIS);
const JsValue JsStringValueArguments = JsValue(JDT_STRING, SS_STRING_IDX_ARGUMENTS);
const JsValue JsStringValueLength = JsValue(JDT_STRING, SS_STRING_IDX_LENGTH);
const JsValue JsStringValueToString = JsValue(JDT_STRING, SS_STRING_IDX_TO_STRING);

const JsValue JsStringValue__proto__ = JsValue(JDT_STRING, SS_STRING_IDX__PROTO__);
const JsValue JsStringValuePrototype = JsValue(JDT_STRING, SS_STRING_IDX_PROTOTYPE);

const JsValue JsStringValueFunctionNativeCode = JsValue(JDT_STRING, SS_STRING_IDX_FUNCTION_NATIVE_CODE);

using VecJsValues = std::vector<JsValue>;
using VecJsDoubles = std::vector<JsDouble>;
using VecJsStrings = std::vector<JsString>;
using VecJsSymbols = std::vector<JsSymbol>;
using VecJsObjects = std::vector<IJsObject *>;

// NameAtom 用于将 JavaScript 中的 name string 转换为对应的 name index.
using NameAtom = uint32_t;

bool decodeBytecode(uint8_t *bytecode, int lenBytecode, BinaryOutputStream &stream);

#endif /* VirtualMachineTypes_hpp */
