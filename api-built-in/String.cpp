//
//  String.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"
#include "objects/JsRegExp.hpp"
#include "objects/JsArray.hpp"
#include "objects/JsObjectFunction.hpp"


static void stringConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    JsValue str = args.count > 0 ? runtime->toString(ctx, args[0]) : jsStringValueEmpty;

    if (thiz.type != JDT_NOT_INITIALIZED) {
        ctx->retValue = str;
        return;
    }

    // New
    auto obj = new JsStringObject(str);
    ctx->retValue = runtime->pushObjectValue(obj);
}

void stringFromCharCode(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    string str;
    for (uint32_t i = 0; i < args.count; i++) {
        auto &item = args.data[i];
        int32_t code = item.type == JDT_INT32 ? item.value.n32 : (int32_t)runtime->toNumber(ctx, item);

        // 按照数据类型都完全正确的情况处理
        if (code <= 0xFFFF) {
            utf32CodeToUtf8((uint16_t)code, str);
        }
    }

    auto tmp = runtime->allocString((uint32_t)str.size());
    memcpy(tmp.data, str.c_str(), str.size());
    ctx->retValue = runtime->pushString(JsString(tmp));
}

void stringFromCodePoint(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    string str;
    for (uint32_t i = 0; i < args.count; i++) {
        auto &item = args.data[i];
        double code = item.type == JDT_INT32 ? item.value.n32 : runtime->toNumber(ctx, item);

        // 按照数据类型都完全正确的情况处理
        if (code == (uint32_t)code && code <= 0x10FFFF) {
            utf32CodeToUtf8((uint32_t)code, str);
        } else {
            SizedStringWrapper str(code);
            ctx->throwException(PE_RANGE_ERROR, "Invalid code point %.*s", str.len, str.data);
            return;
        }
    }

    auto tmp = runtime->allocString((uint32_t)str.size());
    memcpy(tmp.data, str.c_str(), str.size());
    ctx->retValue = runtime->pushString(JsString(tmp));
}

static JsLibProperty stringFunctions[] = {
    { "fromCharCode", stringFromCharCode },
    { "fromCodePoint", stringFromCodePoint },
    { "name", nullptr, "String" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


inline JsValue convertStringToJsValue(VMContext *ctx, const JsValue &thiz, const char *funcName) {
    if (thiz.type == JDT_STRING || thiz.type == JDT_CHAR) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_STRING) {
        auto obj = (JsStringObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(PE_TYPE_ERROR, "String.prototype.%s requires that 'this' be a String", funcName);
    return jsValueNotInitialized;
}

void stringPrototypeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "at");
    if (!strValue.isValid()) {
        return;
    }

    int32_t index = (int32_t)runtime->toNumber(ctx, args.getAt(0, JsValue(JDT_INT32, 0)));

    if (strValue.type == JDT_CHAR) {
        if (index == 0 || index == -1) {
            ctx->retValue = strValue;
        } else {
            ctx->retValue = jsValueUndefined;
        }
        return;
    }

    auto &str = runtime->getStringWithRandAccess(strValue);

    if (index < 0) {
        index = str.size() + index;
        if (index < 0) {
            ctx->retValue = jsValueUndefined;
            return;
        }
    }

    if (index < str.size()) {
        ctx->retValue = JsValue(JDT_CHAR, str.chartAt(index));
    } else {
        ctx->retValue = jsValueUndefined;
    }
}

int getStringCharCodeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args, cstr_t apiName) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, apiName);
    if (!strValue.isValid()) {
        return -1;
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (strValue.type == JDT_STRING) {
        auto &str = runtime->getStringWithRandAccess(strValue);
        if (index < 0 || index >= str.size()) {
            return -1;
        }
        return str.chartAt(index);
    } else {
        if (index != 0) {
            return -1;
        }
        return strValue.value.n32;
    }
}

void stringPrototypeCharAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int code = getStringCharCodeAt(ctx, thiz, args, "charAt");
    if (code == -1) {
        ctx->retValue = jsStringValueEmpty;
        return;
    }

    ctx->retValue = JsValue(JDT_CHAR, code);
}

void stringPrototypeCharCodeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    int code = getStringCharCodeAt(ctx, thiz, args, "charCodeAt");
    if (code == -1) {
        ctx->retValue = jsValueNaN;
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, code);
}

void stringPrototypeCodePointAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "codePointAt");
    if (!strValue.isValid()) {
        return;
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (strValue.type == JDT_STRING) {
        auto &str = runtime->getStringWithRandAccess(strValue);
        if (index < 0 || index >= str.size()) {
            ctx->retValue = jsValueUndefined;
            return;
        }
        ctx->retValue = JsValue(JDT_INT32, str.codePointAt(index));
    } else {
        if (index != 0) {
            ctx->retValue = jsValueUndefined;
            return;
        }
        ctx->retValue = JsValue(JDT_INT32, strValue.value.n32);
    }
}

JsValue toStringStrict(VMContext *ctx, const JsValue &val) {
    if (val.type == JDT_SYMBOL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a string");
        return jsValueUndefined;
    }

    return ctx->runtime->toString(ctx, val);
}

LockedSizedStringWrapper toSizedStringStrict(VMContext *ctx, const JsValue &val) {
    if (val.type == JDT_SYMBOL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a string");
        return SS_EMPTY;
    }

    return ctx->runtime->toSizedString(ctx, val);
}

void stringPrototypeConcat(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "concat");
    if (!strVal.isValid()) {
        return;
    }

    auto other = toStringStrict(ctx, args.getAt(0, jsStringValueEmpty));
    if (ctx->error != PE_OK) {
        return;
    }

    ctx->retValue = ctx->runtime->plusString(strVal, ctx->runtime->toString(ctx, other));
}

void stringPrototypeEndsWith(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "endsWith");
    if (!strVal.isValid()) {
        return;
    }

    auto s = runtime->toSizedString(ctx, strVal);
    auto p = toSizedStringStrict(ctx, args.getAt(0, jsStringValueUndefined));
    if (ctx->error != PE_OK) {
        return;
    }

    ctx->retValue = JsValue(JDT_BOOL, s.endsWith(p));
}

/*
void stringPrototypeFixed(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "fixed");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeFontcolor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "fontcolor");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeFontsize(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "fontsize");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}*/

void stringPrototypeIncludes(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "includes");
    if (!strVal.isValid()) {
        return;
    }

    auto s = runtime->toSizedString(ctx, strVal);
    auto p = toSizedStringStrict(ctx, args.getAt(0, jsStringValueUndefined));
    if (ctx->error != PE_OK) {
        return;
    }

    ctx->retValue = JsValue(JDT_BOOL, s.strstr(p) != -1);
}

void stringPrototypeIndexOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "indexOf");
    if (!strVal.isValid()) {
        return;
    }

    int32_t index = 0;
    if (args.count > 1) {
        index = (int32_t)runtime->toNumber(ctx, args[1]);
    }

    auto p = toSizedStringStrict(ctx, args.getAt(0, jsStringValueUndefined));
    if (ctx->error != PE_OK) {
        return;
    }

    // 转换为 utf16 的 string
    SizedStringWrapper tmp;
    SizedStringUtf16 s;
    if (strVal.type == JDT_CHAR) {
        tmp.append(strVal);
        s.set(tmp);
    } else {
        s = runtime->getString(strVal);
    }

    auto pos = s.indexOf(p, index);

    // 返回的位置是 utf-16 的
    ctx->retValue = JsValue(JDT_INT32, pos);
}

void stringPrototypeLastIndexOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "lastIndexOf");
    if (!strVal.isValid()) {
        return;
    }

    int32_t index = 0x7FFFFFFF;
    if (args.count > 1) {
        index = (int32_t)runtime->toNumber(ctx, args[1]);
    }

    auto p = toSizedStringStrict(ctx, args.getAt(0, jsStringValueUndefined));

    // 转换为 utf16 的 string
    SizedStringWrapper tmp;
    SizedStringUtf16 s;
    if (strVal.type == JDT_CHAR) {
        tmp.append(strVal);
        s.set(tmp);
    } else {
        s = runtime->getString(strVal);
    }

    auto pos = s.lastIndexOf(p, index);

    ctx->retValue = JsValue(JDT_INT32, pos);
}

void stringPrototypeLocaleCompare(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "localeCompare");
    if (!strVal.isValid()) {
        return;
    }

    assert(0 && "NOT SUPPORTED YET.");

    ctx->retValue = strVal;
}

inline JsValue regexMatch(VMContext *ctx, VMRuntime *runtime, std::regex &re, uint32_t flags, const SizedString &str, const JsValue &strVal) {
    JsValue ret = jsValueNull;

    if (flags & RegexpFlags::RF_GLOBAL_SEARCH) {
        auto begin = std::cregex_iterator((cstr_t)str.data, (cstr_t)str.data + str.len, re);
        auto end = std::cregex_iterator();
        if (begin == end) {
            return jsValueNull;
        }

        auto arr = new JsArray();
        ret = runtime->pushObjectValue(arr);

        for (auto it = begin; it != end; ++it) {
            auto &m = *it;
            arr->push(ctx, runtime->pushString(str.subStr(m.position(), m.length())));
        }
    } else {
        std::cmatch matches;
        if (std::regex_search((cstr_t)str.data, (cstr_t)str.data + str.len, matches, re)) {
            auto arr = new JsArray();
            ret = runtime->pushObjectValue(arr);

            for (auto &m : matches) {
                arr->push(ctx, runtime->pushString(str.subStr(m.first, m.second)));
            }

            auto index = utf8ToUtf16Length(str.data, (uint32_t)(matches[0].first - (cstr_t)str.data));
            arr->setByName(ctx, ret, SS_INDEX, JsValue(JDT_INT32, index));
            arr->setByName(ctx, ret, SS_GROUPS, jsValueUndefined);
            arr->setByName(ctx, ret, SS_INPUT, strVal);
        }
    }

    return ret;
}

void stringPrototypeMatch(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "match");
    if (!strVal.isValid()) {
        return;
    }

    auto str = runtime->toSizedString(ctx, strVal);

    auto pattern = args.getAt(0, jsStringValueEmpty);
    if (pattern.type == JDT_REGEX) {
        auto regexp = (JsRegExp *)runtime->getObject(pattern);
        auto &re = regexp->getRegexp();
        auto flags = regexp->flags();

        ctx->retValue = regexMatch(ctx, runtime, re, flags, str, strVal);
    } else {
        if (pattern.type == JDT_UNDEFINED) {
            pattern = jsStringValueEmpty;
        }

        LockedSizedStringWrapper strRe = toSizedStringStrict(ctx, pattern);
        auto flags = std::regex::flag_type::ECMAScript;
        std::regex re((cstr_t)strRe.data, strRe.len, flags);

        ctx->retValue = regexMatch(ctx, runtime, re, flags, str, strVal);
    }
}

class StringMatchAllIterator : public IJsIterator {
public:
    StringMatchAllIterator(VMContext *ctx, std::regex &re, uint32_t flags, const SizedString &str, const JsValue &strVal) : _ctx(ctx), _re(re), _flags(flags), _strBegin(str.data), _strEnd(str.data + str.len), _strVal(strVal), _offset(0) { }

    virtual bool isOfIterable() override { return true; }

