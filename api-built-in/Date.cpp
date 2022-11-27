//
//  Date.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsObjectLazy.hpp"
#include "utils/DateTime.h"


static JsValue jsValuePrototypeDate;


class JsDate : public JsObjectLazy {
public:
    JsDate(int64_t time, bool isValid = true) : JsObjectLazy(nullptr, 0, jsValuePrototypeDate), time(time), isValid(isValid) {
        type = JDT_DATE;
    }

    virtual IJsObject *clone() override {
        return new JsDate(time);
    }

    int64_t                         time;
    bool                            isValid;

};

bool isNanInf(double d) {
    return isnan(d) || isinf(d);
}

void dateConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        auto obj = new JsDate(getTimeInMillisecond());
        ctx->retValue = runtime->pushObjectValue(obj);
        return;
    } else if (args.count == 1) {
        auto arg0 = args[0];
        if (arg0.type == JDT_STRING) {
            auto s = runtime->toSizedString(ctx, arg0);
            DateTime date;
            JsDate *obj;
            if (date.fromString((char *)s.data, s.len)) {
                obj = new JsDate(date.getTimeInMs());
            } else {
                obj = new JsDate(0, false);
            }
            ctx->retValue = runtime->pushObjectValue(obj);
        } else if (arg0.type == JDT_DATE) {
            auto from = (JsDate *)runtime->getObject(arg0);
            auto obj = new JsDate(from->time);
            ctx->retValue = runtime->pushObjectValue(obj);
        } else {
            auto t = args.getInt64At(ctx, 0);
            if (ctx->error == JE_OK) {
                auto obj = new JsDate(t);
                ctx->retValue = runtime->pushObjectValue(obj);
            }
        }
        return;
    }

    auto year = args.getDoubleAt(ctx, 0, 0);
    auto month = args.getDoubleAt(ctx, 1, 0);
    auto day = args.getDoubleAt(ctx, 2, 1);
    auto hours = args.getDoubleAt(ctx, 3, 0);
    auto minutes = args.getDoubleAt(ctx, 4, 0);
    auto seconds = args.getDoubleAt(ctx, 5, 0);
    auto ms = args.getDoubleAt(ctx, 6, 0);

    if (ctx->error != JE_OK) {
        return;
    }

    bool valid = true;
    int64_t time = 0;

    if (isNanInf(year) || isNanInf(month) || isNanInf(day) || isNanInf(hours) ||
        isNanInf(minutes) || isNanInf(seconds) || isNanInf(ms) || year >= 275760)
    {
        valid = false;
    }
    if (year >= 0 && year < 100) {
        year += 1900;
    }

    if (valid) {
        DateTime date((int)year, (int)month + 1, (int)day, (int)hours, (int)minutes, (int)seconds, (int)ms);
        time = date.getTimeInMs();
    }

    auto obj = new JsDate(time, valid);
    ctx->retValue = runtime->pushObjectValue(obj);
}

void date_now(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = new JsDate(getTimeInMillisecond());
    ctx->retValue = ctx->runtime->pushObjectValue(obj);
}

void date_parse(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    DateTime date;

    auto str = ctx->runtime->toSizedString(ctx, args.getAt(0));

    if (!date.fromString((cstr_t)str.data, str.len)) {
        ctx->retValue = jsValueNaN;
    } else {
        ctx->retValue = ctx->runtime->pushDoubleValue(date.getTimeInMs());
    }
}

void date_UTC(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto year = args.getIntAt(ctx, NAN);
    auto monthIndex = args.getIntAt(ctx, 0);
    auto day = args.getIntAt(ctx, 1);
    auto hour = args.getIntAt(ctx, 0);
    auto minute = args.getIntAt(ctx, 0);
    auto second = args.getIntAt(ctx, 0);
    auto millisecond = args.getIntAt(ctx, 0);

    if (isnan(year) || isnan(monthIndex) || isnan(day) || isnan(hour) || isnan(minute) || isnan(second) || isnan(millisecond)) {
        ctx->retValue = jsValueNaN;
        return;
    }

    if (year >= 0 && year <= 99) {
        year += 1900;
    }

    DateTime date(year, monthIndex + 1, day, hour, minute, second, millisecond, -1, false);

    ctx->retValue = ctx->runtime->pushDoubleValue(date.getTimeInMs());
}


static JsLibProperty dateFunctions[] = {
    { "name", nullptr, "Date" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "now", date_now },
    { "parse", date_parse },
    { "UTC", date_UTC },
};

bool checkDateType(VMContext *ctx, const JsValue &thiz) {
    if (thiz.type == JDT_DATE) {
        return true;
    }

    ctx->throwException(JE_TYPE_ERROR, "this is not a Date object");
    return false;
}

