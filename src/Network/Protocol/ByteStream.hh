#ifndef BYTESTREAM_HH
#define BYTESTREAM_HH

#include <cstdint>
#include <cstring>
#include <vector>

class ByteWriter {
public:
    void U8(uint8_t v);
    void U16(uint16_t v);
    void U32(uint32_t v);
    void F32(float v);

    const std::vector<uint8_t>& Data() const { return _data; }

private:
    std::vector<uint8_t> _data;
};

class ByteReader {
public:
    ByteReader(const uint8_t* data, size_t size) : _data(data), _size(size) {}

    uint8_t  U8();
    uint16_t U16();
    uint32_t U32();
    float    F32();

    bool   Ok() const        { return _ok; }
    size_t Remaining() const { return _size - _pos; }

private:
    const uint8_t* _data;

    size_t _size;
    size_t _pos = 0;

    bool _ok = true;
};

#endif