    virtual bool nextOf(JsValue &valueOut) override {
        auto runtime = _ctx->runtime;
        std::cmatch matches;

        if (std::regex_search((cstr_t)_strBegin, (cstr_t)_strEnd, matches, _re)) {
            auto arr = new JsArray();
            auto ret = runtime->pushObjectValue(arr);
            SizedString str(_strBegin, uint32_t(_strEnd - _strBegin));

            for (auto &m : matches) {
                arr->push(_ctx, runtime->pushString(str.subStr(m.first, m.second)));
            }

            auto m0 = matches[0];
            _offset += utf8ToUtf16Length(_strBegin, (uint32_t)(m0.first - (cstr_t)_strBegin));
            arr->setByName(_ctx, ret, SS_INDEX, JsValue(JDT_INT32, _offset));
            arr->setByName(_ctx, ret, SS_GROUPS, jsValueUndefined);
            arr->setByName(_ctx, ret, SS_INPUT, _strVal);

            _strBegin = (uint8_t *)m0.second;
            _offset += utf8ToUtf16Length((uint8_t *)m0.first, (uint32_t)(m0.second - m0.first));

            valueOut = ret;
            return true;
        }

        return false;
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        return false;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        IJsIterator::markReferIdx(rt);

        rt->markReferIdx(_strVal);
    }

protected:
    VMContext                       *_ctx;
    std::regex                      _re;
    uint32_t                        _flags;

    uint32_t                        _offset;
    uint8_t                         *_strBegin, *_strEnd;
    JsValue                         _strVal;

};

void stringPrototypeMatchAll(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "matchAll");
    if (!strVal.isValid()) {
        return;
    }

    auto str = runtime->toSizedString(ctx, strVal);

    auto pattern = args.getAt(0, jsStringValueEmpty);
    if (pattern.type == JDT_REGEX) {
        auto regexp = (JsRegExp *)runtime->getObject(pattern);
        auto &re = regexp->getRegexp();
        auto flags = regexp->flags();
        if (!isFlagSet(flags, RegexpFlags::RF_GLOBAL_SEARCH)) {
            ctx->throwException(PE_TYPE_ERROR, "String.prototype.matchAll called with a non-global RegExp argument");
            return;
        }

        ctx->retValue = runtime->pushObjectValue(new StringMatchAllIterator(ctx, re, flags, str, strVal));
    } else {
        if (pattern.type == JDT_UNDEFINED) {
            pattern = jsStringValueEmpty;
        }

        LockedSizedStringWrapper strRe = toSizedStringStrict(ctx, pattern);
        auto flags = std::regex::flag_type::ECMAScript;
        std::regex re((cstr_t)strRe.data, strRe.len, flags);

        ctx->retValue = runtime->pushObjectValue(new StringMatchAllIterator(ctx, re, flags, str, strVal));
    }
}

void stringPrototypeNormalize(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "normalize");
    if (!strVal.isValid()) {
        return;
    }

    assert(0 && "TBD");

    ctx->retValue = strVal;
}

void stringPrototypePadEnd(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = convertStringToJsValue(ctx, thiz, "padEnd");
    if (!strVal.isValid()) {
        return;
    }

    auto str = runtime->toSizedString(ctx, strVal);
    auto lenVal = args.getAt(0);
    auto len = runtime->toNumber(ctx, lenVal);
    if (str.len >= len || isnan(len) || ctx->error != PE_OK) {
        ctx->retValue = strVal;
        return;
    }

    if (isinf(len) || len >= LEN_MAX_STRING) {
        ctx->throwException(PE_RANGE_ERROR, "Invalid string length");
        return;
    }

    auto pad = args.getAt(1);
    if (pad.type == JDT_UNDEFINED) {
        pad = JsValue(JDT_CHAR, ' ');
    }

    auto sPad = toSizedStringStrict(ctx, pad);
    if (sPad.len == 0) {
        ctx->retValue = strVal;
        return;
    }

    auto retStr = runtime->allocString((uint32_t)len);
    auto p = retStr.data + str.len;
    memcpy(retStr.data, str.data, str.len);
    if (sPad.len == 1) {
        memset(p, sPad.data[0], retStr.len - str.len);
    } else {
        auto end = retStr.data + retStr.len - sPad.len;
        for (; p < end; p += sPad.len) {
            memcpy(p, sPad.data, sPad.len);
        }

        memcpy(p, sPad.data, retStr.data + retStr.len - p);
    }

    ctx->retValue = runtime->pushString(retStr);
}

