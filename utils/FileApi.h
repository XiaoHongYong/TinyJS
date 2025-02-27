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

FILE *fopenUtf8(const char *fn, const char *mode);

bool readFileByBom(const char *fn, std::string &str);
bool readFile(const char *fn, std::string &str);
bool writeFile(cstr_t fn, const StringView &data);
inline bool writeFile(cstr_t fn, const void *data, size_t len)
    { return writeFile(fn, StringView(data, len)); }

bool filetruncate(FILE *fp, long nLen);

bool getFileLength(const char *fileName, uint64_t &length);
int64_t getFileLength(cstr_t szFileName);

bool createDirectory(cstr_t lpPathName);
bool createDirectoryAll(cstr_t szDir);

bool isDirWritable(cstr_t szDir);

bool enumFilesInDir(cstr_t szBaseDir, cstr_t extFilter, VecStrings &vFiles, bool bEnumFullPath);

typedef void (*FUNProcessFile)(cstr_t szFileName, cstr_t szDir, void *lpUserData);
bool processFilesInfolder(cstr_t szBaseDir, FUNProcessFile funProc, void *lpUserData);

void dirStringAddSep(string &strDir);
bool isAbsPath(cstr_t path);

string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir);
string dirStringJoin(cstr_t szDir, cstr_t szSubFileDir1, cstr_t szSubFileDir2);
inline string dirStringJoin(const string &dir, const string &subFileDir)
    { return dirStringJoin(dir.c_str(), subFileDir.c_str()); }
inline string dirStringJoin(const string &dir, const string &subFileDir1, const string &subFileDir2)
    { return dirStringJoin(dir.c_str(), subFileDir1.c_str(), subFileDir2.c_str()); }

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

bool deleteFile(const char *fileName);
bool moveFile(const char *oldname, const char *newname);

bool copyDir(cstr_t lpExistingDir, cstr_t lpNewDir);
bool copyFile(cstr_t existingFile, cstr_t newFile, bool failIfExists);

struct FileStatInfo {
    int64_t                 fileSize = 0;
    time_t                  createdTime = 0;
    time_t                  moifiedTime = 0;
    bool                    isDirectory = 0;
};

bool getFileStatInfo(cstr_t file, FileStatInfo &infoOut);

class FilePtr {
private:
    FilePtr(const FilePtr &);
    FilePtr &operator=(const FilePtr &);

public:
    FilePtr(FILE *fp) : m_fp(fp) { }
    FilePtr() : m_fp(nullptr) { }
    ~FilePtr() { close(); }

    bool open(cstr_t fileName, cstr_t mode);
    bool open(const string &fileName, cstr_t mode) { return open(fileName.c_str(), mode); }
    void close() { if (m_fp) { fclose(m_fp); m_fp = nullptr; } }

    bool isOK() { return m_fp != nullptr && ferror(m_fp) == 0; }
    bool isEof() { return feof(m_fp) != 0; }

    operator FILE*() const { return (FILE*)m_fp; }

    FILE *ptr() { return m_fp; }

    long fileSize();
    bool seek(long pos, int whence);
    long position();

    size_t write(const void *buf, size_t len);
    size_t write(const string &data) { return write(data.c_str(), data.size()); }
    size_t write(const StringView &data) { return write(data.data, data.len); }
    size_t read(void *buf, size_t len);
    string read(size_t len);

    void flush();

protected:
    FILE                    *m_fp;

};

class InputBufferFile {
public:
    void bind(FILE *fp, size_t sizeBuf);
    bool fill() { return read(_buf.capacity()); }
    bool read(size_t size);
    void forward(int offset);
    void clear() { _pos = _end = 0; }

    StringView buf() const { return StringView(_buf.data() + _pos, int(_end - _pos)); }
    bool empty() const { return _pos >= _end; }
    uint8_t *data() const { return (uint8_t *)_buf.data() + _pos; }
    int size() const { return _end - _pos; }

    uint32_t filePosition(const uint8_t *p);

protected:
    std::vector<char>               _buf;
    int                             _pos = 0, _end = 0;
    FILE                            *_fp = nullptr;

};
