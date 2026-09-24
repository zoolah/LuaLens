#pragma once
#include <mutex>
#include <string>
#include "bytecode/compiler.hpp"
#include "bytecode/reader.hpp"
#include "server/bytes.hpp"
#include "server/json.hpp"

namespace api {

inline std::mutex reader_mu;

inline std::string dump_root(const uint8_t* bytes, size_t n) {
    std::lock_guard<std::mutex> lock(reader_mu);
    reader::init(bytes, n);
    if (!reader::read_header())
        return json::error("not lua 5.1 bytecode");
    try {
        Chunk root = reader::read_chunk();
        std::string o = "{\"ok\":true,\"size\":" + std::to_string(n) + ",\"bytes\":[";
        for (size_t i = 0; i < n; i++) {
            if (i) o += ',';
            o += std::to_string((unsigned)bytes[i]);
        }
        o += "],\"root\":";
        json::chunk(o, root);
        return o + '}';
    } catch (const std::exception& e) {
        return json::error("parse error at byte " + std::to_string(reader::pos) + ": " + e.what());
    }
}

inline std::string compile_source(const std::string& src) {
    try {
        auto r = compiler::compile(src);
        if (!r.ok) return json::error(r.error);
        return dump_root(r.bytecode.data(), r.bytecode.size());
    } catch (const std::exception& e) {
        return json::error(e.what());
    }
}

inline std::string disassemble(const std::string& text) {
    try {
        auto bytes = parse_bytes(text);
        if (bytes.empty()) return json::error("compile first");
        return dump_root(bytes.data(), bytes.size());
    } catch (const std::exception& e) {
        return json::error(e.what());
    }
}

inline std::string with_id(const std::string& id, const std::string& payload) {
    if (payload.empty() || payload[0] != '{')
        return "{\"id\":\"" + id + "\",\"ok\":false,\"error\":\"bad payload\"}";
    return "{\"id\":\"" + id + "\"," + payload.substr(1);
}

}
