/*
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
#include "CharEncoding.h"


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
    utf16string pattern = utf8ToUCS2(path) + L"\\*";
    _hfind = FindFirstFileW(pattern.c_str(), &_findData);
    _isFirst = true;

    return _hfind != INVALID_HANDLE_VALUE;
}

void FileFind::close() {
    if (_hfind != INVALID_HANDLE_VALUE) {
        FindClose(_hfind);
        _hfind = INVALID_HANDLE_VALUE;
        memset(&_findData, 0, sizeof(_findData));
    }
}

bool FileFind::_nextFile() {
    if (_hfind != INVALID_HANDLE_VALUE) {
        if (_isFirst) {
            _isFirst = false;
            return true;
        } else {
            return FindNextFileW(_hfind, &_findData);
        }
    }
    return false;
}

bool FileFind::isCurDir() {
    if (_hfind == INVALID_HANDLE_VALUE) {
        return false;
    }

    return _findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

const char *FileFind::getCurName() {
    _curName = ucs2ToUtf8(_findData.cFileName);
    return _curName.c_str();
}

#endif // #ifndef _WIN32

#ifdef _WIN32

bool executeCmd(cstr_t cmdLine) {
    STARTUPINFOW startInfo;
    PROCESS_INFORMATION procInfo;

    memset(&startInfo, 0, sizeof(startInfo));
    memset(&procInfo, 0, sizeof(procInfo));

    utf16string utf16CmdLine = utf8ToUCS2(cmdLine);
    if (!CreateProcessW(nullptr, (LPWSTR)utf16CmdLine.c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &startInfo, &procInfo)) {
        return false;
    }

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return true;
}

bool executeCmdAndWait(cstr_t cmdLine, uint32_t timeOut, DWORD *exitCodeOut) {
    STARTUPINFOW startInfo;
    PROCESS_INFORMATION procInfo;

    memset(&startInfo, 0, sizeof(startInfo));
    memset(&procInfo, 0, sizeof(procInfo));

    utf16string utf16CmdLine = utf8ToUCS2(cmdLine);
    if (!CreateProcessW(nullptr, (LPWSTR)utf16CmdLine.c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &startInfo, &procInfo)) {
        return false;
    }

    bool ret = true;
    if (timeOut != 0 && WaitForSingleObject(procInfo.hProcess, timeOut) == WAIT_TIMEOUT) {
        TerminateProcess(procInfo.hProcess, 0);
        ret = false;
    }

    if (exitCodeOut && ret) {
        GetExitCodeProcess(procInfo.hProcess, exitCodeOut);
    }

    CloseHandle(procInfo.hProcess);
    CloseHandle(procInfo.hThread);

    return ret;
}

#endif