void stringPrototypePadStart(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "padStart");
    if (!strVal.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto str = runtime->toSizedString(ctx, strVal);
    auto lenVal = args.getAt(0);
    auto len = runtime->toNumber(ctx, lenVal);
    if (str.len >= len || isnan(len) || ctx->error != PE_OK) {
        ctx->retValue = strVal;
        return;
    }

    if (isinf(len) || len >= LEN_MAX_STRING) {
        ctx->throwException(PE_RANGE_ERROR, "Invalid string length");
        return;
    }

    auto pad = args.getAt(1);
    if (pad.type == JDT_UNDEFINED) {
        pad = JsValue(JDT_CHAR, ' ');
    }

    auto sPad = toSizedStringStrict(ctx, pad);
    if (sPad.len == 0) {
        ctx->retValue = strVal;
        return;
    }

    auto retStr = runtime->allocString((uint32_t)len);
    auto lenPadded = retStr.len - str.len;
    auto p = retStr.data;
    if (sPad.len == 1) {
        memset(p, sPad.data[0], lenPadded);
    } else {
        auto end = p + lenPadded - sPad.len;
        for (; p < end; p += sPad.len) {
            memcpy(p, sPad.data, sPad.len);
        }

        memcpy(p, sPad.data, retStr.data + lenPadded - p);
    }
    memcpy(retStr.data + lenPadded, str.data, str.len);

    ctx->retValue = runtime->pushString(retStr);
}

void stringPrototypeRepeat(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "repeat");
    if (!strVal.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto str = runtime->toSizedString(ctx, strVal);
    auto countVal = args.getAt(0);
    auto count = runtime->toNumber(ctx, countVal);
    if (isinf(count) || count < 0) {
        ctx->throwExceptionFormatJsValue(PE_RANGE_ERROR, "Invalid count value: %.*s", countVal);
        return;
    }

    auto n = (uint32_t)count;
    if (n * str.len  >= LEN_MAX_STRING) {
        ctx->throwException(PE_RANGE_ERROR, "Invalid string length");
        return;
    }

    if (n == 0) {
        ctx->retValue = jsStringValueEmpty;
    } else if (n == 1) {
        ctx->retValue = strVal;
    } else {
        auto retStr = runtime->allocString(n * str.len);
        auto p = retStr.data, end = p + retStr.len;
        for (; p < end; p += str.len) {
            memcpy(p, str.data, str.len);
        }

        ctx->retValue = runtime->pushString(retStr);
    }
}

void doStringReplaceWithFunction(VMContext *ctx, const SizedString &str, const JsValue &strVal, uint32_t matchBegin, uint32_t matchEnd, const std::cmatch &matches, const JsValue &replacementFunc, string &out, uint32_t offset) {

    assert(replacementFunc.type == JDT_FUNCTION);

    auto runtime = ctx->runtime;

    // function replacer(match, p1, p2, /* …, */ pN, offset, string, groups)
    VecJsValues argvs;

    for (auto &m : matches) {
        argvs.push_back(runtime->pushString(str.subStr(m.first, m.second)));
    }
    argvs.push_back(JsValue(JDT_INT32, offset));
    argvs.push_back(strVal);
    argvs.push_back(jsValueUndefined);

    Arguments args(argvs.data(), (uint32_t)argvs.size());
    ctx->vm->callMember(ctx, jsValueGlobalThis, replacementFunc, args);
    if (ctx->error != PE_OK) {
        return;
    }

    auto s = runtime->toSizedString(ctx, ctx->retValue);
    out.append((cstr_t)s.data, s.len);
}

