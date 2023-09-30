//
//  BinaryFileStream.h
//  TinyJS
//
//  Created by henry_xiao on 2023/2/17.
//

#ifndef BinaryFileStream_h
#define BinaryFileStream_h

#include "BinaryStream.h"


class BinaryFileInputStream {
private:
    BinaryFileInputStream(const BinaryFileInputStream &);
    BinaryFileInputStream &operator=(const BinaryFileInputStream &);

public:
    BinaryFileInputStream(FILE *fp) : m_fp(fp) { fseek(fp, 0, SEEK_SET); }
    BinaryFileInputStream(cstr_t file) { m_fp = fopen(file, "rb"); m_needClose = true; }
    ~BinaryFileInputStream() {
        if (m_needClose && m_fp) {
            fclose(m_fp);
        }
    }

    bool isOK() { return m_fp != nullptr; }

    void find(const StringView &pattern, int maxSearchSize = -1);

    // Search back from current position.
    void rfind(const StringView &pattern, int maxSearchSize = -1);

    uint8_t readUInt8() {
        uint8_t n;
        if (fread(&n, 1, sizeof(uint8_t), m_fp) != sizeof(uint8_t)) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
        return n;
    }

    uint16_t readUInt16() {
        uint16_t n;
        if (fread(&n, 1, sizeof(uint16_t), m_fp) != sizeof(uint16_t)) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
        return n;
    }

    uint32_t readUint24() {
        uint8_t buf[8];
        if (fread(buf, 1, 3, m_fp) != 3) {
            throw BinaryStreamOutOfRange(__LINE__);
        }

        return buf[0] | (buf[1] << 8) | (buf[2] << 16);
    }

    uint32_t readUInt32() {
        uint32_t n;
        if (fread(&n, 1, sizeof(uint32_t), m_fp) != sizeof(uint32_t)) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
        return n;
    }

    uint64_t readUInt64() {
        uint64_t n;
        if (fread(&n, 1, sizeof(uint64_t), m_fp) != sizeof(uint64_t)) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
        return n;
    }

    uint32_t readUint24BE() {
        uint8_t buf[8];
        if (fread(buf, 1, 3, m_fp) != 3) {
            throw BinaryStreamOutOfRange(__LINE__);
        }

        return buf[2] | (buf[1] << 8) | (buf[0] << 16);
    }

    uint16_t readUInt16BE() {
        uint8_t buf[8];
        if (fread(buf, 1, 2, m_fp) != 2) {
            throw BinaryStreamOutOfRange(__LINE__);
        }

        return uint16FromBE(buf);
    }

    uint32_t readUInt32BE() {
        uint8_t buf[8];
        if (fread(buf, 1, 4, m_fp) != 4) {
            throw BinaryStreamOutOfRange(__LINE__);
        }

        return uint32FromBE(buf);
    }

    uint64_t readUInt64BE() {
        uint8_t buf[8];
        if (fread(buf, 1, 8, m_fp) != 8) {
            throw BinaryStreamOutOfRange(__LINE__);
        }

        return uint64FromBE(buf);
    }

    void readBuf(void *buf, size_t len) {
        if (fread((uint8_t *)buf, 1, len, m_fp) != len) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
    }

    string readString(size_t len) {
        string str;
        str.resize(len);

        if (fread((uint8_t *)str.data(), 1, len, m_fp) != len) {
            throw BinaryStreamOutOfRange(__LINE__);
        }

        return str;
    }

    void forward(int n) {
        if (fseek(m_fp, n, SEEK_CUR) != 0) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
    }

    void backward(int n) {
        if (fseek(m_fp, -n, SEEK_CUR) != 0) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
    }

    size_t offset() { return ftell(m_fp); }
    void setOffset(uint32_t offset) {
        if (fseek(m_fp, offset, SEEK_SET) != 0) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
    }
    void setOffsetToEnd() {
        if (fseek(m_fp, 0, SEEK_END) != 0) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
    }

    size_t size() {
        auto org = ftell(m_fp);
        fseek(m_fp, 0, SEEK_END);
        auto size = ftell(m_fp);
        fseek(m_fp, org, SEEK_SET);
        return (size_t)size;
    }

    size_t remainingSize() {
        auto org = ftell(m_fp);
        fseek(m_fp, 0, SEEK_END);
        auto size = ftell(m_fp);
        fseek(m_fp, org, SEEK_SET);
        return (size_t)(size - org);
    }
    bool isRemaining() { return !feof(m_fp); }
    bool isEof() { return feof(m_fp) != 0; }

protected:
    FILE                        *m_fp;
    bool                        m_needClose = false;

};

#endif /* BinaryFileStream_h */
