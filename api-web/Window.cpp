//
//  Console.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "WebAPI.hpp"
#include "interpreter/VirtualMachineTypes.hpp"


class WindowContextTask : public VmTask {
public:
    struct Timer {
        int64_t                 startTime;
        JsValue                 callback;
        VMContext               *ctx;
        int32_t                 duration;
        int32_t                 timerId;
        bool                    repeat;

        Timer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat, int timerId) : ctx(ctx), callback(callback), duration(duration), repeat(repeat), timerId(timerId)
        {
            startTime = getTickCount() + duration;
        }
    };

    using ListTimers = list<Timer>;
    using ListTimersIterator = ListTimers::iterator;

public:
    WindowContextTask() {
        _timerIdNext = 1;
    }

    int registerTimer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat) {
        auto id = _timerIdNext++;
        insertTimer(_timers, _timers.begin(), Timer(ctx, callback, duration, repeat, id));
        return id;
    }

    int allocTimerId() { return _timerIdNext++; }

    void unregisterTimer(int timerId) {
        for (auto it = _timers.begin(); it != _timers.end(); ++it) {
            if ((*it).timerId == timerId) {
                _timers.erase(it);
                break;
            }
        }
    }

    static ListTimersIterator insertTimer(ListTimers &timers, ListTimersIterator begin, const Timer &item) {
        ListTimersIterator end = timers.end();
        for (; begin != end; ++begin) {
            if (item.startTime < (*begin).startTime) {
                break;
            }
        }

        return timers.insert(begin, item);
    }

    virtual bool run() override {
        //
        // 为了避免在执行过程中，注册/删除 timer，导致 iterator 失效，
        // 将要执行的 timer 都预先剥离出来.
        //

        int64_t now = getTickCount();
        ListTimers toRuns, toInserts;

        for (auto it = _timers.begin(); it != _timers.end();) {
            auto &timer = *it;
            if (timer.startTime <= now) {
                toRuns.push_back(timer);
                if (timer.repeat) {
                    timer.startTime = now + timer.duration;
                    insertTimer(toInserts, toInserts.begin(), timer);
                }
                it = _timers.erase(it);
            } else {
                break;
            }
        }

        auto insertBegin = _timers.begin();
        for (auto &timer : toInserts) {
            // toInserts 已经拍好了序，所以只能在上次插入的之后.
            insertBegin = insertTimer(_timers, insertBegin, timer);
        }

        for (auto &timer : toRuns) {
            if (timer.callback.isFunction()) {
                Arguments args;
                timer.ctx->vm->callMember(timer.ctx, jsValueGlobalThis, timer.callback, args);
            } else {
                // eval as string.
                assert(0);
                // timer.ctx->vm->eval();
            }
        }

        return !_timers.empty();
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        for (auto &timer : _timers) {
            rt->markReferIdx(timer.callback);
        }
    }

protected:
    ListTimers              _timers;
    int                     _timerIdNext;

};

using WindowContextTaskPtr = shared_ptr<WindowContextTask>;

WindowContextTaskPtr _windowCtxTaskPtr;

// https://developer.mozilla.org/en-US/docs/Web/API/Window
static void windowConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->throwException(JE_TYPE_ERROR, "Illegal constructor");
}

static JsLibProperty windowFunctions[] = {
    { "name", nullptr, "Window" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static void windowPrototypeAlert(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
}

void window_setInterval(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->throwException(JE_TYPE_ERROR, "Failed to execute 'setInterval' on 'Window': 1 argument required, but only 0 present.");
        return;
    }

    auto callback = args.getAt(0);
    auto duration = args.getIntAt(ctx, 1, 0);

    auto id = _windowCtxTaskPtr->registerTimer(ctx, callback, duration, true);
    ctx->retValue = JsValue(JDT_INT32, id);
}

void window_setTimeout(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->throwException(JE_TYPE_ERROR, "Failed to execute 'setTimeout' on 'Window': 1 argument required, but only 0 present.");
        return;
    }

    auto callback = args.getAt(0);
    auto duration = args.getIntAt(ctx, 1, 0);

    auto id = _windowCtxTaskPtr->registerTimer(ctx, callback, duration, false);
    ctx->retValue = JsValue(JDT_INT32, id);
}

void window_clearTimer(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        return;
    }

    auto id = args.getIntAt(ctx, 0, 0);
    _windowCtxTaskPtr->unregisterTimer(id);
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty globalFunctions[] = {
    { "alert", windowPrototypeAlert },
    { "setInterval", window_setInterval },
    { "setTimeout", window_setTimeout },
    { "clearInterval", window_clearTimer },
    { "clearTimeout", window_clearTimer },
};

static JsLibProperty windowPrototypeFunctions[] = { };

void registerWindow(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, windowPrototypeFunctions, CountOf(windowPrototypeFunctions));
    rt->objPrototypeWindow = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeWindow, prototypeObj);

    SET_PROTOTYPE(windowFunctions, jsValuePrototypeWindow);

    rt->setGlobalObject("Window",
        new JsLibObject(rt, windowFunctions, CountOf(windowFunctions), windowConstructor));

    _windowCtxTaskPtr = make_shared<WindowContextTask>();
    rt->vm->registerTask(_windowCtxTaskPtr);

    // 注册全局函数
    for (auto &item : globalFunctions) {
        item.name.setStable();
        auto f = rt->pushNativeFunction(item.function, item.name);
        rt->setGlobalValue((char *)item.name.data, f);
    }
}
