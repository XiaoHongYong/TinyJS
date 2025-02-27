#pragma once

#ifndef __os__
#define __os__

#include <stdio.h>
#include <string>
#ifndef _WIN32
#include <dirent.h>
#include <sys/stat.h>
#endif


time_t getTimeInSecond();

int64_t getTickCount();

#ifndef _WIN32
void Sleep(uint32_t milliseconds);
#endif

bool executeCmd(cstr_t cmdLine);
bool executeCmdAndWait(cstr_t cmdLine, uint32_t timeOut, uint32_t *exitCodeOut);

class FileFind {
public:
    FileFind();
    virtual ~FileFind();

    // extFilter 为空，则枚举所有文件，否则，只有以 extFilter 为扩展名的会被查找.
    bool openDir(const char *path, const char *extFilter = nullptr);
    void close();

    bool findNext();

    bool isCurDir();
    const string &path() const { return _path; }

#ifdef _WIN32
    const char* getCurName();
#else
    const char *getCurName() { return _dirp ? _dirp->d_name : nullptr; }
#endif

protected:
    bool _openFind(const char *path);
    bool _nextFile();

#ifdef _WIN32
    HANDLE                      _hfind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW            _findData;
    bool                        _isFirst = true;
    string                      _curName;
#else
    dirent                      *_dirp = nullptr;
    DIR                         *_dp = nullptr;
#endif

    std::string                 _path;
    std::string                 _extFilter;

};

#endif /* __os__ */