DateTime toDateTime(VMContext *ctx, const JsValue &thiz, bool &isValidOut, bool isLocalTime = true) {
    if (checkDateType(ctx, thiz)) {
        auto obj = (JsDate *)ctx->runtime->getObject(thiz);
        if (obj->isValid) {
            isValidOut = true;
            return DateTime(obj->time, isLocalTime);
        }
    }

    isValidOut = false;
    ctx->retValue = jsValueNaN;
    return DateTime((int64_t)0);
}

void datePrototype_getDate(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.day());
}

void datePrototype_getDay(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.dayOfWeek());
}

void datePrototype_getFullYear(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.year());
}

void datePrototype_getHours(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.hour());
}

void datePrototype_getMilliseconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.ms());
}

void datePrototype_getMinutes(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.minute());
}

void datePrototype_getMonth(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.month() - 1);
}

void datePrototype_getSeconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.second());
}

void datePrototype_getTime(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (!checkDateType(ctx, thiz)) {
        return;
    }

    auto obj = (JsDate *)ctx->runtime->getObject(thiz);
    ctx->retValue = ctx->runtime->pushDoubleValue(obj->time);
}

void datePrototype_getTimezoneOffset(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.timeZoneOffset());
}

void datePrototype_getUTCDate(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.day());
}

void datePrototype_getUTCDay(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.dayOfWeek());
}

void datePrototype_getUTCFullYear(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.year());
}

void datePrototype_getUTCHours(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.hour());
}

void datePrototype_getUTCMilliseconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.ms());
}

void datePrototype_getUTCMinutes(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.minute());
}

void datePrototype_getUTCMonth(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.month() - 1);
}

void datePrototype_getUTCSeconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool isValid;
    auto datetime = toDateTime(ctx, thiz, isValid, false);
    if (!isValid) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, datetime.second());
}

void datePrototype_setXX(VMContext *ctx, const JsValue &thiz, const Arguments &args, std::function<void(DateTime&, int)> callback, bool isLocalTime = true) {
    if (!checkDateType(ctx, thiz)) {
        return;
    }

    auto obj = (JsDate *)ctx->runtime->getObject(thiz);
    auto value = args.getDoubleAt(ctx, 0, NAN);

    if (isnan(value) || isinf(value)) {
        obj->isValid = false;
        ctx->retValue = jsValueNaN;
        return;
    }

    DateTime datetime(obj->time, isLocalTime);
    callback(datetime, (int)value);
    obj->time = datetime.getTimeInMs();

    ctx->retValue = ctx->runtime->pushDoubleValue(obj->time);
}

void datePrototype_setDate(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setDay(value);
    });
//    if (!checkDateType(ctx, thiz)) {
//        return;
//    }
//
//    auto obj = (JsDate *)ctx->runtime->getObject(thiz);
//    auto value = args.getDoubleAt(ctx, 0, NAN);
//
//    if (isnan(value) || isinf(value)) {
//        obj->isValid = false;
//        ctx->retValue = jsValueNaN;
//        return;
//    }
//
//    DateTime datetime(obj->time);
//    datetime.setDay(value);
//    obj->time = datetime.getTimeInMs();
//
//    ctx->retValue = ctx->runtime->pushDoubleValue(obj->time);
}

void datePrototype_setFullYear(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setYear(value);
    });
}

void datePrototype_setHours(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setHour(value);
    });
}

void datePrototype_setMilliseconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setMs(value);
    });
}

void datePrototype_setMinutes(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setMinute(value);
    });
}

void datePrototype_setMonth(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setMonth(value + 1);
    });
}

void datePrototype_setSeconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setSecond(value);
    });
}

void datePrototype_setTime(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (!checkDateType(ctx, thiz)) {
        return;
    }

    auto obj = (JsDate *)ctx->runtime->getObject(thiz);
    auto value = args.getDoubleAt(ctx, 0, NAN);

    if (isnan(value) || isinf(value)) {
        obj->isValid = false;
        ctx->retValue = jsValueNaN;
        return;
    }

    obj->time = (int64_t)value;

    ctx->retValue = ctx->runtime->pushDoubleValue(obj->time);
}

void datePrototype_setUTCDate(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setDay(value);
    }, false);
}

void datePrototype_setUTCFullYear(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setYear(value);
    }, false);
}

void datePrototype_setUTCHours(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setHour(value);
    }, false);
}

void datePrototype_setUTCMilliseconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setMs(value);
    }, false);
}

void datePrototype_setUTCMinutes(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setMinute(value);
    }, false);
}

void datePrototype_setUTCMonth(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setMonth(value + 1);
    }, false);
}

void datePrototype_setUTCSeconds(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_setXX(ctx, thiz, args, [](DateTime &datetime, int value) {
        datetime.setSecond(value);
    }, false);
}

