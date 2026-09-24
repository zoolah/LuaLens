#pragma once

inline const char* SAMPLE_LUA = R"LUA(print("hi bro")

local function greet(name)
  print("hello, " .. name)
end

local function make_counter()
  local n = 0
  return function()
    n = n + 1
    return n
  end
end

greet("world")
local nxt = make_counter()
print(nxt())
print(nxt())
)LUA";
