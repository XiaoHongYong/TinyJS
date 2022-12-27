#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "UtilsTypes.h"
#include "FileApi.h"
#include "os.h"
#include "StringEx.h"
#include "CharEncoding.h"
#include <copyfile.h>


bool isFileExist(const char *filename) {
    struct stat filestat;

    memset(&filestat, 0, sizeof(filestat));
    return stat(filename, &filestat) == 0;
}

bool isDirExist(const char *filename) {
    struct stat filestat;

    memset(&filestat, 0, sizeof(filestat));
    return stat(filename, &filestat) == 0;
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
    FILE *fp;

    str.clear();

    fp = fopen(fileName, "rb");
    if (fp == nullptr) {
        printf("E::%d\n", errno);
        return false;
    }

    bool bRet = readFile(fp, str);

    fclose(fp);
    return bRet;
}

bool writeFile(cstr_t fn, const SizedString &data) {
    auto fp = fopen(fn, "wb");
    if (fp == nullptr) {
        return false;
    }

    bool ret = fwrite(data.data, 1, data.len, fp) == data.len;
    fclose(fp);

    return ret;
}

bool filetruncate(FILE *fp, long nLen) {
    if (ftruncate(fileno(fp), nLen) == -1) {
        return false;
    }

    return true;
}

bool getFileLength(const char *fileName, uint64_t &length) {
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
}

int64_t getFileLength(cstr_t fileName) {
    uint64_t length;
    if (getFileLength(fileName, length)) {
        return (int64_t)length;
    }

    return -1;
}

bool deleteFile(const char *fileName) {
    if (unlink(fileName) == 0) {
        return true;
    }

    return false;
}

bool moveFile(const char *oldname, const char *newname) {
    return rename(oldname, newname) == 0;
}

bool removeDirectory(cstr_t lpPathName) {
    return rmdir(lpPathName) == 0;
}

bool createDirectory(cstr_t lpPathName) {
    int n = mkdir(lpPathName, S_IRWXU | S_IRWXG | S_IROTH);

    return n == 0;
}

bool createDirectoryAll(cstr_t szDir) {
    char szTemp[512];
    char *szBeg;

    string tmp = szDir;

    szBeg = strchr((char *)tmp.c_str(), PATH_SEP_CHAR);
    while (szBeg != nullptr) {
        *szBeg = '\0';

        if (!isEmptyString(szTemp) && !isDirExist(szTemp)) {
            if (!createDirectory(szTemp)) {
                return false;
            }
        }

        *szBeg = PATH_SEP_CHAR;
        szBeg = strchr(szBeg + 1, PATH_SEP_CHAR);
    }

    if (!isDirExist(szTemp)) {
        if (!createDirectory(szDir)) {
            return false;
        }
    }

    return true;
}

