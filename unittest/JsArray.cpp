//
//  JsArray.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/1.
//

#include "../JsArray.hpp"


#if UNIT_TEST

#include "../../Utils/unittest.h"

const uint32_t ARRAY_BLOCK_SIZE = 1024 * 32; // 32768
const uint32_t ARRAY_RESERVE_MAX_SIZE = ARRAY_BLOCK_SIZE * 10; // 327680 个
const uint32_t ARRAY_MAX_INDEX = 4294967294; // 2 ** 32 - 2


class JsArrayTestable : public JsArray {
public:
    uint32_t countBlocks() const { return (uint32_t)_blocks.size(); }
    JsArray::Block *getBlock(int index) { return _blocks[index]; }
    uint32_t length() const { return _length; }
    uint32_t lengthInBlocks() { auto b = _blocks.back(); return b->index + (uint32_t)b->items.size(); }

};

TEST(JsArray, findToModifyBlock) {
    // 测试目标：需要覆盖到 findToModifyBlock 的所有分支

    JsArrayTestable a;

    ASSERT_EQ(a.countBlocks(), 1);

    uint32_t length;;
    VMContext *ctx = nullptr;
    JsArray::Block *b;
    JsValue thiz, value;
    JsValue ret;

    // 在第一个 block 内
    for (int i = 10; i < 20; i++) {
        value = JsValue(JDT_INT32, i);
        a.setByIndex(ctx, thiz, i, value);
        ASSERT_EQ(a.countBlocks(), 1);
        ASSERT_EQ(a.length(), i + 1);
        ASSERT_EQ(a.lengthInBlocks(), i + 1);
        b = a.getBlock(0);
        ASSERT_EQ(b->index, 0);
        ASSERT_EQ(b->items.size(), i + 1);

        ret = a.getByIndex(ctx, thiz, i);
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, CStrPrintf("%d", i).c_str());
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, SS_LENGTH);
        ASSERT_EQ(ret, JsValue(JDT_INT32, i + 1));
    }

    // 测试不存在的值
    ret = a.getByIndex(ctx, thiz, 30);
    ASSERT_EQ(ret, jsValueUndefined);

    // 添加在第 2 block, 按照 ARRAY_BLOCK_SIZE 分段
    int startIndex = ARRAY_BLOCK_SIZE * 3;
    for (int i = startIndex + 10; i < startIndex + 20; i++) {
        value = JsValue(JDT_INT32, i);
        a.setByIndex(ctx, thiz, i, value);
        ASSERT_EQ(a.countBlocks(), 2);
        ASSERT_EQ(a.length(), i + 1);
        ASSERT_EQ(a.lengthInBlocks(), i + 1);
        b = a.getBlock(1);
        ASSERT_EQ(b->index, startIndex);
        ASSERT_EQ(b->items.size(), i + 1 - startIndex);

        ret = a.getByIndex(ctx, thiz, i);
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, CStrPrintf("%d", i).c_str());
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, SS_LENGTH);
        ASSERT_EQ(ret, JsValue(JDT_INT32, i + 1));
    }
    length = a.length();

    // 插入在第 2 block, 按照 ARRAY_BLOCK_SIZE 分段
    startIndex = ARRAY_BLOCK_SIZE * 2;
    for (int i = startIndex + ARRAY_BLOCK_SIZE / 2; i < startIndex + ARRAY_BLOCK_SIZE / 2 + 20; i++) {
        value = JsValue(JDT_INT32, i);
        a.setByIndex(ctx, thiz, i, value);
        ASSERT_EQ(a.countBlocks(), 3);
        ASSERT_EQ(a.length(), length);
        ASSERT_EQ(a.lengthInBlocks(), length);
        b = a.getBlock(1);
        ASSERT_EQ(b->index, startIndex);
        ASSERT_EQ(b->items.size(), i + 1 - startIndex);

        ret = a.getByIndex(ctx, thiz, i);
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, CStrPrintf("%d", i).c_str());
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, SS_LENGTH);
        ASSERT_EQ(ret, JsValue(JDT_INT32, length));
    }

    // 添加 block，位置超出了 ARRAY_RESERVE_MAX_SIZE，没有按照 ARRAY_BLOCK_SIZE 分段
    startIndex = ARRAY_RESERVE_MAX_SIZE + ARRAY_BLOCK_SIZE * 10 + 10;
    for (int i = startIndex; i < startIndex + 20; i++) {
        value = JsValue(JDT_INT32, i);
        a.setByIndex(ctx, thiz, i, value);
        ASSERT_EQ(a.countBlocks(), 4);
        ASSERT_EQ(a.length(), i + 1);
        ASSERT_EQ(a.lengthInBlocks(), i + 1);
        b = a.getBlock(3);
        ASSERT_EQ(b->index, startIndex);
        ASSERT_EQ(b->items.size(), i + 1 - startIndex);

        ret = a.getByIndex(ctx, thiz, i);
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, CStrPrintf("%d", i).c_str());
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, SS_LENGTH);
        ASSERT_EQ(ret, JsValue(JDT_INT32, i + 1));
    }

    length = a.length();

    // 在之间插入新的 block
    startIndex = ARRAY_RESERVE_MAX_SIZE + ARRAY_BLOCK_SIZE * 9 + 10;
    for (int i = startIndex; i < startIndex + 20; i++) {
        value = JsValue(JDT_INT32, i);
        a.setByIndex(ctx, thiz, i, value);
        ASSERT_EQ(a.countBlocks(), 5);
        ASSERT_EQ(a.length(), length);
        ASSERT_EQ(a.lengthInBlocks(), length);
        b = a.getBlock(3);
        ASSERT_EQ(b->index, startIndex);
        ASSERT_EQ(b->items.size(), i + 1 - startIndex);

        ret = a.getByIndex(ctx, thiz, i);
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, CStrPrintf("%d", i).c_str());
        ASSERT_EQ(ret, value);
    }

    // 添加到最后一个大的 block
    startIndex = ARRAY_RESERVE_MAX_SIZE + ARRAY_BLOCK_SIZE * 9 + ARRAY_BLOCK_SIZE / 2 + 30;
    for (int i = startIndex; i < startIndex + 20; i++) {
        value = JsValue(JDT_INT32, i);
        a.setByIndex(ctx, thiz, i, value);
        ASSERT_EQ(a.countBlocks(), 5);
        ASSERT_EQ(a.length(), length);
        ASSERT_EQ(a.lengthInBlocks(), length);
        b = a.getBlock(4);
        ASSERT_EQ(b->index, startIndex);
        ASSERT_EQ(b->items.size(), length - startIndex);

        ret = a.getByIndex(ctx, thiz, i);
        ASSERT_EQ(ret, value);

        ret = a.getByName(ctx, thiz, CStrPrintf("%d", i).c_str());
        ASSERT_EQ(ret, value);
    }
}

#endif

