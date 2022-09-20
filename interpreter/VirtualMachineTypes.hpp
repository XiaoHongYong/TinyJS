//
//  VirtualMachineTypes.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/6/5.
//

#ifndef VirtualMachineTypes_hpp
#define VirtualMachineTypes_hpp

#include <deque>
#include "utils/SizedString.h"
#include "utils/BinaryStream.h"


class IJsObject;
class VMContext;

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
    OP_ITEM(OP_JUMP_IF_TRUE_KEEP_VALID, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_FALSE, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_FALSE_KEEP_COND, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_NULL_UNDEFINED, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_NOT_NULL_UNDEFINED, "address:u32"), \
    OP_ITEM(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID, "address:u32"), \
    OP_ITEM(OP_SWITCH_CASE_CMP_JUMP, "address:u32"), \
    OP_ITEM(OP_SWITCH_CASE_FAST_CMP_JUMP, "switch_jump_idx:u16"), \
    OP_ITEM(OP_SET_WITH_OBJ, ""), \
    \
    OP_ITEM(OP_PREPARE_RAW_STRING_TEMPLATE_CALL, "raw_string_idx:u16, count_exprs:u16"), \
    OP_ITEM(OP_FUNCTION_CALL, "count_args:u16"), \
    OP_ITEM(OP_MEMBER_FUNCTION_CALL, "count_args:u16"), \
    OP_ITEM(OP_DIRECT_FUNCTION_CALL, "scope_depth:u8, function_idx:u16, count_args:u16"), \
    \
    OP_ITEM(OP_ENTER_SCOPE, "scope_idx:u16"), \
    OP_ITEM(OP_LEAVE_SCOPE, ""), \
    \
    OP_ITEM(OP_POP_STACK_TOP, ""), \
    OP_ITEM(OP_POP_STACK_TOP_N, "count:u16"), \
    \
    OP_ITEM(OP_PUSH_UNDFINED, ""), \
    OP_ITEM(OP_PUSH_NULL, ""), \
    OP_ITEM(OP_PUSH_TRUE, ""), \
    OP_ITEM(OP_PUSH_FALSE, ""), \
    OP_ITEM(OP_PUSH_ID_BY_NAME, "name_idx:u32"), /* 根据名字逐层查找标识符，压栈 */ \
    OP_ITEM(OP_PUSH_ID_LOCAL_SCOPE, "var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_LOCAL_ARGUMENT, "argument_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_ARGUMENT, "scope_depth:u8, argument_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_GLOBAL, "var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_SCOPE, "scope_depth:u8, var_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_GLOBAL_BY_NAME, ""), \
    OP_ITEM(OP_PUSH_ID_LOCAL_FUNCTION, "function_idx:u16"), \
    OP_ITEM(OP_PUSH_ID_PARENT_FUNCTION, "scope_depth:u8, function_idx:u16"), \
    OP_ITEM(OP_PUSH_STRING, "string_idx:u32"), \
    OP_ITEM(OP_PUSH_CHAR, "char:u16"), \
    OP_ITEM(OP_PUSH_REGEXP, "string_idx:u32"), \
    OP_ITEM(OP_PUSH_INT32, "int_number:i32"), \
    OP_ITEM(OP_PUSH_DOUBLE, "value_idx:u32"), \
    OP_ITEM(OP_PUSH_FUNCTION_EXPR, "scope_depth:u8, function_idx:u16"), \
    \
    OP_ITEM(OP_PUSH_MEMBER_INDEX, ""), \
    OP_ITEM(OP_PUSH_MEMBER_INDEX_INT, "index:u32"), \
    OP_ITEM(OP_PUSH_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_PUSH_MEMBER_DOT_OPTIONAL, "property_string_idx:u32"), \
    \
    OP_ITEM(OP_PUSH_MEMBER_INDEX_NO_POP, ""), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_INDEX, ""), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_INDEX_INT, "index:u32"), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_PUSH_THIS_MEMBER_DOT_OPTIONAL, "property_string_idx:u32"), \
    \
    OP_ITEM(OP_ASSIGN_IDENTIFIER, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_ASSIGN_LOCAL_ARGUMENT, "argument_idx:u16"), \
    OP_ITEM(OP_ASSIGN_MEMBER_INDEX, ""), \
    OP_ITEM(OP_ASSIGN_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_ASSIGN_VALUE_AHEAD_MEMBER_INDEX, ""), \
    OP_ITEM(OP_ASSIGN_VALUE_AHEAD_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_ID_PRE, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_INCREMENT_ID_POST, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_DECREMENT_ID_PRE, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_DECREMENT_ID_POST, "identifier_storage_type:varStorageType, scope_depth:u8, var_index:u16"), \
    OP_ITEM(OP_INCREMENT_MEMBER_DOT_PRE, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_MEMBER_DOT_POST, "property_string_idx:u32"), \
    OP_ITEM(OP_DECREMENT_MEMBER_DOT_PRE, "property_string_idx:u32"), \
    OP_ITEM(OP_DECREMENT_MEMBER_DOT_POST, "property_string_idx:u32"), \
    OP_ITEM(OP_INCREMENT_MEMBER_INDEX_PRE, ""), \
    OP_ITEM(OP_INCREMENT_MEMBER_INDEX_POST, ""), \
    OP_ITEM(OP_DECREMENT_MEMBER_INDEX_PRE, ""), \
    OP_ITEM(OP_DECREMENT_MEMBER_INDEX_POST, ""), \
    \
    OP_ITEM(OP_ADD, ""), \
    OP_ITEM(OP_SUB, ""), \
    OP_ITEM(OP_MUL, ""), \
    OP_ITEM(OP_DIV, ""), \
    OP_ITEM(OP_MOD, ""), \
    OP_ITEM(OP_EXP, ""), \
    \
    OP_ITEM(OP_BIT_OR, ""), \
    OP_ITEM(OP_BIT_XOR, ""), \
    OP_ITEM(OP_BIT_AND, ""), \
    OP_ITEM(OP_LEFT_SHIFT, ""), \
    OP_ITEM(OP_RIGHT_SHIFT, ""), \
    OP_ITEM(OP_UNSIGNED_RIGHT_SHIFT, ""), \
    OP_ITEM(OP_INEQUAL_STRICT, ""), \
    OP_ITEM(OP_INEQUAL, ""), \
    OP_ITEM(OP_EQUAL_STRICT, ""), \
    OP_ITEM(OP_EQUAL, ""), \
    OP_ITEM(OP_LESS_THAN, ""), \
    OP_ITEM(OP_LESS_EQUAL_THAN, ""), \
    OP_ITEM(OP_GREATER_THAN, ""), \
    OP_ITEM(OP_GREATER_EQUAL_THAN, ""), \
    OP_ITEM(OP_IN, ""), \
    OP_ITEM(OP_INSTANCE_OF, ""), \
    \
    OP_ITEM(OP_PREFIX_NEGATE, ""), \
    OP_ITEM(OP_PREFIX_PLUS, ""), \
    \
    OP_ITEM(OP_LOGICAL_NOT, ""), \
    OP_ITEM(OP_BIT_NOT, ""), \
    OP_ITEM(OP_DELETE_MEMBER_DOT, "property_string_idx:u32"), \
    OP_ITEM(OP_DELETE_MEMBER_INDEX, ""), \
    OP_ITEM(OP_DELETE_ID_BY_NAME, ""), \
    OP_ITEM(OP_DELETE_ID_GLOBAL, "var_index:u16"), \
    OP_ITEM(OP_DELETE, ""), \
    OP_ITEM(OP_TYPEOF, ""), \
    OP_ITEM(OP_VOID, ""), \
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
    OP_ITEM(OP_ARRAY_CREATE, ""), \
    OP_ITEM(OP_ARRAY_SPREAD_VALUE, ""), \
    OP_ITEM(OP_ARRAY_PUSH_VALUE, ""), \
    OP_ITEM(OP_ARRAY_PUSH_UNDEFINED_VALUE, ""), \
    \
    /* 将数据赋值给 Array */\
    OP_ITEM(OP_ARRAY_ASSING_CREATE, ""), \
    OP_ITEM(OP_ARRAY_ASSIGN_REST_VALUE, ""), \
    OP_ITEM(OP_ARRAY_ASSIGN_PUSH_VALUE, ""), \
    OP_ITEM(OP_ARRAY_ASSIGN_PUSH_UNDEFINED_VALUE, ""), \
    \
    OP_ITEM(OP_ITERATOR_IN_CREATE, ""), \
    OP_ITEM(OP_ITERATOR_OF_CREATE, ""), \
    OP_ITEM(OP_ITERATOR_NEXT_KEY, "address_end_loop:u32"), \
    OP_ITEM(OP_ITERATOR_NEXT_VALUE, "address_end_loop:u32"), \
    \
    OP_ITEM(OP_SPREAD_ARGS, ""), \
    OP_ITEM(OP_REST_PARAMETER, "index:u16"), \
    \
    OP_ITEM(OP_TRY_START, "address_catch:u32, address_finally:u32"), \
    OP_ITEM(OP_PUSH_EXCEPTION, ""), \
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
    JDT_CHAR, // 单个字符的 String
    JDT_INT32,

    // 下面的类型都要参与到 gc 中
    JDT_NUMBER,
    JDT_SYMBOL,
    JDT_STRING,

    // Iterator 仅仅在虚拟机内部的 OP_ITERATOR_NEXT_XX 等几个指令使用
    JDT_ITERATOR,

    // Object 开始
    JDT_OBJECT,
    JDT_REGEX,
    JDT_ARRAY,
    JDT_OBJ_BOOL,
    JDT_OBJ_NUMBER,
    JDT_OBJ_STRING,
    JDT_OBJ_GLOBAL_THIS,

    // 函数 开始
    JDT_FUNCTION,
    JDT_NATIVE_FUNCTION,
    JDT_LIB_OBJECT,
};

