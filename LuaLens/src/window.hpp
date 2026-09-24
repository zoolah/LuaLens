#pragma once
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <wrl/client.h>
#include <wrl/event.h>
#include <WebView2.h>

#include <string>

#include "src/api.hpp"
#include "web/page.hpp"
#include "web/sample.hpp"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace app {

constexpr wchar_t TITLE[] = L"LuaLens";
inline HWND hwnd = nullptr;
inline HICON icon = nullptr;
inline ComPtr<ICoreWebView2Controller> controller;
inline ComPtr<ICoreWebView2> webview;
inline EventRegistrationToken msg_token{};

inline std::wstring utf16(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w((size_t)n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

inline std::string utf8(const wchar_t* s) {
    if (!s || !*s) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 1) return {};
    std::string out((size_t)n - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, s, -1, out.data(), n, nullptr, nullptr);
    return out;
}

inline HICON make_icon() {
    constexpr int N = 32;
    BITMAPV5HEADER bi{};
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = N;
    bi.bV5Height = -N;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    void* bits = nullptr;
    HDC hdc = GetDC(nullptr);
    HBITMAP color = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, hdc);
    if (!color || !bits) return LoadIconW(nullptr, IDI_APPLICATION);

    auto* px = static_cast<uint32_t*>(bits);
    ZeroMemory(px, N * N * sizeof(uint32_t));
    auto plot = [&](int x, int y, uint32_t c) {
        if ((unsigned)x < (unsigned)N && (unsigned)y < (unsigned)N) px[y * N + x] = c;
    };
    constexpr uint32_t bg = 0xFF1C1828, edge = 0xFF4A3870, letter = 0xFFF0E8FA;
    constexpr uint32_t ring = 0xFF9B7ED8, glass = 0xCC2A2040, handle = 0xFFB9A0E8, glint = 0x66F0E8FA;
    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            int dx = x < 3 ? 3 - x : x > 28 ? x - 28 : 0;
            int dy = y < 3 ? 3 - y : y > 28 ? y - 28 : 0;
            if (dx * dx + dy * dy > 10) continue;
            plot(x, y, (dx + dy) ? edge : bg);
        }
    }
    for (int y = 6; y <= 25; y++)
        for (int x = 5; x <= 9; x++)
            plot(x, y, letter);
    for (int y = 21; y <= 25; y++)
        for (int x = 5; x <= 16; x++)
            plot(x, y, letter);
    const float cx = 19.6f, cy = 13.2f;
    const float rin2 = 6.1f * 6.1f, rout2 = 8.4f * 8.4f;
    for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
            float dx = (float)x - cx, dy = (float)y - cy, d2 = dx * dx + dy * dy;
            if (d2 <= rin2) plot(x, y, (dx < -1.5f && dy < -1.0f) ? glint : glass);
            else if (d2 <= rout2) plot(x, y, ring);
        }
    }
    for (int i = 0; i <= 14; i++) {
        float t = (float)i / 14.f;
        int x = (int)(24.2f + t * 5.6f + 0.5f);
        int y = (int)(19.0f + t * 8.4f + 0.5f);
        for (int oy = -1; oy <= 1; oy++)
            for (int ox = -1; ox <= 1; ox++)
                plot(x + ox, y + oy, handle);
    }
    uint8_t mask[N * 4];
    memset(mask, 0xFF, sizeof mask);
    for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++)
            if (px[y * N + x] >> 24)
                mask[y * 4 + (x >> 3)] &= static_cast<uint8_t>(~(0x80u >> (x & 7)));

    HBITMAP mono = CreateBitmap(N, N, 1, 1, mask);
    ICONINFO ii{ TRUE, 0, 0, mono, color };
    HICON h = CreateIconIndirect(&ii);
    DeleteObject(color);
    if (mono) DeleteObject(mono);
    return h ? h : LoadIconW(nullptr, IDI_APPLICATION);
}

inline void resize_webview() {
    if (!controller || !hwnd) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    controller->put_Bounds(rc);
}

