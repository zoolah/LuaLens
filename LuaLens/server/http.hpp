#pragma once
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include "src/api.hpp"
#include "web/assets.hpp"
#include "web/page.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using sock_t = SOCKET;
inline void close_sock(sock_t s) { closesocket(s); }
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
using sock_t = int;
inline void close_sock(sock_t s) { close(s); }
#endif

namespace server {

struct Response { int code = 200; const char* type = "text/plain; charset=utf-8"; std::string body; };

inline Response json_ok(std::string body) {
    return { 200, "application/json; charset=utf-8", std::move(body) };
}

inline bool read_request(sock_t c, std::string& method, std::string& path, std::string& body) {
    constexpr size_t MAX_BODY = 16u << 20;
    std::string buf;
    char tmp[8192];
    size_t he;
    while ((he = buf.find("\r\n\r\n")) == std::string::npos) {
        if (buf.size() > 65536) return false;
        int n = (int)recv(c, tmp, sizeof tmp, 0);
        if (n <= 0) return false;
        buf.append(tmp, n);
    }
    size_t sp1 = buf.find(' '), sp2 = buf.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) return false;
    method = buf.substr(0, sp1);
    path = buf.substr(sp1 + 1, sp2 - sp1 - 1);
    path = path.substr(0, path.find('?'));

    std::string head = buf.substr(0, he);
    for (char& ch : head) ch = (char)std::tolower((unsigned char)ch);
    size_t len = 0, cl = head.find("content-length:");
    if (cl != std::string::npos) len = (size_t)std::strtoull(head.c_str() + cl + 15, nullptr, 10);
    if (len > MAX_BODY) return false;

    body = buf.substr(he + 4);
    while (body.size() < len) {
        int n = (int)recv(c, tmp, sizeof tmp, 0);
        if (n <= 0) return false;
        body.append(tmp, n);
    }
    body.resize(len);
    return true;
}

inline Response route(const std::string& method, const std::string& path, const std::string& body) {
    if (method == "GET" && path == "/") return { 200, "text/html; charset=utf-8", INDEX_HTML };
    if (method == "GET" && path == "/style.css") return { 200, "text/css; charset=utf-8", STYLE_CSS };
    if (method == "GET" && path == "/disasm.js") return { 200, "text/javascript; charset=utf-8", DISASM_JS };
    if (method == "GET" && path == "/graph.js") return { 200, "text/javascript; charset=utf-8", GRAPH_JS };
    if (method == "GET" && path == "/sample") return { 200, "text/plain; charset=utf-8", SAMPLE_LUA };
    if (method == "POST" && path == "/compile") return json_ok(compile_source(body));
    if (method == "POST" && path == "/disassemble") return json_ok(disassemble(body));
    return { 404, "text/plain; charset=utf-8", "Not found" };
}

inline void send_all(sock_t c, const std::string& out) {
    for (size_t sent = 0; sent < out.size();) {
        int n = (int)send(c, out.data() + sent, (int)(out.size() - sent), 0);
        if (n <= 0) break;
        sent += n;
    }
}

inline void handle(sock_t c) {
#ifdef _WIN32
    DWORD ms = 10000;
    setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof ms);
#else
    timeval tv{ 10, 0 };
    setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
#endif
    std::string method, path, body;
    if (read_request(c, method, path, body)) {
        Response r = route(method, path, body);
        send_all(c, "HTTP/1.1 " + std::to_string(r.code) + (r.code == 200 ? " OK" : " Not Found") +
            "\r\nContent-Type: " + r.type +
            "\r\nContent-Length: " + std::to_string(r.body.size()) +
            "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n" + r.body);
    }
    close_sock(c);
}

inline std::atomic<bool> stopping{ false };page::document() };
    if (method == "GET" && path == "/sample") return { 200, "text/plain; charset=utf-8", SAMPLE_LUA };
    if (method == "POST" && path == "/compile") return json_ok(api::compile_source(body));
    if (method == "POST" && path == "/disassemble") return json_ok(api::
    startup_error = msg;
    std::cerr << msg << '\n';
}

inline void request_stop() {
    stopping.store(true);
    sock_t s = listener.exchange((sock_t)-1);
    if (s != (sock_t)-1) close_sock(s);
}

inline int run(uint16_t port = 80) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) { fail("WSAStartup failed."); return 1; }
#endif
    if (stopping.load()) return 0;
    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == (sock_t)-1) { fail("socket() failed."); return 1; }
#ifndef _WIN32
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(srv, (sockaddr*)&addr, sizeof addr) != 0 || listen(srv, 16) != 0) {
        close_sock(srv);
        fail("could not listen on port " + std::to_string(port));
        return 1;
    }
    listen_port.store(port);
    listener.store(srv);
    if (stopping.load()) { request_stop(); return 0; }
    if (on_listening) on_listening(port);
    std::cout << "http://localhost" << (port == 80 ? "" : ":" + std::to_string(port)) << "/\n";
    while (!stopping.load()) {
        sock_t s = listener.load();
        if (s == (sock_t)-1) break;
        sock_t c = accept(s, nullptr, nullptr);
        if (c == (sock_t)-1) {
            if (stopping.load()) break;
            continue;
        }
        std::thread(handle, c).detach();
    }
    request_stop();
    return 0;
}

}
