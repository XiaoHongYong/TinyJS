//
//  TimerTasks.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/9.
//

#ifndef TimerTasks_hpp
#define TimerTasks_hpp


#include "VirtualMachineTypes.hpp"

class TimerTasks {
public:
    struct Timer {
        int64_t                 startTime;
        JsValue                 callback;
        VMContext               *ctx;
        int32_t                 duration;
        int32_t                 timerId;
        bool                    repeat;

        Timer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat, int timerId);
    };

    using ListTimers = list<Timer>;
    using ListTimersIterator = ListTimers::iterator;

public:
    TimerTasks();

    int registerTimer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat);
    void unregisterTimer(int timerId);

    static ListTimersIterator insertTimer(ListTimers &timers, ListTimersIterator begin, const Timer &item);

    bool run();
    void markReferIdx(VMRuntime *rt);

protected:
    ListTimers              _timers;
    int                     _timerIdNext;

};

#endif /* TimerTasks_hpp */
