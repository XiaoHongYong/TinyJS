#include <sys/stat.h>
#include <time.h>

#ifdef _MAC_OS
#include <copyfile.h>
#include <unistd.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT	0x0500
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#endif

#include "UtilsTypes.h"
#include "FileApi.h"
#include "os.h"
#include "StringEx.h"
#include "CharEncoding.h"

bool isFileExist(const char *filename) {
#ifdef _WIN32
    auto attr = GetFileAttributesW(utf8ToUCS2(filename).c_str());
    return attr != INVALID_FILE_ATTRIBUTES;
#else // #ifdef _WIN32
    struct stat filestat;

    memset(&filestat, 0, sizeof(filestat));
    return stat(filename, &filestat) == 0;
#endif // #ifdef _WIN32
}

bool isDirExist(const char *filename) {
#ifdef _WIN32
    auto attr = GetFileAttributesW(utf8ToUCS2(filename).c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else // #ifdef _WIN32
    struct stat filestat;

    memset(&filestat, 0, sizeof(filestat));
    return stat(filename, &filestat) == 0 && S_ISDIR(filestat.st_mode);
#endif // #ifdef _WIN32
}

bool readFile(FILE *fp, std::string &str) {
    if (fseek(fp, 0, SEEK_END) != 0) {
        return false;
    }

    size_t len = ftell(fp);
    if (len == 0) {
        return true;
    }

    fseek(fp, 0, SEEK_SET);

    str.resize((size_t)len);
    char *buf = (char *)str.c_str();
    if (fread(buf, 1, len, fp) != (size_t)len) {
        return false;
    }

    return true;
}

bool readFile(const char *fileName, std::string &str) {
    str.clear();

#ifdef _WIN32
    auto fp = _wfopen(utf8ToUCS2(fileName).c_str(), L"rb");
#else
    auto fp = fopen(fileName, "rb");
#endif

    if (fp == nullptr) {
        printf("E::%d\n", errno);
        return false;
    }

    bool bRet = readFile(fp, str);

    fclose(fp);
    return bRet;
}

bool writeFile(cstr_t fn, const StringView &data) {
#ifdef _WIN32
    auto fp = _wfopen(utf8ToUCS2(fn).c_str(), L"wb");
#else
    auto fp = fopen(fn, "wb");
#endif
    if (fp == nullptr) {
        return false;
    }

    bool ret = fwrite(data.data, 1, data.len, fp) == data.len;
    fclose(fp);

    return ret;
}

bool filetruncate(FILE *fp, long nLen) {
#ifdef _WIN32
    if (fseek(fp, nLen, SEEK_SET) != 0)
        return false;

    HANDLE	file;
    file = (HANDLE)_get_osfhandle(_fileno(fp));
    if (file)
    {
        if (!SetEndOfFile(file))
            return false;
    }

    return true;
#else
    if (ftruncate(fileno(fp), nLen) == -1) {
        return false;
    }

    return true;
#endif
}

bool getFileLength(const char *fileName, uint64_t &length) {
#ifdef WIN32
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExW(utf8ToUCS2(fileName).c_str(), GetFileExInfoStandard, &attr)) {
        length = 0;
        return false;
    }

    length = attr.nFileSizeLow | ((uint64_t)attr.nFileSizeHigh << 32);
    return true;
#else
    struct stat filestat;

    memset(&filestat, 0, sizeof(filestat));
    int ret = stat(fileName, &filestat);
    if (ret == 0) {
        length = filestat.st_size;
        return true;
    } else {
        length = 0;
        return false;
    }
#endif
}

int64_t getFileLength(cstr_t fileName) {
    uint64_t length;
    if (getFileLength(fileName, length)) {
        return (int64_t)length;
    }

    return -1;
}

bool deleteFile(const char *fileName) {
#ifdef _WIN32
    if (_wunlink(utf8ToUCS2(fileName).c_str()) == 0)
#else
    if (unlink(fileName) == 0)
#endif
    {
        return true;
    }

    return false;
}

