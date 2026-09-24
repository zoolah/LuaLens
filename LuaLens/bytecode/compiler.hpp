#pragma once
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace compiler {

struct Result {
    bool ok = false;
    std::string error;
    std::vector<uint8_t> bytecode;
};

struct State {
    lua_State* L;
    State() : L(luaL_newstate()) {}
    ~State() { if (L) lua_close(L); }
    State(const State&) = delete;
    State& operator=(const State&) = delete;
};

inline int writer(lua_State*, const void* p, size_t n, void* ud) {
    auto* out = static_cast<std::vector<uint8_t>*>(ud);
    auto* b = static_cast<const uint8_t*>(p);
    out->insert(out->end(), b, b + n);
    return 0;
}

inline Result compile(const std::string& source, const char* name = "input") {
    Result r;
    if (source.find_first_not_of(" \t\r\n") == std::string::npos) {
        r.error = "paste some lua first";
        return r;
    }

    State st;
    if (!st.L) {
        r.error = "out of memory";
        return r;
    }

    const char* src = source.data();
    size_t len = source.size();
    if (len >= 3 && (unsigned char)src[0] == 0xEF && (unsigned char)src[1] == 0xBB && (unsigned char)src[2] == 0xBF) {
        src += 3;
        len -= 3;
    }

    if (luaL_loadbuffer(st.L, src, len, name) != 0) {
        const char* msg = lua_tostring(st.L, -1);
        r.error = msg ? msg : "compile error";
        return r;
    }
    if (lua_dump(st.L, writer, &r.bytecode) != 0) {
        r.error = "dump failed";
        return r;
    }
    r.ok = true;
    return r;
}

}
