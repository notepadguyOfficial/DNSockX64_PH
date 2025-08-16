#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class SocketManager
{
public:
    SocketManager()
    {
        int result = WSAStartup(MAKEWORD(2, 2), &m_wsa);
        if (result != 0) {
            // Handle error (throw, log, or exit)
            // Example:
            // throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
        }
    }

    ~SocketManager() { WSACleanup(); }

    SOCKET createServer(const std::string& ip, int port)
    {
        SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
            return INVALID_SOCKET;

        if (listen(server, SOMAXCONN) == SOCKET_ERROR)
            return INVALID_SOCKET;

        return server;
    }

    SOCKET createClient(const std::string& ip, int port)
    {
        SOCKET client = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        if (connect(client, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
            return INVALID_SOCKET;

        return client;
    }

private:
    WSADATA m_wsa;
};