bool moveFile(const char *oldname, const char *newname) {
#ifdef _WIN32
    return _wrename(utf8ToUCS2(oldname).c_str(), utf8ToUCS2(newname).c_str()) == 0;
#else
    return rename(oldname, newname) == 0;
#endif
}

bool removeDirectory(cstr_t pathName) {
#ifdef _WIN32
    return RemoveDirectoryW(utf8ToUCS2(pathName).c_str());
#else
    return rmdir(pathName) == 0;
#endif
}

bool createDirectory(cstr_t pathName) {
#ifdef _WIN32
    return CreateDirectoryW(utf8ToUCS2(pathName).c_str(), nullptr);
#else
    int n = mkdir(pathName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    return n == 0;
#endif
}

bool createDirectoryAll(cstr_t szDir) {
    string tmp = szDir;
    auto path = tmp.c_str();
    auto p = strchr((char *)tmp.c_str(), PATH_SEP_CHAR);
    while (p != nullptr) {
        *p = '\0';

        if (!isEmptyString(path) && !isDirExist(path)) {
            if (!createDirectory(path)) {
                return false;
            }
        }

        *p = PATH_SEP_CHAR;
        p = strchr(p + 1, PATH_SEP_CHAR);
    }

    if (!isDirExist(path)) {
        if (!createDirectory(path)) {
            return false;
        }
    }

    return true;
}

bool isDirWritable(cstr_t szDir) {
#ifdef _WIN32
    uint32_t dwAttr;

    utf16string dirUcs2 = utf8ToUCS2(szDir);
    dwAttr = GetFileAttributesW(dirUcs2.c_str());
    if (dwAttr == 0xFFFFFFFF) {
        return false;
    }

    if (!SetFileAttributesW(dirUcs2.c_str(), dwAttr)) {
        return false;
    }

    // Try to create a file in the folder, then delete it.
    string strFile = szDir;
    dirStringAddSep(strFile);
    strFile += "temp.txt";
    if (!writeFile(strFile.c_str(), "abc")) {
        return false;
    }
    deleteFile(strFile.c_str());
#endif

    return true;
}

bool enumFilesInDir(cstr_t szBaseDir, cstr_t extFilter, VecStrings &vFiles, bool bEnumFullPath) {
    FileFind find;

    if (!find.openDir(szBaseDir, extFilter)) {
        return false;
    }

    while (find.findNext()) {
        if (!find.isCurDir()) {
            if (bEnumFullPath) {
                string strFile;

                strFile = szBaseDir;
                dirStringAddSep(strFile);
                strFile += find.getCurName();

                vFiles.push_back(strFile);
            } else {
                vFiles.push_back(find.getCurName());
            }
        }
    }

    return true;
}

bool enumDirsInDir(cstr_t szBaseDir, VecStrings &vFiles, bool bEnumFullPath) {
    FileFind find;

    if (!find.openDir(szBaseDir, "*")) {
        return false;
    }

    while (find.findNext()) {
        if (find.isCurDir()) {
            if (bEnumFullPath) {
                string strFile;

                strFile = szBaseDir;
                dirStringAddSep(strFile);
                strFile += find.getCurName();

                vFiles.push_back(strFile);
            } else {
                vFiles.push_back(find.getCurName());
            }
        }
    }

    return true;
}

// typedef void (*FUNProcessFile)(cstr_t szFileName, cstr_t szDir, void *lpUserData);
bool processFilesInfolder(cstr_t szBaseDir, FUNProcessFile funProc, void *lpUserData) {
    FileFind find;

    if (!find.openDir(szBaseDir, nullptr)) {
        return false;
    }

    while (find.findNext()) {
        if (find.isCurDir()) {
            string subDir = dirStringJoin(szBaseDir, find.getCurName());
            processFilesInfolder(subDir.c_str(), funProc, lpUserData);
        } else {
            funProc(find.getCurName(), szBaseDir, lpUserData);
        }
    }

    return true;
}

// COMMENT:
//        将目录字符串格式化为末尾带DIR_SLASH的
void dirStringAddSep(string &dir) {
    if (!dir.empty() && dir.back() != PATH_SEP_CHAR) {
        dir.append(1, PATH_SEP_CHAR);
    }
}

bool isAbsPath(cstr_t path) {
#ifdef _WIN32
    return path[0] && path[1] == ':';
#else
    return path[0] == PATH_SEP_CHAR;
#endif
}

string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir) {
    if (isAbsPath(szSubFileDir)) {
        return szSubFileDir;
    }

    string strFile = szDir;

    dirStringAddSep(strFile);
    strFile += szSubFileDir;

    return strFile;
}

