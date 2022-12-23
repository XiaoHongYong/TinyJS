//
//  Arguments.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#include "Arguments.hpp"
#include "VirtualMachine.hpp"


Arguments::Arguments(const Arguments &other) {
    if (other.needFree) {
        copy(other);
    } else {
        data = other.data;
        count = other.count;
        capacity = other.capacity;
    }
}

Arguments::~Arguments() {
    free();
}

void Arguments::free() {
    if (needFree) {
        delete [] data;
        needFree = false;
        data = nullptr;
    }

    count = 0;
    capacity = 0;
}

Arguments & Arguments::operator = (const Arguments &other) {
    if (other.needFree) {
        copy(other);
    } else {
        data = other.data;
        count = other.count;
        capacity = other.capacity;
    }

    return *this;
}

int32_t Arguments::getIntAt(VMContext *ctx, uint32_t index, int32_t defVal) const {
    if (index < capacity) {
        auto v = data[index];
        if (v.type == JDT_INT32) {
            return v.value.n32;
        }

        double d = ctx->runtime->toNumber(ctx, v);
        if (isnan(d)) {
            return 0;
        } else if (isinf(d)) {
            return 0x7FFFFFFF;
        } else {
            return (int32_t)d;
        }
    } else {
        return defVal;
    }
}

bool Arguments::getBoolAt(VMContext *ctx, uint32_t index, bool defVal) const {
    if (index < capacity) {
        auto v = data[index];
        return ctx->runtime->testTrue(v);
    } else {
        return defVal;
    }
}

int64_t Arguments::getInt64At(VMContext *ctx, uint32_t index, int64_t defVal) const {
    if (index < capacity) {
        auto v = data[index];
        if (v.type == JDT_INT32) {
            return v.value.n32;
        }

        double d = ctx->runtime->toNumber(ctx, v);
        if (isnan(d)) {
            return 0;
        } else if (isinf(d)) {
            return 0x7FFFFFFFFFFFFFFF;
        } else {
            return (int64_t)d;
        }
    } else {
        return defVal;
    }
}

double Arguments::getDoubleAt(VMContext *ctx, uint32_t index, double defVal) const {
    if (index < capacity) {
        auto v = data[index];
        if (v.type == JDT_INT32) {
            return v.value.n32;
        }

        return ctx->runtime->toNumber(ctx, v);
    } else {
        return defVal;
    }
}

LockedSizedStringWrapper Arguments::getStringAt(VMContext *ctx, uint32_t index, const SizedString &defVal) const {
    if (index < capacity) {
        auto v = data[index];

        return ctx->runtime->toSizedStringStrictly(ctx, v);
    } else {
        return defVal;
    }
}

void Arguments::copy(const Arguments &other, uint32_t minSize) {
    assert(!needFree);

    capacity = std::max(other.count, minSize);
    data = new JsValue[capacity];
    memcpy((void *)data, other.data, sizeof(data[0]) * other.count);
    count = other.count;
    needFree = true;

    // 将额外的位置赋值为 Undefined.
    while (minSize > count) {
        data[--minSize] = jsValueUndefined;
    }
}
