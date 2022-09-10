//
//  base64.cpp
//  Mp3Player
//
//  Created by HongyongXiao on 2021/11/20.
//

#include "UtilsTypes.h"
#include "base64.h"


int initBase64Table();

static const uint8_t *BASE64_ENCODE_TABLE = (const uint8_t *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8_t BASE64_DECODE_TABLE[256];
static int _table_created_ = initBase64Table();

const char *getBase64Set() {
    return (const char *)BASE64_ENCODE_TABLE;
}

int initBase64Table() {
    for (int i = 0; i < 256; i++) {
        BASE64_DECODE_TABLE[i] = 255;
    }

    for (int i = 0; i < 64; i++) {
        BASE64_DECODE_TABLE[BASE64_ENCODE_TABLE[i]] = i;
    }

    return 1;
}

size_t base64Encode(const uint8_t *in, size_t len, uint8_t *out) {
    const uint8_t *p = in;
    const uint8_t *endIn = p + len - len % 3;
    const size_t remaining = len % 3;
    uint8_t *outOrg = out;

    while (p < endIn) {
        out[0] = BASE64_ENCODE_TABLE[p[0] >> 2];
        out[1] = BASE64_ENCODE_TABLE[((p[0] & 3) << 4) | (p[1] >> 4)];
        out[2] = BASE64_ENCODE_TABLE[((p[1] & 0x0f) << 2) | (p[2] >> 6)];
        out[3] = BASE64_ENCODE_TABLE[p[2] & 0x3f];

        out += 4;
        p += 3;
    }

    if (remaining) {
        out[0] = BASE64_ENCODE_TABLE[p[0] >> 2];

        if (remaining == 1) {
            out[1] = BASE64_ENCODE_TABLE[(p[0] & 3) << 4];
            out += 2;
        } else {
            out[1] = BASE64_ENCODE_TABLE[((p[0] & 3) << 4) | (p[1] >> 4)];
            out[2] = BASE64_ENCODE_TABLE[(p[1] & 0x0f) << 2];
            out += 3;
        }
    }

    return (size_t)(out - outOrg);
}

size_t base64EncodeSize(size_t len) {
    size_t n = len / 3 * 4;
    if (len % 3 > 0) {
        n += (len % 3) + 1;
    }
    return n;
}

string base64Encode(const uint8_t *in, size_t len) {
    string out;

    out.resize(base64EncodeSize(len));

    size_t n = base64Encode(in, len, (uint8_t *)out.c_str());
    assert(n == out.size());

    return out;
}

size_t base64Decode(const uint8_t *in, size_t len, uint8_t *out) {
    const uint8_t *p = in, *end = in + len - len % 4;
    const size_t left = len % 4;

    for (; p < end; p += 4, out += 3) {
        uint8_t p0 = BASE64_DECODE_TABLE[p[0]];
        uint8_t p1 = BASE64_DECODE_TABLE[p[1]];
        uint8_t p2 = BASE64_DECODE_TABLE[p[2]];
        uint8_t p3 = BASE64_DECODE_TABLE[p[3]];
        if (p0 >= 64 || p1 >= 64 || p2 >= 64 || p3 >= 64) {
            return -1;
        }

        out[0] = (p0 << 2) | (p1 >> 4);
        out[1] = ((p1 & 0xf) << 4) | (p2 >> 2);
        out[2] = ((p2 & 0x3) << 6) | p3;
    }

    size_t lenOut = len / 4 * 3;

    if (left > 0) {
        if (left == 1) {
            return -1;
        }

        uint8_t p0 = BASE64_DECODE_TABLE[p[0]];
        uint8_t p1 = BASE64_DECODE_TABLE[p[1]];
        if (p0 >= 64 || p1 >= 64) {
            return -1;
        }
        out[0] = (p0 << 2) | (p1 >> 4);
        lenOut++;

        if (left >= 3) {
            uint8_t p2 = BASE64_DECODE_TABLE[p[2]];
            if (p2 >= 64) {
                return -1;
            }
            out[1] = ((p1 & 0xf) << 4) | (p2 >> 2);
            lenOut++;
        }
    }

    return lenOut;
}

size_t base64DecodeSize(size_t len) {
    size_t n = len / 4 * 3;
    if (len % 4 > 0) {
        n += (len % 4) - 1;
    }
    return n;
}

bool base64Decode(const char *in, size_t len, string &out) {
    out.resize(base64DecodeSize(len));

    size_t n = base64Decode((uint8_t *)in, len, (uint8_t *)out.c_str());
    assert(n == -1 || n == out.size());
    if (n == -1) {
        return false;
    }

    return true;
}
