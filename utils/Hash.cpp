#include "UtilsTypes.h"
#include "Hash.h"


inline uint64_t rotl64(uint64_t x, int8_t r) {
    return (x << r) | (x >> (64 - r));
}

inline uint64_t fmix64(uint64_t k) {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccd;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53;
    k ^= k >> 33;

    return k;
}

// Refer: https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
uint64_t hashBytes(const void *data, size_t len, uint64_t seed) {
    const uint64_t *p = (const uint64_t *)data;
    const uint64_t *end = p + len / sizeof(uint64_t);

    uint64_t h = seed;
    const uint64_t c = 0x87c37b91114253d5LL;

    for (; p < end; p++) {
        uint64_t k = *p;
        k *= c; k = rotl64(k, 31); h ^= k;

        h = rotl64(h, 27); h = h * 5 + 0x52dce729;
    }

    // tail
    const uint8_t *tail = (const uint8_t*)p;
    uint64_t k = 0;
    switch(len & 7) {
        case 7: k ^= ((uint64_t)tail[6]) << 48;
        case 6: k ^= ((uint64_t)tail[5]) << 40;
        case 5: k ^= ((uint64_t)tail[4]) << 32;
        case 4: k ^= ((uint64_t)tail[3]) << 24;
        case 3: k ^= ((uint64_t)tail[2]) << 16;
        case 2: k ^= ((uint64_t)tail[1]) << 8;
        case 1: k ^= ((uint64_t)tail[0]) << 0;
        k *= c; k = rotl64(k, 31); h ^= k;
    };

    // finalization
    h ^= len;
    h = fmix64(h);

    return h;
}

//
//uint64_t hashBytes(uint8_t *data, size_t len) {
//    uint64_t key = 0;
//
//    for (size_t i = 0; i < len; i++) {
//        key = key * 31 + data[i];
//    }
//
//    return key;
//}
