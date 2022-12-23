//
//  VMScope.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#ifndef VMScope_hpp
#define VMScope_hpp

#include "VMRuntime.hpp"
#include "Arguments.hpp"


/**
 * VMScope 用于存储当前作用域内的标识符信息.
 */
class VMScope {
public:
    VMScope(Scope *scopeDsc);
    virtual ~VMScope() { }

    void dump(BinaryOutputStream &stream);

    void free();

    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

    Scope                       *scopeDsc;

    // 作用域内的所有变量
    VecJsValues                 vars; // 当前作用域下的变量

    Arguments                   args;

    // 当使用 with 语句，在执行时会在 'withValue' 的 member 中查找标识符
    JsValue                     withValue;

};

class VMGlobalScope : public VMScope {
public:
    VMGlobalScope();
    VMGlobalScope(VMGlobalScope *other);

    JsValue get(VMContext *ctx, uint32_t index) const;
    JsError set(VMContext *ctx, uint32_t index, const JsValue &value);
    JsValue increase(VMContext *ctx, uint32_t index, int inc, bool isPost);
    bool remove(VMContext *ctx, uint32_t index);

    void set(const SizedString &name, const JsValue &val);

    uint32_t countVars() const { return (uint32_t)vars.size(); }

    void checkSpace();

protected:
    ResourcePool                _resourcePool;
    Function                    *_rootFunc;

};

#endif /* VMScope_hpp */
