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
    BinaryFileInputStream(FILE *fp) : m_fp(fp) { }
    BinaryFileInputStream(cstr_t file) { m_fp = fopen(file, "rb"); m_needClose = true; }
    ~BinaryFileInputStream() {
        if (m_fp) {
            fclose(m_fp);
        }
    }

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

    size_t offset() { return ftell(m_fp); }
    void setOffset(uint32_t offset) {
        if (fseek(m_fp, offset, SEEK_SET) != 0) {
            throw BinaryStreamOutOfRange(__LINE__);
        }
    }
    size_t remainingSize() {
        auto org = ftell(m_fp);
        fseek(m_fp, 0, SEEK_END);
        auto size = ftell(m_fp);
        fseek(m_fp, org, SEEK_SET);
        return (size_t)(size - org);
    }
    bool isRemaining() { return !feof(m_fp); }

protected:
    FILE                        *m_fp;
    bool                        m_needClose = false;

};

#endif /* BinaryFileStream_h */
