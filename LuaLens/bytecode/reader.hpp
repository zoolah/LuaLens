#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include "bytecode/chunk.hpp"

namespace reader {

inline const uint8_t* data = nullptr;
inline size_t size = 0, pos = 0, szt = 4;
inline int depth = 0;

inline void init(const uint8_t* bytes, size_t len) {
    data = bytes; size = len; pos = 0; szt = 4; depth = 0;
}

inline void need(size_t n) {
    if (n > size - pos) throw std::out_of_range("read past end of bytecode");
}

template <class T> T read() {
    need(sizeof(T));
    T v;
    std::memcpy(&v, data + pos, sizeof v);
    pos += sizeof v;
    return v;
}

inline std::string str() {
    size_t n = szt == 8 ? read<uint64_t>() : read<uint32_t>();
    need(n);
    std::string s(reinterpret_cast<const char*>(data + pos), n);
    pos += n;
    if (!s.empty()) s.pop_back();
    return s;
}

inline bool read_header() {
    if (size < 12 || std::memcmp(data, "\x1BLua", 4) != 0) return false;
    if (data[4] != 0x51 || data[6] != 1 || data[7] != 4) return false;
    if (data[8] != 4 && data[8] != 8) return false;
    if (data[9] != 4 || data[10] != 8 || data[11] != 0) return false;
    szt = data[8];
    pos = 12;
    return true;
}

template <class F> void counted(F fn) {
    for (uint32_t n = read<uint32_t>(); n; n--) fn();
}

inline Ins decode(uint32_t raw) {
    uint32_t bx = raw >> 14;
    return { raw & 0x3F, (raw >> 6) & 0xFF, raw >> 23, bx & 0x1FF, bx, (int32_t)bx - (int32_t)SBX_BIAS };
}

inline Const read_const() {
    Const k{ read<uint8_t>(), 0, "" };
    if (k.type == K_BOOL) k.n = read<uint8_t>();
    else if (k.type == K_NUMBER) k.n = read<double>();
    else if (k.type == K_STRING) k.s = str();
    return k;
}

inline Local read_local() {
    return { str(), read<uint32_t>(), read<uint32_t>() };
}

inline Chunk read_chunk() {
    if (++depth > 200) throw std::runtime_error("functions nested too deeply");
    Chunk c;
    c.name = str();
    c.first = read<int32_t>();
    c.last = read<int32_t>();
    c.nups = read<uint8_t>();
    c.args = read<uint8_t>();
    c.varg = read<uint8_t>();
    c.stack = read<uint8_t>();
    counted([&] { c.code.push_back(decode(read<uint32_t>())); });
    counted([&] { c.k.push_back(read_const()); });
    counted([&] { c.protos.push_back(read_chunk()); });
    counted([&] { c.lines.push_back(read<uint32_t>()); });
    counted([&] { c.locals.push_back(read_local()); });
    counted([&] { c.upvals.push_back(str()); });
    depth--;
    return c;
}

}
