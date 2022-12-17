//
//  Arguments.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#ifndef Arguments_hpp
#define Arguments_hpp

#include "VirtualMachineTypes.hpp"
#include "strings/CommonString.hpp"


class Arguments {
public:
    Arguments() { data = nullptr; count = 0; capacity = 0; needFree = false; }
    Arguments(const Arguments &other);
    Arguments(JsValue *args, uint32_t count) : data(args), count(count), capacity(count) { needFree = false; }
    ~Arguments();

    void free();

    Arguments &operator = (const Arguments &other);
    JsValue &operator[](uint32_t n) const { assert(n < capacity); return data[n]; }
    JsValue getAt(uint32_t n, const JsValue &defValue = jsValueUndefined) const { return n < capacity ? data[n] : defValue; }
    int32_t getIntAt(VMContext *ctx, uint32_t index, int32_t defVal = 0) const;
    int64_t getInt64At(VMContext *ctx, uint32_t index, int64_t defVal = 0) const;
    double getDoubleAt(VMContext *ctx, uint32_t index, double defVal = 0) const;
    LockedSizedStringWrapper getStringAt(VMContext *ctx, uint32_t index, const SizedString &defVal = sizedStringEmpty) const;

    void copy(const Arguments &other, uint32_t minSize = 0);

    JsValue                     *data;
    uint32_t                    count;
    uint32_t                    capacity;
    bool                        needFree;
};

class ArgumentsX : public Arguments {
protected:
    ArgumentsX(uint32_t count) {
        this->count = count;
        data = args;
        needFree = false;
        capacity = sizeof(args);
    }

public:
    ArgumentsX(const JsValue &one) : ArgumentsX(1) {
        args[0] = one;
    }

    ArgumentsX(const JsValue &one, const JsValue &two) : ArgumentsX(2) {
        args[0] = one;
        args[1] = two;
    }

    ArgumentsX(const JsValue &one, const JsValue &two, const JsValue &three) : ArgumentsX(3) {
        args[0] = one;
        args[1] = two;
        args[2] = three;
    }

    ArgumentsX(const JsValue &one, const JsValue &two, const JsValue &three, const JsValue &four) : ArgumentsX(3) {
        args[0] = one;
        args[1] = two;
        args[2] = three;
        args[3] = four;
    }

    JsValue                     args[4];

};

#endif /* Arguments_hpp */
