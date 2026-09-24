#pragma once
#ifdef _WIN32
#include "server/http.hpp"
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

namespace tray {

constexpr UINT WM_TRAY = WM_APP + 1;
constexpr UINT WM_SERVER_FAILED = WM_APP + 2;
constexpr UINT WM_SERVER_READY = WM_APP + 3;
constexpr UINT CMD_OPEN = 1;
constexpr UINT CMD_EXIT = 2;
constexpr wchar_t TITLE[] = L"lua 5.1";

inline HWND hwnd = nullptr;
inline HICON icon = nullptr;

inline void page_url(wchar_t (&url)[64]) {
    unsigned port = server::listen_port.load();
    if (port == 80) wcscpy_s(url, L"http://localhost/");
    else swprintf_s(url, L"http://localhost:%u/", port);
}

inline void open_page() {
    wchar_t url[64];
    page_url(url);
    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

inline NOTIFYICONDATAW base_nid() {
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    return nid;
}

inline void exit_app(UINT code) {
    if (hwnd) {
        auto nid = base_nid();
        Shell_NotifyIconW(NIM_DELETE, &nid);
    }
    if (icon) DestroyIcon(icon);
    server::request_stop();
    ExitProcess(code);
}

inline void show_menu() {
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, CMD_OPEN, L"Open in browser");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, CMD_EXIT, L"Exit");
    POINT pt{};
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
    PostMessageW(hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
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
    constexpr uint32_t fill = 0xFF4A3870, bar = 0xFFF0E8FA;
    for (int y = 2; y < 30; y++)
        for (int x = 2; x < 30; x++)
            px[y * N + x] = fill;
    auto stripe = [&](int top, int right) {
        for (int y = top; y < top + 3; y++)
            for (int x = 7; x < right; x++)
                px[y * N + x] = bar;
    };
    stripe(9, 25);
    stripe(15, 25);
    stripe(21, 20);

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

inline void set_tip(NOTIFYICONDATAW& nid) {
    wchar_t url[64];
    page_url(url);
    swprintf_s(nid.szTip, L"lua 5.1 — %s", url);
}

inline void note_ready(unsigned) {
    auto nid = base_nid();
    nid.uFlags = NIF_TIP | NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
    wcscpy_s(nid.szInfoTitle, TITLE);
    wcscpy_s(nid.szInfo, L"right-click to open or exit");
    set_tip(nid);
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

inline LRESULT CALLBACK proc(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_TRAY:
        if (lp == WM_RBUTTONUP) show_menu();
        else if (lp == WM_LBUTTONDBLCLK) open_page();
        return 0;
    case WM_COMMAND:
        if (LOWORD(wp) == CMD_OPEN) open_page();
        else if (LOWORD(wp) == CMD_EXIT) exit_app(0);
        return 0;
    case WM_SERVER_READY:
        note_ready((unsigned)wp);
        return 0;
    case WM_SERVER_FAILED:
        exit_app(1);
        return 0;
    default:
        return DefWindowProcW(w, msg, wp, lp);
    }
}

inline void server_thread() {
    int rc = server::run();
    if (rc != 0 && hwnd) {
        const char* msg = server::startup_error.empty() ? "server stopped" : server::startup_error.c_str();
        MessageBoxA(nullptr, msg, "lua 5.1", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
        PostMessageW(hwnd, WM_SERVER_FAILED, 0, 0);
    }
}

inline int run() {
    WNDCLASSW wc{};
    wc.lpfnWndProc = proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"Lua51Disasm.Tray";
    RegisterClassW(&wc);

    hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName,
        TITLE, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return 1;

    icon = make_icon();
    auto nid = base_nid();
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = icon;
    set_tip(nid);
    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        MessageBoxW(nullptr, L"could not add tray icon", TITLE, MB_OK | MB_ICONERROR);
        return 1;
    }

    server::on_listening = [](uint16_t port) {
        if (hwnd) PostMessageW(hwnd, WM_SERVER_READY, port, 0);
    };
    std::thread(server_thread).detach();

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    exit_app(0);
    return 0;
}

}
#endif