string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir1, cstr_t szSubFileDir2) {
    if (isAbsPath(szSubFileDir2)) {
        return szSubFileDir2;
    }

    if (isAbsPath(szSubFileDir1)) {
        return dirStringJoin(szSubFileDir1, szSubFileDir2);
    }

    string strFile = szDir;

    dirStringAddSep(strFile);
    strFile += szSubFileDir1;

    dirStringAddSep(strFile);
    strFile += szSubFileDir2;

    return strFile;
}


cstr_t urlGetName(cstr_t szFile) {
    cstr_t szBeg, szBeg2;

    szBeg = strrchr(szFile, '/');
    szBeg2 = strrchr(szFile, '\\');
    if (szBeg2 > szBeg) {
        szBeg = szBeg2;
    }
    if (szBeg) {
        return szBeg + 1;
    } else {
        return szFile;
    }
}

cstr_t urlGetExt(cstr_t szFile) {
    cstr_t szBeg;

    szBeg = strrchr(szFile, '.');
    if (szBeg == nullptr) {
        return "";
    } else {
        cstr_t szTemp = strrchr(szFile, '/');
        cstr_t szTemp2 = strrchr(szFile, '\\');
        if (szTemp2 > szTemp) {
            szTemp = szTemp2;
        }
        if (szTemp == nullptr || szTemp < szBeg) {
            return szBeg;
        } else {
            return "";
        }
    }
}

string urlGetTitle(cstr_t szFile) {
    cstr_t szBeg, szBeg2, szEnd;
    size_t nLen;
    string strFileTitle;

    szEnd = strrchr(szFile, '.');

    szBeg = strrchr(szFile, '/');
    szBeg2 = strrchr(szFile, '\\');
    if (szBeg2 > szBeg) {
        szBeg = szBeg2;
    }

    if (szBeg == nullptr) {
        szBeg = szFile;
    } else {
        szBeg++;
    }

    if (szEnd == nullptr || szBeg > szEnd) {
        nLen = szFile - szBeg + strlen(szFile);
    } else {
        nLen = szEnd - szBeg;
    }

    strFileTitle.append(szBeg, nLen);

    return strFileTitle;
}

string fileGetPath(cstr_t szFile) {
    string strPath;
    long ilen = strlen(szFile);
    ilen -= 2;
    while (ilen >= 0) {
        if (szFile[ilen] == PATH_SEP_CHAR) {
            break;
        }
        ilen--;
    }

    strPath.resize(0);
    if (ilen >= 0) {
        strPath.append(szFile, ilen + 1);
    }

    return strPath;
}

void fileSetExt(string &strFile, cstr_t szExt) {
    cstr_t szFile = strFile.c_str();
    long len = strFile.size();

    len --;
    while (len >= 0) {
        if (szFile[len] == PATH_SEP_CHAR || szFile[len] == '.') {
            break;
        }
        len --;
    }

    if (len >= 0) {
        if (szFile[len] == '.') {
            strFile.resize(len);
        }
    }

    strFile += szExt;
}