void datePrototype_toXXString(VMContext *ctx, const JsValue &thiz, cstr_t format, bool isLocalTime) {
    if (!checkDateType(ctx, thiz)) {
        return;
    }

    auto obj = (JsDate *)ctx->runtime->getObject(thiz);

    if (!obj->isValid) {
        ctx->retValue = jsStringValueInvalidDate;
        return;
    }

    char buf[256];
    time_t time = (time_t)(obj->time / 1000);
    struct tm *tm = isLocalTime ? localtime(&time) : gmtime(&time);
    auto len = strftime(buf, sizeof(buf), format, tm);

    ctx->retValue = ctx->runtime->pushString(SizedString(buf, len));
}

void datePrototype_toDateString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%a %b %d %Y", true);
}

void datePrototype_toGMTString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%a, %d %b %Y %H:%M:%S GMT", false);
}

void datePrototype_toISOString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (!checkDateType(ctx, thiz)) {
        return;
    }

    auto obj = (JsDate *)ctx->runtime->getObject(thiz);
    int ms = int(obj->time % 1000);

    char format[128];
    snprintf(format, sizeof(format), "%s.%03dZ", "%Y-%m-%dT%H:%M:%S", ms);

    datePrototype_toXXString(ctx, thiz, format, false);
}

void datePrototype_toLocaleDateString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%Y-%m-%d", true);
}

void datePrototype_toLocaleString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%Y-%m-%d %H:%M:%S", true);
}

void datePrototype_toLocaleTimeString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%H:%M:%S", true);
}

void datePrototype_toString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%a %b %d %Y %H:%M:%S GMT+%z (%Z)", true);
}

void datePrototype_toTimeString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%H:%M:%S GMT+%z (%Z)", true);
}

void datePrototype_toUTCString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    datePrototype_toXXString(ctx, thiz, "%a, %d %b %Y %H:%M:%S GMT", false);
}

static JsLibProperty datePrototypeFunctions[] = {
    { "getDate", datePrototype_getDate },
    { "getDay", datePrototype_getDay },
    { "getFullYear", datePrototype_getFullYear },
    { "getHours", datePrototype_getHours },
    { "getMilliseconds", datePrototype_getMilliseconds },
    { "getMinutes", datePrototype_getMinutes },
    { "getMonth", datePrototype_getMonth },
    { "getSeconds", datePrototype_getSeconds },
    { "getTime", datePrototype_getTime },
    { "getTimezoneOffset", datePrototype_getTimezoneOffset },
    { "getUTCDate", datePrototype_getUTCDate },
    { "getUTCDay", datePrototype_getUTCDay },
    { "getUTCFullYear", datePrototype_getUTCFullYear },
    { "getUTCHours", datePrototype_getUTCHours },
    { "getUTCMilliseconds", datePrototype_getUTCMilliseconds },
    { "getUTCMinutes", datePrototype_getUTCMinutes },
    { "getUTCMonth", datePrototype_getUTCMonth },
    { "getUTCSeconds", datePrototype_getUTCSeconds },
    { "setDate", datePrototype_setDate },
    { "setFullYear", datePrototype_setFullYear },
    { "setHours", datePrototype_setHours },
    { "setMilliseconds", datePrototype_setMilliseconds },
    { "setMinutes", datePrototype_setMinutes },
    { "setMonth", datePrototype_setMonth },
    { "setSeconds", datePrototype_setSeconds },
    { "setTime", datePrototype_setTime },
    { "setUTCDate", datePrototype_setUTCDate },
    { "setUTCFullYear", datePrototype_setUTCFullYear },
    { "setUTCHours", datePrototype_setUTCHours },
    { "setUTCMilliseconds", datePrototype_setUTCMilliseconds },
    { "setUTCMinutes", datePrototype_setUTCMinutes },
    { "setUTCMonth", datePrototype_setUTCMonth },
    { "setUTCSeconds", datePrototype_setUTCSeconds },
    { "toDateString", datePrototype_toDateString },
    { "toGMTString", datePrototype_toGMTString },
    { "toISOString", datePrototype_toISOString },
    { "toJSON", datePrototype_toISOString },
    { "toLocaleDateString", datePrototype_toLocaleDateString },
    { "toLocaleString", datePrototype_toLocaleString },
    { "toLocaleTimeString", datePrototype_toLocaleTimeString },
    { "toString", datePrototype_toString },
    { "toTimeString", datePrototype_toTimeString },
    { "toUTCString", datePrototype_toUTCString },
    { "valueOf", datePrototype_getTime },
};

void registerDate(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, datePrototypeFunctions, CountOf(datePrototypeFunctions));
    jsValuePrototypeDate = rt->pushObjectValue(prototypeObj);

    SET_PROTOTYPE(dateFunctions, jsValuePrototypeDate);

    rt->setGlobalObject("Date",
        new JsLibObject(rt, dateFunctions, CountOf(dateFunctions), dateConstructor));
}
