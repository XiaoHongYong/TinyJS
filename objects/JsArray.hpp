//
//  JsArray.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#ifndef JsArray_hpp
#define JsArray_hpp

#include "JsObject.hpp"


/**
 * JsArray 对应于 JavaScript 中的 Array。
 * 小于 ARRAY_RESERVE_MAX_SIZE 个元素内的对象，按照每个 block ARRAY_BLOCK_SIZE 个对象来管理
 * 超过的，则使用就近原则，一个元素靠近哪个 block 就归其管理
 * block 采用 deque 来存储，方便添加、修改，同时支持二分查找.
 */
class JsArray : public IJsObject {
public:
    JsArray(uint32_t length = 0);
    ~JsArray();

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, int8_t configurable = -1, int8_t writable = -1) override;
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsObject *clone() override;
    virtual bool isOfIterable() override { return true; }
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    void push(VMContext *ctx, const JsValue &value);
    void push(VMContext *ctx, const JsValue *first, uint32_t count);

    void toString(VMContext *ctx, const JsValue &thiz, BinaryOutputStream &stream);

    void setLength(uint32_t length);

    /**
     * 采用按 Block 存储所有的元素，平均每一个 Block 约 2 ** 14 = 16384 个元素
     * 整个数组采用 block 数组存储。查找位置时，先根据 index 使用二分查找到 block，再直接定位具体的元素
     */
    struct Block {
        uint32_t                index; // 此块的起始 index
        DequeJsProperties       items; // items 的数量 >= setters 的数量

        Block() { index = 0; }
        ~Block() { }
    };
    using VecBlocks = vector<Block *>;
    using VecBlocksIterator = VecBlocks::iterator;

protected:
    void _newObject(VMContext *ctx);

    Block *findBlock(uint32_t index);
    Block *findToModifyBlock(uint32_t index);

    void reserveSize(uint32_t length);

    friend class JsArrayIterator;

protected:

    VecBlocks                   _blocks;
    uint32_t                    _length;
    Block                       *_firstBlock;
    DequeJsProperties           *_firstBlockItems;
    JsObject                    *_obj;

};

#endif /* JsArray_hpp */
