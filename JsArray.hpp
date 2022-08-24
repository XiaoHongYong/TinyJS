//
//  JsArray.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#ifndef JsArray_hpp
#define JsArray_hpp

#include "IJsObject.hpp"


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

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) override;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) override;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) override;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) override;

    virtual JsValue getSetterByName(VMContext *ctx, const SizedString &prop) override;
    virtual JsValue getSetterByIndex(VMContext *ctx, uint32_t index) override;
    virtual JsValue getSetterBySymbol(VMContext *ctx, uint32_t index) override;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) override;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) override;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual IJsObject *clone() override;

    void push(VMContext *ctx, const JsValue &value);
    void push(VMContext *ctx, const JsValue *first, uint32_t count);

    void toString(VMContext *ctx, const JsValue &thiz, BinaryOutputStream &stream);
    
    /**
     * 采用按 Block 存储所有的元素，平均每一个 Block 约 2 ** 14 = 16384 个元素
     * 整个数组采用 block 数组存储。查找位置时，先根据 index 使用二分查找到 block，再直接定位具体的元素
     */
    struct Block {
        uint32_t                index; // 此块的起始 index
        DequeJsProperties       items; // items 的数量 >= setters 的数量
        DequeJsValue            *setters; // setters 一般不会存在.

        Block() { index = 0; setters = nullptr; }
        ~Block() { if (setters) delete setters; }
    };
    using VecBlocks = vector<Block *>;
    using VecBlocksIterator = VecBlocks::iterator;

protected:
    void _newObject();

    Block *findBlock(uint32_t index);
    Block *findToModifyBlock(uint32_t index);

    void reserveSize(uint32_t length);

protected:

    VecBlocks                   _blocks;
    uint32_t                    _length;
    bool                        _needGC;
    Block                       *_firstBlock;
    DequeJsProperties           *_firstBlockItems;
    JsObject                    *_obj;

};

#endif /* JsArray_hpp */
