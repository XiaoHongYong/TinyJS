//
//  VirtualMachineTypes.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/6/5.
//

#include "VirtualMachineTypes.hpp"
#include "VirtualMachine.hpp"


#ifdef OP_ITEM
#undef OP_ITEM
#endif
#define OP_ITEM(a, b) {a, #a, b}

struct OpCodeDesc {
    OpCode code;
    const char *name;
    const char *params;
};

static OpCodeDesc OP_CODE_DESCRIPTIONS[] = {
    OP_CODE_DEFINES
};

const char *opCodeToString(OpCode code) {
    assert(code >= 0 && code < CountOf(OP_CODE_DESCRIPTIONS));
    auto &item = OP_CODE_DESCRIPTIONS[code];
    assert(item.code == code);
    return item.name;
}

const char *jsDataTypeToString(JsDataType type) {
    const char *NAMES[] = {
        "JDT_UNDEFINED",
        "JDT_NULL",
        "JDT_BOOL",
        "JDT_CHAR",
        "JDT_INT32",

        "JDT_NUMBER",
        "JDT_SYMBOL",
        "JDT_GETTER_SETTER",
        "JDT_STRING",

        "JDT_ITERATOR",

        "JDT_OBJECT",
        "JDT_ARRAY",
        "JDT_REGEX",
        "JDT_DATE",
        "JDT_PROMISE",
        "JDT_ARGUMENTS",
        "JDT_OBJ_X",

        "JDT_OBJ_BOOL",
        "JDT_OBJ_NUMBER",
        "JDT_OBJ_STRING",
        "JDT_OBJ_SYMBOL",
        "JDT_OBJ_GLOBAL_THIS",

        "JDT_FUNCTION",
        "JDT_BOUND_FUNCTION",
        "JDT_NATIVE_FUNCTION",
        "JDT_LIB_OBJECT",
    };

    static_assert(JDT_LIB_OBJECT + 1 == CountOf(NAMES), "Should be same.");
    assert(type <= CountOf(NAMES));
    return NAMES[type];
}

const char *readVarStorageType(uint8_t *&bytecode) {
    return varStorageTypeToString((VarStorageType)*bytecode++);
}

bool decodeParameters(uint8_t *&bytecode, uint8_t *end, const OpCodeDesc &desc, BinaryOutputStream &stream) {
    auto params = desc.params;

    while (!isEmptyString(params)) {
        auto p = strchr(params, ':');
        if (p == nullptr) {
            // assert(strcmp(params, "not_used"))
            printf("OpCode shoud not be used, or parameter description is INCORRECT: %s\n", desc.name);
            printf("%.*s\n", (int)100, stream.toLinkedString()->data + stream.toLinkedString()->len - 100);
            assert(0);
            return false;
        }
        p++;

        if (strncmp(p, "u8", 2) == 0) { stream.writeFormat(" %.*s%d", (int)(p - params), params, *bytecode++); p += 2; }
        else if (strncmp(p, "u16", 3) == 0) { stream.writeFormat(" %.*s%d", (int)(p - params), params, readUInt16(bytecode)); p += 3; }
        else if (strncmp(p, "u32", 3) == 0) { stream.writeFormat(" %.*s%d", (int)(p - params), params, readUInt32(bytecode)); p += 3; }
        else if (strncmp(p, "i32", 3) == 0) { stream.writeFormat(" %.*s%d", (int)(p - params), params, readInt32(bytecode)); p += 3; }
        else if (strncmp(p, "u64", 3) == 0) { stream.writeFormat(" %.*s%d", (int)(p - params), params, readUInt64(bytecode)); p += 3; }
        else if (strncmp(p, "i64", 3) == 0) { stream.writeFormat(" %.*s%d", (int)(p - params), params, readInt64(bytecode)); p += 3; }
        else if (strncmp(p, "varStorageType", 14) == 0) { stream.writeFormat(" %.*s%s", (int)(p - params), params, readVarStorageType(bytecode)); p += 14; }
        else { assert(0); }
        if (*p == ',') {
            p++;
            while (*p == ' ') {
                p++;
            }
        }
        params = p;
    }

    stream.writeUInt8('\n');

    return true;
}

bool decodeBytecode(uint8_t *bytecode, int lenBytecode, BinaryOutputStream &stream) {
    auto p = bytecode, end = bytecode + lenBytecode;
    while (p < end) {
        auto code = *p++;
        assert(code >= 0 && code < CountOf(OP_CODE_DESCRIPTIONS));
        auto item = OP_CODE_DESCRIPTIONS[code];
        assert(item.code == code);

        stream.write("  ");
        stream.write(item.name);
        decodeParameters(p, end, item, stream);
    }

    return true;
}

string JsSymbol::toString() const {
    return "Symbol(" + name + ")";
}
