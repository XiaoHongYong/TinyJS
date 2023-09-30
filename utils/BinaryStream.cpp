//
//  Copyright (c) 2021 CrintSoft, Ltd. All rights reserved.
//

#include "BinaryStream.h"

void BinaryOutputStream::writeFormat(const char *format, ...) {
    va_list args;

    va_start(args, format);

    if (_last + 100 > _end) {
        newBuffer();
    }

    int n = vsnprintf((char *)_last, (size_t)(_end - _last), format, args);
    if (n < 0) {
        char buf[1024 * 8];
        n = vsnprintf(buf, CountOf(buf), format, args);
        if (n > 0) {
            write(buf, n);
        }
    } else {
        _last += n;
    }

    va_end(args);

}

#ifdef UNIT_TEST
#include "unittest.h"

void testBinaryStream() {
    // This function is used to make sure the Unittest of BinaryStream will be linked to run.
}

TEST(BinaryStream, BinaryOutputStreamSize) {
    const int LOOP_TIMES = 10;
    BinaryOutputStream os;

    uint8_t buf[1024 * 4 - 1];
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = i % 256;
    }

    for (int i = 0; i < LOOP_TIMES; i++) {
        os.write(buf, sizeof(buf));
    }

    StringView str = os.toStringView();
    auto *p = (uint8_t *)str.data;
    ASSERT_EQ(str.len, LOOP_TIMES * sizeof(buf));
    for (int i = 0; i < LOOP_TIMES; i++) {
        for (size_t k = 0; k < sizeof(buf); k++) {
            ASSERT_EQ(*p, k % 256);
            ++p;
        }
    }
}

TEST(BinaryStream, BinaryInputStream) {
    size_t offset;

    BinaryOutputStream os;

    os.writeUInt8(0);
    os.writeUInt8(255);

    os.writeUInt16(255);
    os.writeUInt16(0xFFFF);

    os.writeUInt32(0xFF);
    os.writeUInt32(0xFFFF);
    os.writeUInt32(0xFFFFFFFF);

    os.writeUInt64(0xFF);
    os.writeUInt64(0xFFFF);
    os.writeUInt64(0xFFFFFFFF);
    os.writeUInt64(0xFFFFFFFFFFFFFFFFL);

    for (uint32_t i = 0x7F - 10; i <= 0x7F; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 1, os.size());
    }
    for (uint32_t i = 0x7F + 1; i <= 0x7F + 10; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 2, os.size());
    }

    for (uint32_t i = 0x3FFF - 10; i <= 0x3FFF; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 2, os.size());
    }
    for (uint32_t i = 0x3FFF + 1; i <= 0x3FFF + 10; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 3, os.size());
    }

    for (uint32_t i = 0x1FFFFF - 10; i <= 0x1FFFFF; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 3, os.size());
    }
    for (uint32_t i = 0x1FFFFF + 1; i <= 0x1FFFFF + 10; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 4, os.size());
    }

    for (uint32_t i = 0xFFFFFFF - 10; i <= 0xFFFFFFF; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 4, os.size());
    }
    for (uint32_t i = 0xFFFFFFF + 1; i <= 0xFFFFFFF + 10; i++) {
        offset = os.size();
        os.writeVarUInt32(i);
        ASSERT_EQ(offset + 5, os.size());
    }

    for (uint64_t i = 0xFFFFFFFF - 10; i <= 0xFFFFFFFFL; i++) {
        offset = os.size();
        os.writeVarUInt32((uint32_t)i);
        ASSERT_EQ(offset + 5, os.size());
    }

    BinaryInputStream is(os.toStringView());

    ASSERT_EQ(is.readUInt8(), 0);
    ASSERT_EQ(is.readUInt8(), 255);

    ASSERT_EQ(is.readUInt16(), 255);
    ASSERT_EQ(is.readUInt16(), 0xFFFF);

    ASSERT_EQ(is.readUInt32(), 0xFF);
    ASSERT_EQ(is.readUInt32(), 0xFFFF);
    ASSERT_EQ(is.readUInt32(), 0xFFFFFFFF);

    ASSERT_EQ(is.readUInt64(), 0xFF);
    ASSERT_EQ(is.readUInt64(), 0xFFFF);
    ASSERT_EQ(is.readUInt64(), 0xFFFFFFFF);
    ASSERT_EQ(is.readUInt64(), 0xFFFFFFFFFFFFFFFFL);


    for (uint32_t i = 0x7F - 10; i <= 0x7F; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }
    for (uint32_t i = 0x7F + 1; i <= 0x7F + 10; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }

    for (uint32_t i = 0x3FFF - 10; i <= 0x3FFF; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }
    for (uint32_t i = 0x3FFF + 1; i <= 0x3FFF + 10; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }

    for (uint32_t i = 0x1FFFFF - 10; i <= 0x1FFFFF; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }
    for (uint32_t i = 0x1FFFFF + 1; i <= 0x1FFFFF + 10; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }

    for (uint32_t i = 0xFFFFFFF - 10; i <= 0xFFFFFFF; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }
    for (uint32_t i = 0xFFFFFFF + 1; i <= 0xFFFFFFF + 10; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }

    for (uint64_t i = 0xFFFFFFFF - 10; i <= 0xFFFFFFFFL; i++) {
        ASSERT_EQ(is.readVarUint32(), i);
    }
}

