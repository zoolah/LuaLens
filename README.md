# LuaLens

Local tool that compiles Lua 5.1, dumps the chunk, and draws the function graph in its own window.

Windows desktop app. Uses Edge WebView2 

## Build

Needs CMake 3.20+, a C++20 compiler (MSVC on Windows), and the [WebView2 runtime](https://developer.microsoft.com/microsoft-edge/webview2/) (ships with Microsoft Edge). First configure downloads the WebView2 SDK.

```
build.bat
```

Or:

```
cmake -B build -A x64
cmake --build build --config Release
```

The exe is `build/Release/LuaLens.exe` (Visual Studio generator) or `build/LuaLens.exe` (Ninja).

Visual Studio: **File → Open → Folder** on this repo, or generate a solution:

```
cmake -B build -G "Visual Studio 17 2022" -A x64
```

## Use

1. Run the exe. A window opens with the UI.
2. Paste Lua 5.1 source, then **compile & disassemble**.

Ctrl+Enter compiles and disassembles. Scroll pans, ctrl-scroll zooms, drag a function header to move it.

## Layout

```
LuaLens/
  src/            entry + native window
  bytecode/       chunk layout, reader, lua 5.1 compiler wrapper
  server/         json, byte parse (non-Windows http fallback)
  web/            page assets (embedded)
  third_party/lua51/   Lua 5.1.5 (compiler core only)
```

## License

MIT. Lua 5.1.5 is MIT (Lua.org, PUC-Rio).