const char *jsDataTypeToString(JsDataType type);

using VMAddress = uint32_t;
const VMAddress addressInvalid = (VMAddress)-1;

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
    JsValue(JsDataType type, uint32_t objIdx) { *(uint64_t *)this = 0; this->type = type; value.index = objIdx; }

    inline bool isValid() const { return type > JDT_NOT_INITIALIZED; }
    inline bool equal(const JsValue &other) const { return *(uint64_t *)this == *(uint64_t *)&other; }
};

inline bool operator==(const JsValue &a, const JsValue &b) {
    return a.type == b.type && a.isInResourcePool == b.isInResourcePool && a.value.index == b.value.index;
}

inline JsValue makeJsValueOfStringInResourcePool(uint16_t poolIndex, uint16_t index) {
    JsValue r(JDT_STRING, makeResourceIndex(poolIndex, index));
    r.isInResourcePool = true;
    return r;
}

inline JsValue makeJsValueOfNumberInResourcePool(uint16_t poolIndex, uint16_t index) {
    JsValue r(JDT_NUMBER, makeResourceIndex(poolIndex, index));
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
    JsSymbol() { referIdx = 0; nextFreeIdx = 0; }
    JsSymbol(const SizedString &name) { referIdx = 0; nextFreeIdx = 0; this->name.assign((cstr_t)name.data, name.len); }

    string toString() const;

    int8_t                      referIdx; // 用于资源回收时所用
    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    string                      name;
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

    JsJoinedString() {
        isStringIdxInResourcePool = false;
        isNextStringIdxInResourcePool = false;
        stringIdx = 0;
        nextStringIdx = 0;
        len = 0;
    }

    JsJoinedString(const JsValue &s1, const JsValue &s2) {
        stringIdx = s1.value.index;
        isStringIdxInResourcePool = s1.isInResourcePool;

        nextStringIdx = s2.value.index;
        isNextStringIdxInResourcePool = s2.isInResourcePool;
    }
};