bool fileIsExtSame(cstr_t szFile, cstr_t szExt) {
    return iEndsWith(szFile, szExt);
}

// /:*?"<>|
#ifdef _WIN32
const char _SZ_FILE_NAME_INVALID_CHARS[] = "/:*?\"<>|";
#else
const char _SZ_FILE_NAME_INVALID_CHARS[] = "\\:*?\"<>|";
#endif

bool fileNameIsIncInvalidChars(cstr_t szFile) {
    cstr_t szBeg;

    szBeg = szFile;

#ifdef _WIN32
    // Ignore string like "c:\\"
    if (IsCharAlpha(*szBeg)) {
        szBeg++;
        if (*szBeg == ':') {
            if (*(szBeg + 1) == PATH_SEP_CHAR) {
                szBeg += 2;
            }
        }
    }
#endif // _WIN32

    while (*szBeg) {
        cstr_t szInv;

        szInv = _SZ_FILE_NAME_INVALID_CHARS;
        while (*szInv && *szBeg != *szInv) {
            szInv++;
        }
        if (*szInv != '\0') {
            return true;
        }

        szBeg++;
    }
    return false;
}

string fileNameFilterInvalidChars(cstr_t szFile) {
    cstr_t szBeg = szFile;
    string strNewName;

#ifdef _WIN32
    if (IsCharAlpha(*szBeg)) {
        strNewName += *szBeg;
        szBeg++;
        if (*szBeg == ':' && *(szBeg + 1) == PATH_SEP_CHAR) {
            strNewName.append(szBeg, 2);
            szBeg += 2;
        }
    }
#endif // _WIN32

    while (*szBeg) {
        cstr_t szInv;

        szInv = _SZ_FILE_NAME_INVALID_CHARS;
        while (*szInv && *szBeg != *szInv) {
            szInv++;
        }
        if (*szInv != '\0') {
            strNewName += '_';
        } else {
            strNewName += *szBeg;
        }

        szBeg++;
    }

    return strNewName;
}

// const char _SZ_FILE_PATH_INVALID_CHARS[] = ":*?\"<>|" PATH_SEP_STR;

bool copyDir(cstr_t lpExistingDir, cstr_t lpNewDir) {
    FileFind finder;
    if (!finder.openDir(lpExistingDir)) {
        return false;
    }

    if (!isFileExist(lpNewDir)) {
        createDirectory(lpNewDir);
    }

    while (finder.findNext()) {
        string src = dirStringJoin(lpExistingDir, finder.getCurName());
        string dst = dirStringJoin(lpNewDir, finder.getCurName());

        bool ret;
        if (finder.isCurDir()) {
            ret = copyDir(src.c_str(), dst.c_str());
        } else {
            ret = copyFile(src.c_str(), dst.c_str(), false);
        }
        if (!ret) {
            return false;
        }
    }

    return true;
}

bool copyFile(cstr_t existingFile, cstr_t newFile, bool failIfExists) {
    if (!failIfExists) {
        if (isFileExist(newFile)) {
            deleteFile(newFile);
        }
    }

#ifdef _MAC_OS
    int n = copyfile(existingFile, newFile, 0, COPYFILE_STAT | COPYFILE_DATA);
    return n == 0;
#elif defined(WIN32)
    return CopyFileW(utf8ToUCS2(existingFile).c_str(), utf8ToUCS2(newFile).c_str(), failIfExists);
#else
    int input, output;
    if ((input = open(existingFile, O_RDONLY)) == -1) {
        return false;
    }
    if ((output = creat(newFile, 0660)) == -1) {
        close(input);
        return false;
    }

    off_t bytesCopied = 0;
    struct stat fileinfo = {0};
    fstat(input, &fileinfo);
    int bytesSent = sendfile(output, input, &bytesCopied, fileinfo.st_size);

    close(input);
    close(output);

    return bytesSent != -1;
#endif
}

