#pragma once

#include <assert.h>
#include <cstddef>
#include <stdint.h>
#include <memory.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <mutex>

using namespace std;

using ListStrings = list<string>;
using VecStrings = vector<string>;
using VecInts = vector<int>;
using MapStrings = map<string, string>;
using SetStrings = set<string>;

class MutexAutolock {
public:
    MutexAutolock(mutex &mutex) : m_mutex(&mutex) { mutex.lock(); }
    ~MutexAutolock() { if (m_mutex) m_mutex->unlock(); }
    void unlock() { if (m_mutex) { m_mutex->unlock(); m_mutex = nullptr; } }

private:
    mutex                       *m_mutex;
};

class RMutexAutolock {
public:
    RMutexAutolock(recursive_mutex &mutex) : m_mutex(&mutex) { mutex.lock(); }
    ~RMutexAutolock() { if (m_mutex) m_mutex->unlock(); }
    void unlock() { if (m_mutex) { m_mutex->unlock(); m_mutex = nullptr; } }

private:
    recursive_mutex             *m_mutex;
};

#define CountOf(arr)        (sizeof(arr) / sizeof(arr[0]))

#ifndef _WIN32
// Windows types and defines.
#define MAKEWORD(a, b)      ((uint16_t)(((uint8_t)((a) & 0xff)) | ((uint16_t)((uint8_t)((b) & 0xff))) << 8))
#define MAKEINT(a, b)       ((uint32_t)(((uint16_t)((a) & 0xffff)) | ((Duint16_t)((uint16_t)((b) & 0xffff))) << 16))
#define LOWORD(l)           ((uint16_t)((l) & 0xffff))
#define HIWORD(l)           ((uint16_t)((l) >> 16))
#define LOBYTE(w)           ((uint8_t)((w) & 0xff))
#define HIBYTE(w)           ((uint8_t)((w) >> 8))

inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) { return r | (g << 8) | (b << 16); }

#define MAX_PATH            260
#endif

template<typename T1, typename T2>
inline bool isFlagSet(T1 n, T2 flags) { return (n & flags) == flags; }

template<typename T>
inline bool tobool(T v) { return v != 0; }

typedef char16_t WCHAR;
typedef const char * cstr_t;
typedef const WCHAR * cwstr_t;

#ifndef WIN32
typedef uint32_t COLORREF;
#endif

#include "StringView.h"