TEST(BinaryStream, BinaryOutputStreamStartNew) {
    const int LOOP_TIMES = 10;
    BinaryOutputStream os;

    uint8_t buf[1024 * 4 - 1];
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = i % 256;
    }

    StringView s1, s2, s3, s4;
    for (int i = 0; i < LOOP_TIMES; i++) {
        if (i == 1) {
            s1 = os.toStringView();
            os.startNew();
        } else if (i == 3) {
            s2 = os.toStringView();
            os.startNew();
        } else if (i == 9) {
            s3 = os.toStringView();
            os.startNew();
        }
        os.write(buf, sizeof(buf));
    }
    s4 = os.toStringView();
    os.startNew();

    auto *p = (uint8_t *)s1.data;
    for (int i = 0; i < LOOP_TIMES; i++) {
        if (i == 1) {
            p = (uint8_t *)s2.data;
        } else if (i == 3) {
            p = (uint8_t *)s3.data;
        } else if (i == 9) {
            p = (uint8_t *)s4.data;
        }

        for (size_t k = 0; k < sizeof(buf); k++) {
            if (*p != k % 256) {
                printf("%d, %ld\n", i, k);
            }
            ASSERT_EQ(*p, k % 256);
            ++p;
        }
    }
}

TEST(BinaryStream, BinaryInputStreamBE) {
    BinaryOutputStream os;

    os.writeUInt8(0);

    os.writeUInt16BE(255);
    os.writeUInt16BE(0xFFFF);

    os.writeUInt32BE(0xFF);
    os.writeUInt32BE(0xFFFF);
    os.writeUInt32BE(0xFFFFFFFF);

    os.writeUInt64BE(0xFF);
    os.writeUInt64BE(0xFFFF);
    os.writeUInt64BE(0xFFFFFFFF);
    os.writeUInt64BE(0xFFFFFFFFFFFFFFFFL);

    BinaryInputStream is(os.toStringView());

    ASSERT_EQ(is.readUInt8(), 0);

    ASSERT_EQ(is.readUInt16BE(), 255);
    ASSERT_EQ(is.readUInt16BE(), 0xFFFF);

    ASSERT_EQ(is.readUInt32BE(), 0xFF);
    ASSERT_EQ(is.readUInt32BE(), 0xFFFF);
    ASSERT_EQ(is.readUInt32BE(), 0xFFFFFFFF);

    ASSERT_EQ(is.readUInt64BE(), 0xFF);
    ASSERT_EQ(is.readUInt64BE(), 0xFFFF);
    ASSERT_EQ(is.readUInt64BE(), 0xFFFFFFFF);
    ASSERT_EQ(is.readUInt64BE(), 0xFFFFFFFFFFFFFFFFL);
}

#endif
