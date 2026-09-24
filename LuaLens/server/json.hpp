#pragma once
#include <charconv>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "bytecode/chunk.hpp"

namespace json {

inline void str(std::string& o, const std::string& s) {
    o += '"';
    for (unsigned char ch : s) {
        switch (ch) {
        case '"':  o += "\\\""; break;
        case '\\': o += "\\\\"; break;
        case '\n': o += "\\n";  break;
        case '\r': o += "\\r";  break;
        case '\t': o += "\\t";  break;
        default:
            if (ch < 0x20) {
                char b[8];
                std::snprintf(b, sizeof b, "\\u%04x", ch);
                o += b;
            } else o += (char)ch;
        }
    }
    o += '"';
}

inline void num(std::string& o, double d) {
    if (std::isnan(d)) o += "\"NaN\"";
    else if (std::isinf(d)) o += d > 0 ? "\"Infinity\"" : "\"-Infinity\"";
    else {
        char b[40];
        auto r = std::to_chars(b, b + sizeof b, d);
        o.append(b, r.ptr);
    }
}

inline void i64(std::string& o, long long v) { o += std::to_string(v); }

template <class T, class F>
void arr(std::string& o, const std::vector<T>& v, F fn) {
    o += '[';
    for (size_t i = 0; i < v.size(); i++) {
        if (i) o += ',';
        fn(v[i]);
    }
    o += ']';
}

inline void chunk(std::string& o, const Chunk& c) {
    o += "{\"name\":"; str(o, c.name);
    o += ",\"first\":"; i64(o, c.first);
    o += ",\"last\":"; i64(o, c.last);
    o += ",\"nups\":"; i64(o, c.nups);
    o += ",\"args\":"; i64(o, c.args);
    o += ",\"varg\":"; i64(o, c.varg);
    o += ",\"stack\":"; i64(o, c.stack);
    o += ",\"code\":";
    arr(o, c.code, [&](const Ins& x) {
        o += '['; i64(o, x.op); o += ','; i64(o, x.A); o += ','; i64(o, x.B); o += ',';
        i64(o, x.C); o += ','; i64(o, x.Bx); o += ','; i64(o, x.sBx); o += ']';
    });
    o += ",\"lines\":";
    arr(o, c.lines, [&](uint32_t n) { i64(o, n); });
    o += ",\"k\":";
    arr(o, c.k, [&](const Const& k) {
        o += '['; i64(o, k.type); o += ',';
        if (k.type == K_BOOL) o += k.n ? "true" : "false";
        else if (k.type == K_NUMBER) num(o, k.n);
        else if (k.type == K_STRING) str(o, k.s);
        else o += "null";
        o += ']';
    });
    o += ",\"locals\":";
    arr(o, c.locals, [&](const Local& l) {
        o += '['; str(o, l.name); o += ','; i64(o, l.startpc); o += ','; i64(o, l.endpc); o += ']';
    });
    o += ",\"upvals\":";
    arr(o, c.upvals, [&](const std::string& u) { str(o, u); });
    o += ",\"protos\":";
    arr(o, c.protos, [&](const Chunk& p) { chunk(o, p); });
    o += '}';
}

inline std::string error(const std::string& msg) {
    std::string o = "{\"ok\":false,\"error\":";
    str(o, msg);
    return o + '}';
}

}