/**
 * 存储 string 类型的值
 */
struct JsString {
    JsString() { referIdx = 0; nextFreeIdx = 0; isJoinedString = false; }
    JsString(const SizedString &str) { referIdx = 0; nextFreeIdx = 0; isJoinedString = false; value.str = str; }
    JsString(const JsJoinedString &joinedString) { referIdx = 0; nextFreeIdx = 0; isJoinedString = true; value.joinedString = joinedString; }

    uint32_t                    nextFreeIdx; // 下一个空闲的索引位置
    int8_t                      referIdx; // 用于资源回收时所用
    bool                        isJoinedString;
    uint8_t                     reserved[2];

    union Value {
        Value() { }
        JsJoinedString          joinedString;
        SizedString             str;
    } value;
};

static_assert(sizeof(JsString) == 24, "JsString should be 24 bytes long.");

/**
 * JsProperty 定义了 Object 的基本属性
 */
struct JsProperty {
    JsValue                     value; // 属性值，或者 getter
    JsValue                     setter;
    int8_t                      isGSetter; // is getter/setter 中的一个有效？
    int8_t                      isConfigurable;
    int8_t                      isEnumerable;
    int8_t                      isWritable;

    JsProperty(const JsValue &value, int8_t isGSetter = false, int8_t isConfigurable = true, int8_t isEnumerable = true, int8_t isWritable = true) : value(value), isGSetter(isGSetter), isConfigurable(isConfigurable), isEnumerable(isEnumerable), isWritable(isWritable) {
    }

