//
//  TimerTasks.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/9.
//

#include "TimerTasks.hpp"
#include "utils/os.h"
#include "VirtualMachine.hpp"


TimerTasks::Timer::Timer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat, int timerId) : ctx(ctx), callback(callback), duration(duration), repeat(repeat), timerId(timerId)
{
    startTime = getTickCount() + duration;
}

TimerTasks::TimerTasks() {
    _timerIdNext = 1;
}

int TimerTasks::registerTimer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat) {
    auto id = _timerIdNext++;
    insertTimer(_timers, _timers.begin(), Timer(ctx, callback, duration, repeat, id));
    return id;
}

void TimerTasks::unregisterTimer(int timerId) {
    for (auto it = _timers.begin(); it != _timers.end(); ++it) {
        if ((*it).timerId == timerId) {
            _timers.erase(it);
            break;
        }
    }
}

TimerTasks::ListTimersIterator TimerTasks::insertTimer(ListTimers &timers, ListTimersIterator begin, const Timer &item) {
    ListTimersIterator end = timers.end();
    for (; begin != end; ++begin) {
        if (item.startTime < (*begin).startTime) {
            break;
        }
    }

    return timers.insert(begin, item);
}

bool TimerTasks::run() {
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

void TimerTasks::markReferIdx(VMRuntime *rt) {
    for (auto &timer : _timers) {
        rt->markReferIdx(timer.callback);
    }
}
