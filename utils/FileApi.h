#pragma once


#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((uint32_t)   0xFFFFFFFF)
#endif

#ifdef WIN32
#define PATH_SEP_CHAR       '\\'
#define PATH_SEP_STR        "\\"
#else
#define PATH_SEP_CHAR       '/'
#define PATH_SEP_STR        "/"
#endif

bool isFileExist(const char *filename);
bool isDirExist(const char *filename);

bool readFileByBom(const char *fn, std::string &str);
bool readFile(const char *fn, std::string &str);
bool writeFile(cstr_t fn, const SizedString &data);
inline bool writeFile(cstr_t fn, const void *data, size_t len) { return writeFile(fn, SizedString(data, len)); }

bool filetruncate(FILE *fp, long nLen);

bool getFileLength(const char *fileName, uint64_t &length);
int64_t getFileLength(cstr_t szFileName);

bool createDirectory(cstr_t lpPathName);
bool createDirectoryAll(cstr_t szDir);

bool isDirWritable(cstr_t szDir);

bool enumFilesInDir(cstr_t szBaseDir, cstr_t extFilter, vector<string> &vFiles, bool bEnumFullPath);

typedef void (*FUNProcessFile)(cstr_t szFileName, cstr_t szDir, void *lpUserData);
bool processFilesInfolder(cstr_t szBaseDir, FUNProcessFile funProc, void *lpUserData);

void dirStringAddSep(string &strDir);

string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir);
string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir1, cstr_t szSubFileDir2);

void getRelatedPath(cstr_t szDir, cstr_t szBaseDir, char * szRelatedPath, int nRelatedPathLen);

cstr_t urlGetName(cstr_t szFile);
cstr_t urlGetExt(cstr_t szFile);
string urlGetTitle(cstr_t szFile);

inline cstr_t fileGetName(cstr_t szFile) { return urlGetName(szFile); }
inline string fileGetTitle(cstr_t szFile) { return urlGetTitle(szFile); }
inline cstr_t fileGetExt(cstr_t szFile) { return urlGetExt(szFile); }

string fileGetPath(cstr_t szFile);

void fileSetExt(string &strFile, cstr_t szExt);

bool fileIsExtSame(cstr_t szFile, cstr_t szExt);

bool fileNameIsIncInvalidChars(cstr_t szFile);
string fileNameFilterInvalidChars(cstr_t szFile);
void filePathFilterInvalidChars(char * szFile);

bool deleteFile(const char *fileName);
bool moveFile(const char *oldname, const char *newname);

bool copyDir(cstr_t lpExistingDir, cstr_t lpNewDir);
bool copyFile(cstr_t existingFile, cstr_t newFile, bool failIfExists);