#ifdef WIN32
// https://learn.microsoft.com/en-us/windows/win32/sysinfo/converting-a-time-t-value-to-a-file-time
time_t fileTimeToTime(FILETIME& ft) {
    ULARGE_INTEGER v;
    v.LowPart = ft.dwLowDateTime;
    v.HighPart = ft.dwHighDateTime;

    return (v.QuadPart - 116444736000000000LL) / 10000000LL;
}
#endif

bool getFileStatInfo(cstr_t file, FileStatInfo &infoOut) {
#ifdef WIN32
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesExW(utf8ToUCS2(file).c_str(), GetFileExInfoStandard, &attr)) {
        return false;
    }

    infoOut.createdTime = fileTimeToTime(attr.ftCreationTime);
    infoOut.moifiedTime = fileTimeToTime(attr.ftLastWriteTime);
    infoOut.fileSize = attr.nFileSizeLow | ((uint64_t)attr.nFileSizeHigh << 32);
    infoOut.isDirectory = (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

    return true;
#else
    struct stat filestat;

    memset(&filestat, 0, sizeof(filestat));
    int ret = stat(file, &filestat);
    if (ret != 0) {
        return false;
    }

    infoOut.fileSize = filestat.st_size;
    infoOut.moifiedTime = filestat.st_mtime;
    infoOut.isDirectory = S_ISDIR(filestat.st_mode);

#ifdef _MAC_OS
    infoOut.createdTime = filestat.st_birthtime;
#else
    infoOut.createdTime = filestat.st_ctime;
#endif
#endif

    return true;
}

bool FilePtr::open(const char *fileName, const char *mode) {
#ifdef _WIN32
    m_fp = _wfopen(utf8ToUCS2(fileName).c_str(), utf8ToUCS2(mode).c_str());
#else
    m_fp = fopen(fileName, mode);
#endif

    return m_fp != nullptr;
}

long FilePtr::fileSize() {
    auto n = ftell(m_fp);
    fseek(m_fp, 0, SEEK_END);
    auto len = ftell(m_fp);
    fseek(m_fp, n, SEEK_SET);

    return len;
}

bool FilePtr::seek(long pos, int whence) {
    return fseek(m_fp, pos, whence) == 0;
}

long FilePtr::position() {
    return ftell(m_fp);
}

size_t FilePtr::write(const void *buf, size_t len) {
    return fwrite(buf, 1, len, m_fp);
}

size_t FilePtr::read(void *buf, size_t len) {
    return fread(buf, 1, len, m_fp);
}

string FilePtr::read(size_t len) {
    string str;
    str.resize(len);
    len = fread((char *)str.data(), 1, len, m_fp);
    str.resize(len);

    return str;
}

void FilePtr::flush() {
    fflush(m_fp);
}

void InputBufferFile::bind(FILE *fp, size_t sizeBuf) {
    _fp = fp;
    _buf.resize(sizeBuf);
}

bool InputBufferFile::read(size_t size) {
    assert(_fp);

    int toRead = int(size) - (_end - _pos);
    if (toRead > 0) {
        if (_buf.capacity() < size) {
            _buf.resize(size);
        }

        if (_pos + size > _buf.capacity()) {
            _end = _end - _pos;
            if (_end > 0) {
                memmove(_buf.data(), _buf.data() + _pos, _end);
            }
            _pos = 0;
        }

        auto read = fread(_buf.data() + _end, 1, toRead, _fp);
        _end += read;
        assert(_end <= _buf.size());

        return read > 0;
    }

    return true;
}

void InputBufferFile::forward(int offset) {
    _pos += offset;
    if (_pos > _end) {
        fseek(_fp, _pos - _end, SEEK_CUR);
        _pos = _end = 0;
    }
}

uint32_t InputBufferFile::filePosition(const uint8_t *p) {
    auto pos = uint32_t((cstr_t)p - _buf.data());

    assert(_fp);
    assert(pos >= _pos && pos <= _end);

    return uint32_t(ftell(_fp) - (_end - pos));
}