inline void handle_message(const std::string& msg) {
    auto p1 = msg.find('\x1e');
    auto p2 = p1 == std::string::npos ? p1 : msg.find('\x1e', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos || !webview) return;
    std::string id = msg.substr(0, p1);
    std::string cmd = msg.substr(p1 + 1, p2 - p1 - 1);
    std::string body = msg.substr(p2 + 1);

    std::string reply;
    if (cmd == "compile") {
        reply = api::with_id(id, api::compile_source(body));
    } else if (cmd == "sample") {
        reply = "{\"id\":\"" + id + "\",\"ok\":true,\"sample\":";
        json::str(reply, SAMPLE_LUA);
        reply += '}';
    } else if (cmd == "open") {
        wchar_t path[MAX_PATH]{};
        OPENFILENAMEW ofn{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = L"Lua (*.lua)\0*.lua\0Text (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
        ofn.lpstrDefExt = L"lua";
        if (!GetOpenFileNameW(&ofn)) {
            reply = "{\"id\":\"" + id + "\",\"ok\":false,\"cancel\":true}";
        } else {
            HANDLE fh = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (fh == INVALID_HANDLE_VALUE) {
                reply = "{\"id\":\"" + id + "\",\"ok\":false,\"error\":\"could not read file\"}";
            } else {
                LARGE_INTEGER sz{};
                GetFileSizeEx(fh, &sz);
                if (sz.QuadPart < 0 || sz.QuadPart > 8 * 1024 * 1024) {
                    CloseHandle(fh);
                    reply = "{\"id\":\"" + id + "\",\"ok\":false,\"error\":\"file too large\"}";
                } else {
                    std::string src((size_t)sz.QuadPart, '\0');
                    DWORD nread = 0;
                    BOOL ok = ReadFile(fh, src.data(), (DWORD)src.size(), &nread, nullptr);
                    CloseHandle(fh);
                    src.resize(nread);
                    if (!ok) {
                        reply = "{\"id\":\"" + id + "\",\"ok\":false,\"error\":\"could not read file\"}";
                    } else {
                        std::string fname = utf8(path);
                        auto slash = fname.find_last_of("\\/");
                        if (slash != std::string::npos) fname = fname.substr(slash + 1);
                        reply = "{\"id\":\"" + id + "\",\"ok\":true,\"name\":";
                        json::str(reply, fname);
                        reply += ",\"source\":";
                        json::str(reply, src);
                        reply += '}';
                    }
                }
            }
        }
    } else {
        reply = "{\"id\":\"" + id + "\",\"ok\":false,\"error\":\"unknown command\"}";
    }
    webview->PostWebMessageAsJson(utf16(reply).c_str());
}

inline void fail_webview(const wchar_t* msg) {
    MessageBoxW(hwnd, msg, TITLE, MB_OK | MB_ICONERROR);
    if (hwnd) DestroyWindow(hwnd);
}

inline HRESULT setup_webview(ICoreWebView2Controller* ctrl) {
    controller = ctrl;
    HRESULT hr = controller->get_CoreWebView2(&webview);
    if (FAILED(hr) || !webview) return hr;

    ComPtr<ICoreWebView2Settings> settings;
    if (SUCCEEDED(webview->get_Settings(&settings)) && settings) {
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreDefaultContextMenusEnabled(TRUE);
        settings->put_AreDevToolsEnabled(TRUE);
        ComPtr<ICoreWebView2Settings3> s3;
        if (SUCCEEDED(settings.As(&s3)) && s3)
            s3->put_AreBrowserAcceleratorKeysEnabled(FALSE);
    }

    ComPtr<ICoreWebView2Controller2> ctrl2;
    if (SUCCEEDED(controller.As(&ctrl2)) && ctrl2) {
        COREWEBVIEW2_COLOR bg{ 255, 17, 15, 22 };
        ctrl2->put_DefaultBackgroundColor(bg);
    }
    resize_webview();

    hr = webview->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                if (!args) return S_OK;
                LPWSTR text = nullptr;
                if (SUCCEEDED(args->TryGetWebMessageAsString(&text)) && text) {
                    handle_message(utf8(text));
                    CoTaskMemFree(text);
                }
                return S_OK;
            }).Get(),
        &msg_token);
    if (FAILED(hr)) return hr;

    auto html = utf16(page::document());
    return webview->NavigateToString(html.c_str());
}

inline std::wstring user_data_dir() {
    wchar_t base[MAX_PATH]{};
    if (FAILED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, base)))
        return L".\\LuaLensWebView";
    std::wstring dir = std::wstring(base) + L"\\LuaLens\\WebView2";
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
    return dir;
}

inline void start_webview() {
    auto data = user_data_dir();
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr, data.c_str(), nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    fail_webview(L"WebView2 runtime is missing.\nInstall Microsoft Edge WebView2, then try again.");
                    return S_OK;
                }
                env->CreateCoreWebView2Controller(
                    hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [](HRESULT result, ICoreWebView2Controller* ctrl) -> HRESULT {
                            if (FAILED(result) || !ctrl) {
                                fail_webview(L"could not create WebView2 controller");
                                return S_OK;
                            }
                            if (FAILED(setup_webview(ctrl)))
                                fail_webview(L"could not load the LuaLens page");
                            return S_OK;
                        }).Get());
                return S_OK;
            }).Get());
    if (FAILED(hr))
        fail_webview(L"WebView2 runtime is missing.\nInstall Microsoft Edge WebView2, then try again.");
}

inline void apply_dark_title(HWND w) {
    BOOL dark = TRUE;
    DwmSetWindowAttribute(w, 20, &dark, sizeof dark);
}

inline LRESULT CALLBACK proc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_SIZE:
        resize_webview();
        return 0;
    case WM_SETFOCUS:
        if (controller)
            controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
        return 0;
    case WM_GETMINMAXINFO: {
        auto* m = reinterpret_cast<MINMAXINFO*>(lp);
        m->ptMinTrackSize.x = 800;
        m->ptMinTrackSize.y = 500;
        return 0;
    }
    case WM_DESTROY:
        if (controller) {
            controller->Close();
            controller = nullptr;
        }
        webview = nullptr;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(w, msg, wp, lp);
    }
}

inline void dpi_aware() {
    auto fn = reinterpret_cast<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>(
        GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetProcessDpiAwarenessContext"));
    if (fn) fn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    else SetProcessDPIAware();
}

inline int run() {
    dpi_aware();
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 1;

    icon = make_icon();
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hIcon = icon;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(18, 17, 15));
    wc.lpszClassName = L"LuaLens.Window";
    RegisterClassW(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    int ww = 1280, wh = 800;
    hwnd = CreateWindowExW(0, wc.lpszClassName, TITLE, WS_OVERLAPPEDWINDOW,
        (sw - ww) / 2, (sh - wh) / 2, ww, wh, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) {
        CoUninitialize();
        return 1;
    }
    apply_dark_title(hwnd);
    SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    start_webview();

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (icon) DestroyIcon(icon);
    CoUninitialize();
    return (int)msg.wParam;
}

}

#endif