bool isDirWritable(cstr_t szDir) {
#ifdef _WIN32
    uint32_t dwAttr;

    dwAttr = GetFileAttributes(szDir);
    if (dwAttr == 0xFFFFFFFF) {
        return false;
    }

    if (!setFileAttributes(szDir, dwAttr)) {
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

bool enumFilesInDir(cstr_t szBaseDir, cstr_t extFilter, vector<string> &vFiles, bool bEnumFullPath) {
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

bool enumDirsInDir(cstr_t szBaseDir, vector<string> &vFiles, bool bEnumFullPath) {
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
        if (!find.isCurDir()) {
            funProc(find.getCurName(), szBaseDir, lpUserData);
        } else if (strcmp(find.getCurName(), ".") != 0
            && strcmp(find.getCurName(), "..") != 0) {
            string subDir = dirStringJoin(szBaseDir, find.getCurName());
            processFilesInfolder(subDir.c_str(), funProc, lpUserData);
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

string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir) {
    string strFile = szDir;

    dirStringAddSep(strFile);
    strFile += szSubFileDir;

    return strFile;
}

string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir1, cstr_t szSubFileDir2) {
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

#ifdef _WIN32_DESKTOP
    // Ignore string like "c:\\"
    if (IsCharAlpha(*szBeg)) {
        szBeg++;
        if (*szBeg == ':') {
            if (*(szBeg + 1) == PATH_SEP_CHAR) {
                szBeg += 2;
            }
        }
    }
#endif // _WIN32_DESKTOP

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

#ifdef _WIN32_DESKTOP
    if (IsCharAlpha(*szBeg)) {
        strNewName += *szBeg;
        szBeg++;
        if (*szBeg == ':' && *(szBeg + 1) == PATH_SEP_CHAR) {
            strNewName.append(szBeg, 2);
            szBeg += 2;
        }
    }
#endif // _WIN32_DESKTOP

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

const char _SZ_FILE_PATH_INVALID_CHARS[] = ":*?\"<>|" PATH_SEP_STR;

void filePathFilterInvalidChars(char * szFile) {
    char * szBeg;

    szBeg = szFile;

#ifdef _WIN32_DESKTOP
    // Ignore string like "c:\\"
    if (IsCharAlpha(*szBeg)) {
        szBeg++;
        if (*szBeg == ':') {
            if (*(szBeg + 1) == PATH_SEP_CHAR) {
                szBeg += 2;
            }
        }
    }
#endif // _WIN32_DESKTOP

    while (*szBeg) {
        cstr_t szInv;

        szInv = _SZ_FILE_PATH_INVALID_CHARS;
        while (*szInv && *szBeg != *szInv) {
            szInv++;
        }
        if (*szInv != '\0') {
            *szBeg = '_';
        } else if (*szBeg == '.') {
            char * szTemp;
            szTemp = szBeg;
            while (*szTemp == '.') {
                szTemp++;
            }
            if (*szTemp == PATH_SEP_CHAR) {
                while (szBeg != szTemp) {
                    *szBeg = '_';
                    szBeg++;
                }
            }
        }
        szBeg++;
    }
}

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

    int n = copyfile(existingFile, newFile, 0, COPYFILE_STAT | COPYFILE_DATA);
    return n == 0;
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

TEST(FileApi, testGetRelatedDir) {
    // #ifdef _WIN32
    //         cstr_t        SZ_DIR[] =  { "c:\\a\\b\\c", "c:\\a\\b\\c", "c:\\a\\b\\c", "c:\\a\\b\\c", "c:\\a\\b\\c" };
    //         cstr_t        SZ_DIR_BASE[] =  { "c:\\a", "c:\\a\\", "", "c:\\a\\b\\c\\d", "c:\\a\\b\\e" };
    //         cstr_t        SZ_DIR_RELATED[] = { "b\\c", "b\\c", "c:\\a\\b\\c", "..\\", "..\\c" };
    // #else
    //         cstr_t        SZ_DIR[] =  { "/a/b/c", "/a/b/c", "/a/b/c", "/a/b/c", "/a/b/c" };
    //         cstr_t        SZ_DIR_BASE[] =  { "/a", "/a/", "", "/a/b/c/d", "/a/b/e" };
    //         cstr_t        SZ_DIR_RELATED[] = { "b/c", "b/c", "/a/b/c", "../", "../c" };
    // #endif
    //         char        szDirRelated[MAX_PATH];
    //
    //         assert(CountOf(SZ_DIR) == CountOf(SZ_DIR_BASE));
    //         assert(CountOf(SZ_DIR) == CountOf(SZ_DIR_RELATED));
    //         for (int i = 0; i < CountOf(SZ_DIR_RELATED); i++)
    //         {
    //             getRelatedPath(SZ_DIR[i], SZ_DIR_BASE[i], szDirRelated, CountOf(szDirRelated));
    //             if (!(strcmp(szDirRelated, SZ_DIR_RELATED[i]) == 0))
    //                 FAIL() << stringPrintf("GetRelatedDir, case: %d, %s", i, szDirRelated);
    //         }
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
        ASSERT_TRUE(strcmp(strOut.c_str(), szFile[i + 1]) == 0);
    }
}

#endif // UNIT_TEST
