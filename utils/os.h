#pragma once

#ifndef __os__
#define __os__

#include <stdio.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>


time_t getTimeInSecond();

int64_t getTickCount();

void Sleep(uint32_t milliseconds);

class FileFind {
public:
    FileFind();
    virtual ~FileFind();

    // extFilter 为空，则枚举所有文件，否则，只有以 extFilter 为扩展名的会被查找.
    bool openDir(const char *path, const char *extFilter = nullptr);
    void close();

    bool findNext();

    const char *getCurName() { return _dirp->d_name; }
    bool isCurDir();
    const string &path() const { return _path; }

protected:
    dirent                      *_dirp;
    DIR                         *_dp;
    std::string                 _path;
    std::string                 _extFilter;

};

#endif /* __os__ */