void doStringReplaceWithString(VMContext *ctx, const SizedString &str, uint32_t matchBegin, uint32_t matchEnd, const std::cmatch &matches, const SizedString &replacement, string &out) {

    auto p = replacement.data, end = p + replacement.len;

    while (p < end) {
        auto c = *p++;
        if (c == '$' && p < end) {
            c = *p++;
            if (isDigit(c)) {
                // $n    Inserts the nth (1-indexed) capturing group where n is a positive integer less than 100.
                auto start = p;
                uint32_t n = c - '0';
                while (p < end && isDigit(*p)) {
                    auto tmp = n * 10 + *p - '0';
                    if (tmp >= matches.size()) {
                        break;
                    }
                    n = tmp;
                    p++;
                }

                if (n > 0 && n < matches.size()) {
                    auto &m = matches[n];
                    out.append(m.first, (uint32_t)(m.second - m.first));
                } else {
                    out.append((cstr_t)start - 2, (uint32_t)(2 + p - start));
                }
            } else if (c == '<') {
                // $<Name>    Inserts the named capturing group where Name is the group name.
                assert(0 && "TBD");
            } else if (c == '$') {
                // $$    Inserts a "$".
                out.append(1, '$');
            } else if (c == '&') {
                // $&    Inserts the matched substring.
                out.append((cstr_t)str.data + matchBegin, matchEnd - matchBegin);
            } else if (c == '`') {
                // $`    Inserts the portion of the string that precedes the matched substring.
                out.append((cstr_t)str.data, matchBegin);
            } else if (c == '\'') {
                // $'    Inserts the portion of the string that follows the matched substring.
                out.append((cstr_t)str.data + matchEnd, str.len - matchEnd);
            } else {
                out.append((cstr_t)p - 2, 2);
            }
        } else {
            out.append(1, c);
        }
    }
}

class StringSearch {
public:
    StringSearch(VMContext *ctx, const JsValue &patternVal) {
        if (patternVal.type == JDT_REGEX) {
            regexp = (JsRegExp *)ctx->runtime->getObject(patternVal);
        } else {
            regexp = nullptr;
            pattern = toSizedStringStrict(ctx, patternVal);
            if (ctx->error != PE_OK) {
                return;
            }
        }
    }

    virtual bool search(const SizedString &str, uint32_t &matchBegin, uint32_t &matchEnd, std::cmatch &matches) {
        if (regexp) {
            if (std::regex_search((cstr_t)str.data, (cstr_t)str.data + str.len, matches, regexp->getRegexp())) {
                auto &m0 = matches[0];
                matchBegin = str.offsetOf(m0.first);
                matchEnd = str.offsetOf(m0.second);
                return true;
            } else {
                // 找不到
                return false;
            }
        } else {
            auto pos = str.strstr(pattern);
            if (pos == -1) {
                // 找不到
                return false;
            } else {
                matchBegin = pos;
                matchEnd = pos + pattern.len;
                return true;
            }
        }
    }

public:
    LockedSizedStringWrapper    pattern;
    JsRegExp                    *regexp;

};

void stringPrototypeReplace(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "replace");
    if (!strVal.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto str = runtime->toSizedString(ctx, strVal);
    auto replacementVal = args.getAt(1, jsValueUndefined);

    StringSearch pattern(ctx, args.getAt(0));
    uint32_t matchBegin, matchEnd;
    std::cmatch matches;

    if (pattern.search(str, matchBegin, matchEnd, matches)) {
        string ret((cstr_t)str.data, matchBegin);

        if (replacementVal.type == JDT_FUNCTION) {
            auto offset = utf8ToUtf16Length(str.data, matchBegin);
            doStringReplaceWithFunction(ctx, str, strVal, matchBegin, matchEnd, matches, replacementVal, ret, offset);
        } else {
            auto replacement = toSizedStringStrict(ctx, replacementVal);
            if (ctx->error != PE_OK) {
                return;
            }
            doStringReplaceWithString(ctx, str, matchBegin, matchEnd, matches, replacement, ret);
        }

        ret.append((cstr_t)str.data + matchEnd, str.len - matchEnd);
        ctx->retValue = runtime->pushString(SizedString(ret));
    } else {
        // 找不到
        ctx->retValue = strVal;
    }
}

