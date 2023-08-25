/*
 * os.cpp
 *
 *  Copyright (c) 2019 River Security Technology Corporation, Ltd. All rights reserved.
 */

#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <chrono>
#include "UtilsTypes.h"
#include "StringView.h"
#include "StringEx.h"
#include "os.h"


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
    _dirp = nullptr;
    _dp = nullptr;
}

FileFind::~FileFind() {
    close();
}

bool FileFind::openDir(const char *path, const char *extFilter) {
    close();

    _dp = opendir(path);
    _dirp = nullptr;
    _path = path;
    if (!StringView(_path).endsWith(StringView("/"))) {
        _path += "/";
    }

    if (extFilter) {
        if (extFilter[0] == '.') {
            _extFilter.assign(extFilter);
        } else {
            _extFilter = ".";
            _extFilter.append(extFilter);
        }
    }

    return _dp != nullptr;
}

void FileFind::close() {
    if (_dp) {
        closedir(_dp);
        _dirp = nullptr;
        _dp = nullptr;
    }
}

bool FileFind::findNext() {
    _dirp = readdir(_dp);

    while (_dirp) {
        cstr_t name = _dirp->d_name;
        if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
            // name is . or ..
        } else if (_extFilter.empty() || endsWith(name, _extFilter.c_str())) {
            // Match extension.
            break;
        }
        _dirp = readdir(_dp);
    }

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
