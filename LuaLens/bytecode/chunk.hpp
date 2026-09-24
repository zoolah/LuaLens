#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct Ins { uint32_t op, A, B, C, Bx; int32_t sBx; };
struct Const { uint8_t type; double n; std::string s; };
struct Local { std::string name; uint32_t startpc, endpc; };

struct Chunk {
    std::string name;
    int32_t first = 0, last = 0;
    uint8_t nups = 0, args = 0, varg = 0, stack = 0;
    std::vector<Ins> code;
    std::vector<Const> k;
    std::vector<Chunk> protos;
    std::vector<uint32_t> lines;
    std::vector<Local> locals;
    std::vector<std::string> upvals;
};

constexpr uint32_t SBX_BIAS = 131071;
enum ConstType : uint8_t { K_NIL = 0, K_BOOL = 1, K_NUMBER = 3, K_STRING = 4 };