#if UNIT_TEST

#include "unittest.h"


//////////////////////////////////////////////////////////////////////////
// CPPUnit test
TEST(FileApi, testFileNameStr) {
#ifdef _WIN32
    cstr_t SZ_DIR_CMP[] = { "c:\\folder", "c:\\folder\\", "" };
    cstr_t SZ_DIR_CMP_OK[] = { "c:\\folder\\", "c:\\folder\\", "" };
#else
    cstr_t SZ_DIR_CMP[] = { "/tmp/folder", "/tmp/folder/", "" };
    cstr_t SZ_DIR_CMP_OK[] = { "/tmp/folder/", "/tmp/folder/", "" };
#endif
    string strDir;

    assert(CountOf(SZ_DIR_CMP) == CountOf(SZ_DIR_CMP_OK));
    for (int i = 0; i < CountOf(SZ_DIR_CMP); i++) {
        strDir = SZ_DIR_CMP[i];
        dirStringAddSep(strDir);
        if (!(strcmp(strDir.c_str(), SZ_DIR_CMP_OK[i]) == 0)) {
            FAIL() << stringPrintf("dirStringAddSep(stl), case: %d, %s", i, SZ_DIR_CMP_OK[i]).c_str();
        }
    }
}

TEST(FileApi, testFileSetExt) {
    cstr_t SZ_EXT = ".ext";
#ifdef _WIN32
    cstr_t SZ_FILE[] = { "c:\\file.txt", "c:\\file.txt3", "", "c:\\file.t\\xt3" };
    cstr_t SZ_FILE_OK[] = { "c:\\file.ext", "c:\\file.ext", ".ext", "c:\\file.t\\xt3.ext" };
#else
    cstr_t SZ_FILE[] = { "/file.txt", "/file.txt3", "", "/file.t/xt3" };
    cstr_t SZ_FILE_OK[] = { "/file.ext", "/file.ext", ".ext", "/file.t/xt3.ext" };
#endif
    cstr_t SZ_EXT_OK[] = { ".txt", ".txt3", "", "" };

    string strFile;

    assert(CountOf(SZ_FILE) == CountOf(SZ_FILE_OK));
    for (int i = 0; i < CountOf(SZ_FILE); i++) {
        strFile = SZ_FILE[i];
        fileSetExt(strFile, SZ_EXT);
        if (!(strcmp(strFile.c_str(), SZ_FILE_OK[i]) == 0)) {
            FAIL() << stringPrintf("fileSetExt(stl), case: %d, %s", i, SZ_FILE_OK[i]);
        }

        if (!(strcmp(fileGetExt(SZ_FILE[i]), SZ_EXT_OK[i]) == 0)) {
            FAIL() << stringPrintf("fileGetExt, case: %d, %s", i, SZ_EXT_OK[i]);
        }
    }
}

TEST(FileApi, testFileNameFilterInvalidChars) {
#ifdef _WIN32
    // /:*?\"<>|
    cstr_t        szFile[] = {
        "c:\\aa?:!/<>|*.txt", "c:\\aa__!_____.txt",
        "c:\\bb\\cc\aa.txt", "c:\\bb\\cc\aa.txt",
        "a?:a.txt", "a__a.txt",
    };
#else
    cstr_t        szFile[] = {
        "/aa?:!\\<>|*.txt", "/aa__!_____.txt",
        "/bb/cc/aa.txt", "/bb/cc/aa.txt",
        "a?:a.txt", "a__a.txt",
    };
#endif

    for (int i = 0; i < CountOf(szFile); i += 2) {
        string strOut = fileNameFilterInvalidChars(szFile[i]);
        // printf("%d: %s, %s\n", i, strOut.c_str(), szFile[i + 1]);
        ASSERT_TRUE(strcmp(strOut.c_str(), szFile[i + 1]) == 0);
    }
}

#endif // UNIT_TEST
