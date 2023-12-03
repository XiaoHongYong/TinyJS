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

    virtual void setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) override;
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsValue *getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp = true) override;
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const StringView &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) override;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    virtual bool getLength(VMContext *ctx, int32_t &lengthOut) override { lengthOut = _length; return true; }

    std::tuple<JsValue, JsError, int> popFront(VMContext *ctx);
    void sort(VMContext *ctx, const JsValue &callback);

    void pushEmpty();
    JsError push(VMContext *ctx, const JsValue &value);
    JsError push(VMContext *ctx, const JsValue *first, uint32_t count);
    JsError extend(VMContext *ctx, const JsArray *other);
    JsError extend(VMContext *ctx, const JsValue &other, uint32_t depth);

    void reverse(VMContext *ctx);

    void toString(VMContext *ctx, const JsValue &thiz, BinaryOutputStream &stream);

    void setLength(uint32_t length);
    uint32_t length() const { return _length; }

    JsArray *cloneArrayOnly();

    void dump(VMContext *ctx, const JsValue &thiz, VecJsValues &values);

    /**
     * 采用按 Block 存储所有的元素，平均每一个 Block 约 2 ** 14 = 16384 个元素
     * 整个数组采用 block 数组存储。查找位置时，先根据 index 使用二分查找到 block，再直接定位具体的元素
     */
    struct Block {
        uint32_t                index; // 此块的起始 index
        bool                    hasPropDescriptor; // 此 block 中有无 Getter/Setter
        DequeJsProperties       items; // items 的数量 >= setters 的数量

        Block() { index = 0; hasPropDescriptor = false; }
        ~Block() { }
    };
    using VecBlocks = vector<Block *>;
    using VecBlocksIterator = VecBlocks::iterator;

protected:
    void _newObject(VMContext *ctx);

    Block *findBlock(uint32_t index);
    Block *findToModifyBlock(uint32_t index);

    std::pair<JsError, int> popFront(VMContext *ctx, Block *b);
    JsValue front(VMContext *ctx, Block *b);

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