void stringPrototypeReplaceAll(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "replaceAll");
    if (!strVal.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    StringSearch pattern(ctx, args.getAt(0));
    if (pattern.regexp) {
        if (!isFlagSet(pattern.regexp->flags(), RegexpFlags::RF_GLOBAL_SEARCH)) {
            ctx->throwException(PE_TYPE_ERROR, "String.prototype.replaceAll called with a non-global RegExp argument");
            return;
        }
    }

    auto str = runtime->toSizedString(ctx, strVal);
    auto replacementVal = args.getAt(1, jsValueUndefined);

    LockedSizedStringWrapper replacement;
    if (replacementVal.type != JDT_FUNCTION) {
        replacement = toSizedStringStrict(ctx, replacementVal);
        if (ctx->error != PE_OK) {
            return;
        }
    }

    uint32_t offset = 0;
    bool replaced = false;
    auto p = str.data, strEnd = str.data + str.len;
    string ret;

    while (p < strEnd) {
        uint32_t matchBegin, matchEnd;
        std::cmatch matches;
        SizedString remain = SizedString(p, uint32_t(strEnd - p));

        if (pattern.search(remain, matchBegin, matchEnd, matches)) {
            ret.append((cstr_t)remain.data, matchBegin);

            if (replacementVal.type == JDT_FUNCTION) {
                offset += utf8ToUtf16Length(remain.data, matchBegin);
                doStringReplaceWithFunction(ctx, remain, strVal, matchBegin, matchEnd, matches, replacementVal, ret, offset);
                offset += utf8ToUtf16Length(remain.data + matchBegin, matchEnd - matchBegin);
            } else {
                doStringReplaceWithString(ctx, remain, matchBegin, matchEnd, matches, replacement, ret);
            }
            replaced = true;
            p += matchEnd;
        } else {
            break;
        }
    }

    if (replaced) {
        ret.append((cstr_t)p, uint32_t(strEnd - p));
        ctx->retValue = runtime->pushString(SizedString(ret));
    } else {
        ctx->retValue = strVal;
    }
}

inline void regexSearch(VMContext *ctx, std::regex &re, const SizedString &str) {
    std::cmatch matches;
    if (std::regex_search((cstr_t)str.data, (cstr_t)str.data + str.len, matches, re)) {
        ctx->retValue = JsValue(JDT_INT32, utf8ToUtf16Length(str.data, (uint32_t)(matches[0].first - (cstr_t)str.data)));
    } else {
        ctx->retValue = JsValue(JDT_INT32, -1);
    }
}

void stringPrototypeSearch(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "search");
    if (!strVal.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto str = runtime->toSizedString(ctx, strVal);

    auto pattern = args.getAt(0, jsStringValueEmpty);
    if (pattern.type == JDT_REGEX) {
        auto regexp = (JsRegExp *)runtime->getObject(pattern);
        auto &re = regexp->getRegexp();

        regexSearch(ctx, re, str);
    } else {
        if (pattern.type == JDT_UNDEFINED) {
            pattern = jsStringValueEmpty;
        }

        LockedSizedStringWrapper strRe = toSizedStringStrict(ctx, pattern);
        std::regex re((cstr_t)strRe.data, strRe.len, std::regex::flag_type::ECMAScript);

        regexSearch(ctx, re, str);
    }
}

