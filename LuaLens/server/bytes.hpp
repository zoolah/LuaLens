#pragma once
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

inline std::string format_bytes(const uint8_t* p, size_t n) {
    static const char* hex = "0123456789ABCDEF";
    std::string s;
    s.reserve(n * 6);
    for (size_t i = 0; i < n; i++) {
        s += "0x";
        s += hex[p[i] >> 4];
        s += hex[p[i] & 15];
        if (i + 1 < n) s += (i % 16 == 15) ? ",\n" : ", ";
    }
    return s;
}

inline std::vector<uint8_t> parse_bytes(const std::string& text) {
    std::string s;
    s.reserve(text.size());
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '/' && i + 1 < text.size() && text[i + 1] == '/') {
            while (i < text.size() && text[i] != '\n') i++;
            s += '\n';
        } else if (text[i] == '/' && i + 1 < text.size() && text[i + 1] == '*') {
            size_t e = text.find("*/", i + 2);
            i = e == std::string::npos ? text.size() : e + 1;
            s += ' ';
        } else s += text[i];
    }
    size_t open = s.find('{'), close = s.rfind('}');
    if (open != std::string::npos)
        s = s.substr(open + 1, close > open ? close - open - 1 : std::string::npos);

    std::vector<uint8_t> out;
    for (size_t i = 0; i < s.size();) {
        if (!std::isalnum((unsigned char)s[i])) { i++; continue; }
        size_t j = i;
        while (j < s.size() && std::isalnum((unsigned char)s[j])) j++;
        std::string tok = s.substr(i, j - i);
        i = j;
        int base = 10;
        size_t off = 0;
        if (tok.size() > 2 && tok[0] == '0' && (tok[1] == 'x' || tok[1] == 'X')) { base = 16; off = 2; }
        else if (tok.size() > 1 && (tok[0] == 'x' || tok[0] == 'X')) { base = 16; off = 1; }
        else if (tok.find_first_not_of("0123456789") != std::string::npos) base = 16;
        char* end = nullptr;
        unsigned long v = std::strtoul(tok.c_str() + off, &end, base);
        if (*end || v > 255) throw std::invalid_argument("'" + tok + "' is not a byte");
        out.push_back((uint8_t)v);
    }
    return out;
}