    JsProperty() : JsProperty(JsValue(JDT_UNDEFINED, 0)) { }

    bool merge(const JsProperty &src);

    JsProperty defineProperty() const {
        JsProperty ret = *this;
        if (value.type == JDT_NOT_INITIALIZED) ret.value = JsValue(JDT_UNDEFINED, 0);
        if (isGSetter == -1) ret.isGSetter = false;
        if (isConfigurable == -1) ret.isConfigurable = false;
        if (isEnumerable == -1) ret.isEnumerable = false;
        if (isWritable == -1) ret.isWritable = false;
        return ret;
    }
};


enum JsObjectValueIndex {
    JS_OBJ_GLOBAL_THIS_IDX                  = 1,
    JS_OBJ_PROTOTYPE_IDX_BOOL               = 2,
    JS_OBJ_PROTOTYPE_IDX_NUMBER             = 4,
    JS_OBJ_PROTOTYPE_IDX_STRING             = 6,
    JS_OBJ_PROTOTYPE_IDX_SYMBOL             = 8,
    JS_OBJ_PROTOTYPE_IDX_OBJECT             = 10,
    JS_OBJ_PROTOTYPE_IDX_REGEXP             = 12,
    JS_OBJ_PROTOTYPE_IDX_ARRAY              = 14,
    JS_OBJ_PROTOTYPE_IDX_FUNCTION           = 16,
};

enum JsNumberValueIndex {
    JS_NUMBER_IDX_NAN               = 1,
    JS_NUMBER_IDX_INF               = 2,
};

const JsValue jsValueGlobalThis = JsValue(JDT_OBJ_GLOBAL_THIS, JS_OBJ_GLOBAL_THIS_IDX);
const JsValue jsValuePrototypeBool = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_BOOL);
const JsValue jsValuePrototypeNumber = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_NUMBER);
const JsValue jsValuePrototypeString = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_STRING);
const JsValue jsValuePrototypeSymbol = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_SYMBOL);
const JsValue jsValuePrototypeObject = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_OBJECT);
const JsValue jsValuePrototypeRegExp = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_REGEXP);
const JsValue jsValuePrototypeArray = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_ARRAY);
const JsValue jsValuePrototypeFunction = JsValue(JDT_LIB_OBJECT, JS_OBJ_PROTOTYPE_IDX_FUNCTION);

const JsValue jsValueNotInitialized = JsValue();
const JsValue jsValueNull = JsValue(JDT_NULL, 0);
const JsValue jsValueUndefined = JsValue(JDT_UNDEFINED, 0);
const JsValue jsValueTrue = JsValue(JDT_BOOL, true);
const JsValue jsValueFalse = JsValue(JDT_BOOL, false);
const JsValue jsValueNaN = JsValue(JDT_NUMBER, JS_NUMBER_IDX_NAN);
const JsValue jsValueInf = JsValue(JDT_NUMBER, JS_NUMBER_IDX_INF);

using VecJsValues = std::vector<JsValue>;
using VecJsDoubles = std::vector<JsDouble>;
using VecJsStrings = std::vector<JsString>;
using VecJsSymbols = std::vector<JsSymbol>;
using VecJsObjects = std::vector<IJsObject *>;
using VecJsProperties = std::vector<JsProperty>;
using DequeJsProperties = std::deque<JsProperty>;
using DequeJsValue = std::deque<JsValue>;

bool decodeBytecode(uint8_t *bytecode, int lenBytecode, BinaryOutputStream &stream);

#endif /* VirtualMachineTypes_hpp */