void stringPrototypeSlice(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "slice");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeSplit(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "split");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeStartsWith(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "startsWith");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeSubstring(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "substring");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeToLocaleLowerCase(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "toLocaleLowerCase");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeToLocaleUpperCase(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "toLocaleUpperCase");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeToLowerCase(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "toLowerCase");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "toString");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeToUpperCase(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "toUpperCase");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeTrim(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "trim");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeTrimEnd(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "trimEnd");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeTrimStart(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "trimStart");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeValueOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto strVal = convertStringToJsValue(ctx, thiz, "valueOf");
    if (!strVal.isValid()) {
        return;
    }

    ctx->retValue = strVal;
}

void stringPrototypeLength(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strValue = convertStringToJsValue(ctx, thiz, "length");
    if (!strValue.isValid()) {
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, runtime->getStringLength(strValue));
}

static JsLibProperty stringPrototypeFunctions[] = {
    // @@iterator
    // * anchor
    { "at", stringPrototypeAt },
    // * big
    // * blink
    // * bold
    { "charAt", stringPrototypeCharAt },
    { "charCodeAt", stringPrototypeCharCodeAt },
    { "codePointAt", stringPrototypeCodePointAt },
    { "concat", stringPrototypeConcat },
    { "endsWith", stringPrototypeEndsWith },
    // { "fixed", stringPrototypeFixed },
    // { "fontcolor", stringPrototypeFontcolor },
    // { "fontsize", stringPrototypeFontsize },
    { "includes", stringPrototypeIncludes },
    { "indexOf", stringPrototypeIndexOf },
    // * { "italics", stringPrototypeItalics },
    { "lastIndexOf", stringPrototypeLastIndexOf },
    // * { "link", stringPrototypeLink },
    { "localeCompare", stringPrototypeLocaleCompare },
    { "match", stringPrototypeMatch },
    { "matchAll", stringPrototypeMatchAll },
    { "normalize", stringPrototypeNormalize },
    { "padEnd", stringPrototypePadEnd },
    { "padStart", stringPrototypePadStart },
    { "repeat", stringPrototypeRepeat },
    { "replace", stringPrototypeReplace },
    { "replaceAll", stringPrototypeReplaceAll },
    { "search", stringPrototypeSearch },
    { "slice", stringPrototypeSlice },
    // * { "small", stringPrototypeSmall },
    { "split", stringPrototypeSplit },
    { "startsWith", stringPrototypeStartsWith },
    // * { "strike", stringPrototypeStrike },
    // * { "sub", stringPrototypeSub },
    // * { "substr", stringPrototypeSubstr },
    { "substring", stringPrototypeSubstring },
    // * { "sup", stringPrototypeSup },
    { "toLocaleLowerCase", stringPrototypeToLocaleLowerCase },
    { "toLocaleUpperCase", stringPrototypeToLocaleUpperCase },
    { "toLowerCase", stringPrototypeToLowerCase },
    { "toString", stringPrototypeToString },
    { "toUpperCase", stringPrototypeToUpperCase },
    { "trim", stringPrototypeTrim },
    { "trimEnd", stringPrototypeTrimEnd },
    { "trimStart", stringPrototypeTrimStart },
    { "valueOf", stringPrototypeValueOf },
    makeJsLibPropertyGetter("length", stringPrototypeLength),
};

class StringPrototypeObject : public JsLibObject {
public:
    JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
        if (name.len > 0 && isDigit(name.data[0])) {
            bool successful = false;
            auto n = name.atoi(successful);
            if (successful) {
                return getRawByIndex(ctx, (uint32_t)n, includeProtoProp);
            }
        }

        return nullptr;
    }

    JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
        static JsProperty propIndex;
        return &propIndex;
    }

};

void registerString(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, stringPrototypeFunctions, CountOf(stringPrototypeFunctions));
    rt->objPrototypeString = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeString, prototypeObj);

    SET_PROTOTYPE(stringFunctions, jsValuePrototypeString);

    rt->setGlobalObject("String",
        new JsLibObject(rt, stringFunctions, CountOf(stringFunctions), stringConstructor));
}
