//
//  Copyright (c) 2021 CrintSoft, Ltd. All rights reserved.
//

#ifndef __BinaryStream__
#define __BinaryStream__

#include "AllocatorPool.h"
#include "LinkedString.hpp"


inline uint16_t uint16FromBE(uint8_t *buf) {
	return buf[1] | (buf[0] << 8);
}

inline void uint16ToBE(uint16_t nValue, uint8_t *buf) {
	buf[1] = nValue & 0xFF;
	buf[0] = (nValue >> 8) & 0xFF;
}

inline uint32_t uint32FromBE(uint8_t *buf) {
	return buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
}

inline void uint32ToBE(uint32_t nValue, uint8_t *buf) {
	buf[3] = nValue & 0xFF;
	buf[2] = (nValue >> 8) & 0xFF;
	buf[1] = (nValue >> 16) & 0xFF;
	buf[0] = (nValue >> 24) & 0xFF;
}

inline void uint64ToBE(uint64_t value, uint8_t buf[]) {
    buf[0] = (value >> 56) & 0xff;
    buf[1] = (value >> 48) & 0xff;
    buf[2] = (value >> 40) & 0xff;
    buf[3] = (value >> 32) & 0xff;
    buf[4] = (value >> 24) & 0xff;
    buf[5] = (value >> 16) & 0xff;
    buf[6] = (value >> 8) & 0xff;
    buf[7] = value & 0xff;
}

inline uint64_t uint64FromBE(const uint8_t buf[]) {
    return (uint64_t)buf[7] | ((uint64_t)buf[6] << 8) | ((uint64_t)buf[5] << 16) |
        ((uint64_t)buf[4] << 24) | ((uint64_t)buf[3] << 32) | ((uint64_t)buf[2] << 40) |
        ((uint64_t)buf[1] << 48) | ((uint64_t)buf[0] << 56);
}

inline uint16_t uint16FromLE(uint8_t *buf) {
	return buf[0] | (buf[1] << 8);
}

inline void uint16ToLE(uint16_t nValue, uint8_t *buf) {
	buf[0] = nValue & 0xFF;
	buf[1] = (nValue >> 8) & 0xFF;
}

