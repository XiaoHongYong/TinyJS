﻿/*
 * os.cpp
 *
 */

#include <sys/stat.h>
#include <time.h>
#include <chrono>

#ifdef _WIN32
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "UtilsTypes.h"
#include "StringView.h"
#include "StringEx.h"
#include "os.h"
#include "FileApi.h"


time_t getTimeInSecond() {
    return time(nullptr);
}

int64_t getTickCount() {
#ifdef WIN32
    return ::GetTickCount();
#else
    auto now = std::chrono::steady_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return now_ms.time_since_epoch().count();
#endif
}

#ifndef _WIN32
void Sleep(uint32_t milliseconds) {
    usleep((unsigned int)(milliseconds * 1000));
}
#endif

FileFind::FileFind() {
}

FileFind::~FileFind() {
    close();
}

bool FileFind::openDir(const char *path, const char *extFilter) {
    close();

    if (!_openFind(path)) {
        return false;
    }

    _path = path;
    if (!StringView(_path).endsWith(PATH_SEP_STR)) {
        _path.push_back(PATH_SEP_CHAR);
    }

    if (extFilter) {
        if (extFilter[0] == '.') {
            _extFilter.assign(extFilter);
        } else {
            _extFilter = ".";
            _extFilter.append(extFilter);
        }
    }

    return true;
}

bool FileFind::findNext() {
    while (_nextFile()) {
        cstr_t name = getCurName();
        if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
            // name is . or ..
        } else if (_extFilter.empty() || endsWith(name, _extFilter.c_str())) {
            // Match extension.
            return true;
        }
    }

    return false;
}

#ifndef _WIN32

void FileFind::close() {
    if (_dp) {
        closedir(_dp);
        _dirp = nullptr;
        _dp = nullptr;
    }
}

bool FileFind::_openFind(const char *path) {
    _dp = opendir(path);
    _dirp = nullptr;
    return _dp != nullptr;
}

bool FileFind::_nextFile() {
    _dirp = readdir(_dp);
    return _dirp != nullptr;
}

bool FileFind::isCurDir() {
    if (!_dirp) {
        return false;
    }

    std::string fn = _path + _dirp->d_name;

    struct stat st;
    memset(&st, 0, sizeof(st));
    if (stat(fn.c_str(), &st) != 0) {
        return false;
    }

    return S_ISDIR(st.st_mode);
}

#else // #ifndef _WIN32

bool FileFind::_openFind(const char *path) {
    memset(&_findData, 0, sizeof(_findData));
    _hfind = FindFirstFileA(path, &_findData);
    _isFirst = true;

    return _hfind != nullptr;
}

void FileFind::close() {
    if (_hfind) {
        FindClose(_hfind);
        _hfind = nullptr;
        memset(&_findData, 0, sizeof(_findData));
    }
}

bool FileFind::_nextFile() {
    if (_hfind) {
        if (_isFirst) {
            _isFirst = false;
            return true;
        } else {
            return FindNextFileA(_hfind, &_findData);
        }
    }
    return false;
}

bool FileFind::isCurDir() {
    if (!_hfind) {
        return false;
    }

    return _findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

#endif // #ifndef _WIN32