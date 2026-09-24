cmake_minimum_required(VERSION 3.20)
project(LuaLens LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/LuaLens")

file(GLOB LUA51_SRC CONFIGURE_DEPENDS "${SRC_ROOT}/third_party/lua51/*.c")
set(HDRS
  "${SRC_ROOT}/src/api.hpp"
  "${SRC_ROOT}/src/window.hpp"
  "${SRC_ROOT}/bytecode/chunk.hpp"
  "${SRC_ROOT}/bytecode/compiler.hpp"
  "${SRC_ROOT}/bytecode/reader.hpp"
  "${SRC_ROOT}/server/bytes.hpp"
  "${SRC_ROOT}/server/http.hpp"
  "${SRC_ROOT}/server/json.hpp"
  "${SRC_ROOT}/web/assets.hpp"
  "${SRC_ROOT}/web/css.hpp"
  "${SRC_ROOT}/web/disasm.hpp"
  "${SRC_ROOT}/web/graph.hpp"
  "${SRC_ROOT}/web/html.hpp"
  "${SRC_ROOT}/web/page.hpp"
  "${SRC_ROOT}/web/sample.hpp"
)

add_executable(LuaLens "${SRC_ROOT}/src/main.cpp" ${LUA51_SRC} ${HDRS})

target_include_directories(LuaLens PRIVATE
  "${SRC_ROOT}"
  "${SRC_ROOT}/src"
  "${SRC_ROOT}/third_party/lua51"
)

if(WIN32)
  include(FetchContent)
  FetchContent_Declare(webview2
    URL "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2/1.0.2849.39"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(webview2)

  if(CMAKE_SYSTEM_PROCESSOR MATCHES "[Aa][Rr][Mm]64")
    set(WV2_ARCH arm64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WV2_ARCH x64)
  else()
    set(WV2_ARCH x86)
  endif()

  set_target_properties(LuaLens PROPERTIES WIN32_EXECUTABLE TRUE)
  target_include_directories(LuaLens PRIVATE "${webview2_SOURCE_DIR}/build/native/include")
  target_link_libraries(LuaLens PRIVATE
    "${webview2_SOURCE_DIR}/build/native/${WV2_ARCH}/WebView2LoaderStatic.lib"
    ole32 oleaut32 uuid version dwmapi shell32 user32 gdi32 advapi32 comdlg32
  )
  target_compile_definitions(LuaLens PRIVATE _CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
  if(MSVC)
    target_link_options(LuaLens PRIVATE /ENTRY:mainCRTStartup)
  endif()
endif()

if(MSVC)
  set_source_files_properties(${LUA51_SRC} PROPERTIES COMPILE_OPTIONS "/W0")
endif()

source_group(TREE "${SRC_ROOT}" FILES "${SRC_ROOT}/src/main.cpp" ${LUA51_SRC} ${HDRS})
