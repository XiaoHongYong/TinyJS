//
//  BinaryFileStream.cpp
//  TinyJS
//
//  Created by henry_xiao on 2023/2/17.
//

#include "BinaryFileStream.h"


void BinaryFileInputStream::find(const StringView &pattern, int maxSearchSize) {
    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE + 1024];
    assert(pattern.len < 1024);
    assert(pattern.len > 0);

    if (pattern.len == 0) {
        return;
    }

    long offset = ftell(m_fp);
    if (offset == -1) {
        throw BinaryStreamOutOfRange(__LINE__);
    }
    long offsetEnd = maxSearchSize == -1 ? LONG_MAX : offset + maxSearchSize + pattern.len;

    char fc = pattern.data[0], *ps = pattern.data + 1;
    uint32_t pl = pattern.len - 1;

    long remaining = 0;
    while (true) {
        auto len = fread(buf + remaining, 1, std::min((long)BUF_SIZE, offsetEnd - offset), m_fp);
        char *p = buf, *end = buf + remaining + len - pattern.len;
        for (; p <= end; p++) {
            if (*p == fc && memcmp(p + 1, ps, pl) == 0) {
                offset += (p - buf);
                if (fseek(m_fp, offset, SEEK_SET) != 0) {
                    throw BinaryStreamOutOfRange(__LINE__);
                }
                return;
            }
        }

        if (len < BUF_SIZE) {
            // 读取结束
            throw BinaryStreamOutOfRange(__LINE__);
        }

        memcpy(buf, p, pl);
        offset += len + remaining - pl;
        remaining = pl;
    }
}

void BinaryFileInputStream::rfind(const StringView &pattern, int maxSearchSize) {
    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE + 1024];
    assert(pattern.len < 1024);

    long offset = ftell(m_fp);
    if (offset == -1) {
        throw BinaryStreamOutOfRange(__LINE__);
    }

    // offset 如果包含 pattern 也算满足，所以需要再往后读取 pattern.len
    fseek(m_fp, 0, SEEK_END);
    long fileSize = ftell(m_fp);
    offset += pattern.len;
    if (offset > fileSize) {
        offset = fileSize;
    }
    fseek(m_fp, offset, SEEK_SET);

    // 最多搜索到 offsetStart 的位置结束
    long offsetStart = maxSearchSize == -1 ? 0 : offset - maxSearchSize - pattern.len;
    if (offsetStart < 0) {
        offsetStart = 0;
    }

    char fc = pattern.data[0], *ps = pattern.data + 1;
    uint32_t pl = pattern.len - 1;

    while (true) {
        // 本次读取的数据长度
        long len = std::min((long)BUF_SIZE, offset - offsetStart);
        fseek(m_fp, offset - len, SEEK_SET);
        char *start = buf + BUF_SIZE - len;
        len = fread(start, 1, len, m_fp);

        char *end = start + len;
        for (char *p = end - pattern.len; p >= start; p--) {
            if (*p == fc && memcmp(p + 1, ps, pl) == 0) {
                offset -= (end - p);
                if (fseek(m_fp, offset, SEEK_SET) != 0) {
                    throw BinaryStreamOutOfRange(__LINE__);
                }
                return;
            }
        }

        if (len < BUF_SIZE) {
            // 读取结束
            throw BinaryStreamOutOfRange(__LINE__);
        }

        // 下一个循环接着从 [start, start + pattern.len) 搜索
        offset -= len - pl;
    }
}


#if UNIT_TEST

#include "unittest.h"
#include "FileApi.h"

TEST(BinaryFileStream, rfind) {
    auto path = getUnittestTempDir();
    auto fn = dirStringJoin(path.c_str(), "testBinaryFileStream.txt");

    {
        char buf[4096 * 4];
        memset(buf, '-', sizeof(buf));
        for (int i = 0; i < 4096 * 2; i += 2) {
            buf[i] = 'a'; buf[i + 1] = 'b';
        }

        memcpy(buf, "abcd", 4);
        memcpy(buf + 4096, "abc", 3);
        memcpy(buf + 4096 * 2 + 1, "abc", 3);
        memcpy(buf + 4096 * 3 + 2, "abc", 3);
        memcpy(buf + 4096 * 4 - 3, "abc", 3);

        FilePtr fp(fopen(fn.c_str(), "wb"));
        fwrite(buf, 1, sizeof(buf), fp.ptr());
    }

    {
        BinaryFileInputStream fs(fn.c_str());
        assertException([&fs]() {
            fs.setOffsetToEnd();
            fs.find("abcd");
        }, __LINE__);

        assertException([&fs]() {
            fs.setOffset(0);
            fs.find("abcx");
        }, __LINE__);

        assertException([&fs]() {
            fs.setOffset(0);
            fs.rfind("abcx");
        }, __LINE__);

        fs.setOffset(0);
        fs.find("abc");
        ASSERT_EQ(fs.offset(), 0);

        fs.forward(1);
        fs.find("abc");
        ASSERT_EQ(fs.offset(), 4096);

        fs.forward(1);
        fs.find("abc");
        ASSERT_EQ(fs.offset(), 4096 * 2 + 1);

        fs.forward(1);
        fs.find("abc");
        ASSERT_EQ(fs.offset(), 4096 * 3 + 2);

        fs.forward(1);
        fs.find("abc");
        ASSERT_EQ(fs.offset(), 4096 * 4 - 3);
    }

    {
        BinaryFileInputStream fs(fn.c_str());
        fs.setOffsetToEnd();
        fs.rfind("abc");
        ASSERT_EQ(fs.offset(), 4096 * 4 - 3);

        fs.rfind("abc");
        ASSERT_EQ(fs.offset(), 4096 * 4 - 3);

        fs.backward(1);
        fs.rfind("abc");
        ASSERT_EQ(fs.offset(), 4096 * 3 + 2);

        fs.backward(1);
        fs.rfind("abc");
        ASSERT_EQ(fs.offset(), 4096 * 2 + 1);

        fs.backward(1);
        fs.rfind("abc");
        ASSERT_EQ(fs.offset(), 4096);

        fs.rfind("abcd");
        ASSERT_EQ(fs.offset(), 0);
    }

    deleteFile(fn.c_str());
}

#endif