inline uint32_t uint32FromLE(uint8_t *buf) {
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

inline void uint32ToLE(uint32_t nValue, uint8_t *buf) {
	buf[0] = nValue & 0xFF;
	buf[1] = (nValue >> 8) & 0xFF;
	buf[2] = (nValue >> 16) & 0xFF;
	buf[3] = (nValue >> 24) & 0xFF;
}

class binaryStreamOutOfRange : public std::out_of_range {
public:
    binaryStreamOutOfRange(int lineno) : std::out_of_range(formatLine(lineno)) { }

    static std::string formatLine(int lineno) {
        char buf[256] = { 0 };
        snprintf(buf, sizeof(buf), "Out of range in BinaryInputStream, at line: %d", lineno);
        return buf;
    }
};


class BinaryInputStream {
public:
    BinaryInputStream(const SizedString &s) {
        _buf = (uint8_t *)s.data;
        _pos = (uint8_t *)s.data;
        _end = (uint8_t *)s.data + s.len;
    }

    BinaryInputStream(uint8_t *buf, size_t len) {
        _buf = buf;
        _pos = buf;
        _end = buf + len;
    }

    uint8_t readUInt8() {
        if (_pos + 1 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        return *_pos++;
    }

    uint16_t readUInt16() {
        if (_pos + 2 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        uint16_t n = *(uint16_t *)_pos;
        _pos += 2;
        return n;
    }

    uint32_t readVarUint32() {
        if (_pos + 1 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }

        uint8_t a0 = *_pos++;
        if ((a0 & 0x80) == 0) {
            return a0;
        } else if ((a0 & 0xc0) == 0x80) {
            if (_pos + 1 > _end) {
                throw binaryStreamOutOfRange(__LINE__);
            }
            return ((a0 & 0x3f) << 8) | *_pos++;
        } else if ((a0 & 0xe0) == 0xc0) {
            if (_pos + 2 > _end) {
                throw binaryStreamOutOfRange(__LINE__);
            }
            uint8_t a1 = *_pos++;
            return ((a0 & 0x1f) << 16) | (a1 << 8) | *_pos++;
        } else if ((a0 & 0xf0) == 0xe0) {
            if (_pos + 3 > _end) {
                throw binaryStreamOutOfRange(__LINE__);
            }
            uint8_t a1 = *_pos++;
            uint8_t a2 = *_pos++;
            return (((uint32_t)(a0 & 0xf)) << 24) | (a1 << 16) | (a2 << 8) | *_pos++;
        } else if ((a0 & 0xf8) == 0xf0) {
            if (_pos + 4 > _end) {
                throw binaryStreamOutOfRange(__LINE__);
            }
            assert(a0 == 0xf0);
            uint8_t a1 = *_pos++;
            uint8_t a2 = *_pos++;
            uint8_t a3 = *_pos++;
            return ((uint32_t)(a1 << 24)) | (a2 << 16) | (a3 << 8) | *_pos++;
        }

        assert(false);
        return 0;
    }

    uint32_t readUInt32() {
        if (_pos + 4 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        uint32_t n = *(uint32_t *)_pos;
        _pos += 4;
        return n;
    }

    uint64_t readUInt64() {
        if (_pos + 8 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        uint64_t n = *(uint64_t *)_pos;
        _pos += 8;
        return n;
    }

    uint16_t readUInt16BE() {
        if (_pos + 2 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        uint16_t n = uint16FromBE(_pos);
        _pos += 2;
        return n;
    }

    uint32_t readUInt32BE() {
        if (_pos + 4 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        uint32_t n = uint32FromBE(_pos);
        _pos += 4;
        return n;
    }

    uint64_t readUInt64BE() {
        if (_pos + 8 > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        uint64_t n = uint64FromBE(_pos);
        _pos += 8;
        return n;
    }

    uint8_t *currentPtr() { return _pos; }

    SizedString readSizedStr(size_t len) {
        if (_pos + len > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }

        SizedString s(_pos, len);
        _pos += len;

        return s;
    }

    void forward(size_t n) {
        if (_pos + n > _end) {
            throw binaryStreamOutOfRange(__LINE__);
        }
        _pos += n;
    }

    size_t offset() { return (size_t)(_pos - _buf); }
    size_t remainingSize() { return (size_t)(_end - _pos); }
    bool isRemaining() { return _pos < _end; }

protected:
    uint8_t             *_buf;
    uint8_t             *_pos, *_end;

};


struct StreamBuffer {
    uint32_t            capacity;
    LinkedString        data;

    uint8_t *dataPtr() { return data.data; }
    uint8_t *endPtr() { return data.data + capacity; }

};

// 计算 streamBuffer 的可用空间.
inline uint32_t streamBufferCapacity(uint32_t size) { return size - sizeof(StreamBuffer) + sizeof(LinkedString::data); }

#define streamBufferLen(sb)     (size_t)(sb->last - sb->pos)

class BinaryOutputStream {
private:
    BinaryOutputStream(const BinaryOutputStream &);
    BinaryOutputStream &operator=(const BinaryOutputStream &);

public:
    BinaryOutputStream(AllocatorPool *pool = nullptr, size_t BUFFER_SIZE = 1024 * 4) {
        _defBufCapacity = (uint32_t)BUFFER_SIZE;
        _last = _end = nullptr;
        _linkedStringStart = nullptr;
        _linkedStringLast = nullptr;

        if (pool) {
            _pool = pool;
            _freePool = false;
        } else {
            _pool = new AllocatorPool(BUFFER_SIZE);
            _freePool = true;
        }
    }

    virtual ~BinaryOutputStream() {
        if (_freePool) {
            delete _pool;
        }
    }

    void writeUInt8(uint8_t c) {
        if (_last + 1 > _end) {
            newBuffer();
        }

        *_last++ = c;
    }

    void writeUInt16(uint16_t n) {
        if (_last + 2 > _end) {
            newBuffer();
        }

        *(uint16_t *)_last = n;
        _last += 2;
    }

    void writeVarUInt32(uint32_t n) {
        if (_last + 8 > _end) {
            newBuffer();
        }

        if (n <= 0x7f) {  // 0111 1111
            *_last++ = (uint8_t)n;
        } else if (n <= 0x3FFF) {  // 1011 1111 1111 1111
            *_last++ = (uint8_t)(((n >> 8) & 0xff) | 0x80);
            *_last++ = (uint8_t)(n & 0xff);
        } else if (n <= 0x1FFFFF) {  // 1101 1111 1111 1111 1111 1111
            *_last++ = (uint8_t)(((n >> 16) & 0xff) | 0xc0);
            *_last++ = (uint8_t)((n >> 8) & 0xff);
            *_last++ = (uint8_t)(n & 0xff);
        } else if (n <= 0x0FFFFFFF) {  // 1110 1111 1111 1111 1111 1111 1111 1111
            *_last++ = (uint8_t)(((n >> 24) & 0xff) | 0xe0);
            *_last++ = (uint8_t)((n >> 16) & 0xff);
            *_last++ = (uint8_t)((n >> 8) & 0xff);
            *_last++ = (uint8_t)(n & 0xff);
        } else {  // 1111 0111 1111 1111 1111 1111 1111 1111 1111 1111
            *_last++ = 0xF0;
            *_last++ = (uint8_t)((n >> 24) & 0xff);
            *_last++ = (uint8_t)((n >> 16) & 0xff);
            *_last++ = (uint8_t)((n >> 8) & 0xff);
            *_last++ = (uint8_t)(n & 0xff);
        }
    }

    void writeUInt32(uint32_t n) {
        if (_last + 4 > _end) {
            newBuffer();
        }

        *(uint32_t *)_last = n;
        _last += 4;
    }

    void writeUInt64(uint64_t n) {
        if (_last + 8 > _end) {
            newBuffer();
        }

        *(uint64_t *)_last = n;
        _last += 8;
    }

    void writeUInt16BE(uint16_t n) {
        if (_last + 2 > _end) {
            newBuffer();
        }

        uint16ToBE(n, _last);
        _last += 2;
    }

    void writeUInt32BE(uint32_t n) {
        if (_last + 4 > _end) {
            newBuffer();
        }

        uint32ToBE(n, _last);
        _last += 4;
    }

    void writeUInt64BE(uint64_t n) {
        if (_last + 8 > _end) {
            newBuffer();
        }

        uint64ToBE(n, _last);
        _last += 8;
    }

    void write(const uint8_t *data, size_t len) {
        while (len > 0) {
            uint32_t avail = (uint32_t)(_end - _last);
            if (avail < len) {
                memcpy(_last, data, avail);
                _last += avail;
                data += avail;
                len -= avail;

                newBuffer();
            } else {
                memcpy(_last, data, len);
                _last += len;
                break;
            }
        }
    }

    void write(const void *data, size_t len) {
        write((uint8_t *)data, len);
    }

    void write(const uint8_t *start, const uint8_t *end) {
        write(start, (size_t)(end - start));
    }

    void write(const SizedString &s) {
        write(s.data, s.len);
    }

    void write(const char *s) {
        write((uint8_t *)s, strlen(s));
    }

    void writeFormat(const char *format, ...);

    void write(const char *s, size_t len) {
        write((uint8_t *)s, len);
    }

    void write(BinaryOutputStream &other) {
        auto *p = other._linkedStringStart;
        while (p != nullptr) {
            write(p->data, p->len);

            p = p->next;
        }
    }

    void writeItoA(long num) {
        uint8_t str[64];

        size_t i = 0;
        bool isNegative = false;

        if (num < 0) {
            isNegative = true;
            num = -num;
        }

        do {
            str[i++] = (num % 10) + '0';
            num = num / 10;
        } while (num != 0);

        // append '-'
        if (isNegative) {
            str[i++] = '-';
        }

        // Reverse str
        uint8_t *p = str, *end = str + i - 1;
        while (p < end) {
            char tmp = *p;
            *p = *end;
            *end = tmp;

            p++;
            end--;
        }

        write(str, i);
    }

    uint8_t *writeReserved(uint32_t n) {
        if (_last + n > _end) {
            newBuffer();
        }

        uint8_t *p = _last;
        _last += n;
        return p;
    }

    SizedString sizedStringStartNew() {
        auto ret = toSizedString();

        startNew();

        return ret;
    }

    LinkedString *startNew() {
        auto ret = toLinkedString();

        if (_end - _last > sizeof(LinkedString) * 2) {
            // 空间足够，利用剩下的空间
            _linkedStringStart = _linkedStringLast = (LinkedString *)_last;
            _last = _linkedStringStart->data;
            _linkedStringLast->len = 0;
            _linkedStringLast->next = nullptr;
        } else {
            _linkedStringLast = _linkedStringStart = nullptr;
            newBuffer();
        }

        return ret;
    }

    size_t size() {
        auto p = toLinkedString();
        size_t len = 0;

        while (p != nullptr) {
            len += p->len;
            p = p->next;
        }

        return len;
    }

    bool isEmpty() {
        return _linkedStringStart->data == _last && _linkedStringStart->next == nullptr;
    }

    LinkedString *toLinkedString() {
        if (_linkedStringLast) {
            _linkedStringLast->len = (uint32_t)(_last - _linkedStringLast->data);
        }
        return _linkedStringStart;
    }

    SizedString toSizedString() {
        auto ls = toLinkedString();
        if (ls == nullptr) {
            return SizedString();
        }

        if (ls->next == nullptr) {
            // 只有一个 buffer
            return SizedString(ls->data, ls->len);
        }

        auto *b = ls;
        size_t len = 0;

        while (b != nullptr) {
            len += b->len;
            b = b->next;
        }

        uint8_t *data = (uint8_t *)_pool->allocate(len), *p = data;

        b = ls;
        while (b != nullptr) {
            memcpy(p, b->data, b->len);
            p += b->len;
            b = b->next;
        }

        return SizedString(len, data);
    }

protected:
    void newBuffer() {
        auto buf = (StreamBuffer *)_pool->allocate(_defBufCapacity);
        buf->capacity = streamBufferCapacity(_defBufCapacity);
        buf->data.len = 0;
        buf->data.next = nullptr;

        if (_linkedStringLast) {
            assert(_linkedStringStart != nullptr);
            _linkedStringLast->len = (uint32_t)(_last - _linkedStringLast->data);
            _linkedStringLast->next = &buf->data;
        } else {
            _linkedStringStart = &buf->data;
        }

        _linkedStringLast = &buf->data;

        _last = buf->dataPtr();
        _end = buf->endPtr();
    }

protected:
    AllocatorPool           *_pool;
    bool                    _freePool;
    uint32_t                _defBufCapacity;

    LinkedString            *_linkedStringStart;
    LinkedString            *_linkedStringLast;
    uint8_t                 *_last;
    uint8_t                 *_end;

};


#endif /* defined(__BinaryStream__) */
