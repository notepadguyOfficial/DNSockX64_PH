#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <openssl/evp.h>

#include "utils/Logs/Logs.h"
#include "utils/Config/Config.h"
#include "utils/Config/DatabaseManager.h"
#include "utils/Config/SocketManager.h"
#include "version.h"


std::string md5(const std::string& input)
{
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_md5();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;

    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, input.data(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &length);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < length; ++i)
        oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

int main(int argc, char** argv[])
{
    Logs::instance()
        .showTimeStamp(true)
        .showFileName(true)
        .showLineNumber(true)
        .enableColors(true);

    Config cfg("./Config/DNSocket.ini");

    if (!cfg.load())
    {
        LOG_ERROR("Failed to load configuration file.");
        exit(EXIT_FAILURE);
    }

    DBManager db(cfg.db.DBIP, cfg.db.DBPort, cfg.db.DBID, cfg.db.DBPassword, cfg.db.DBName);

    if (!db.connect())
    {
        LOG_ERROR("Connection Failed!");
        exit(EXIT_FAILURE);
    }

    SocketManager sm;
    SOCKET server = sm.createServer(cfg.db.DBIP, cfg.connection.ServerPort);

    if (server == INVALID_SOCKET)
    {
        LOG_ERROR("Execute CListenSocket::bInitialize Failed!");
        exit(EXIT_FAILURE);
    }

    SOCKET client = sm.createClient(cfg.db.DBIP, cfg.connection.ClientPort);
    if (client == INVALID_SOCKET)
    {
        LOG_ERROR("Client connect failed!");
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    std::vector<SOCKET> sockets;
    sockets.push_back(server);

    LOG_INFO("[IocpMax] : {} | [SQLMax] : {} | [WorkerThreadMax] : {}",
        cfg.info.IocpMax, cfg.info.SQLMax, cfg.info.WorkerThreadMax);

    LOG_INFO("[ClientPort] : {} | [ServerPort] : {}",
        cfg.connection.ClientPort, cfg.connection.ServerPort);

    LOG_INFO("[IP Address] : {} | [Port] : {} | [DBID] : {} | [DBName] : {} | [DBPassword] : {} | [DBTablePass] : {}",
        cfg.db.DBIP, cfg.db.DBPort, cfg.db.DBID, cfg.db.DBName, cfg.db.DBPassword, cfg.db.DBTablePasswordName);

    LOG_INFO("[Game Resource] : {}", cfg.resource.Path);

    while (true)
    {
        FD_ZERO(&readfds);
        for (auto s : sockets) FD_SET(s, &readfds);

        timeval tv{ 1,0 };
        int activity = select(0, &readfds, NULL, NULL, &tv);

        if (activity > 0)
        {
            for (auto it = sockets.begin(); it != sockets.end(); )
            {
                SOCKET sock = *it;
                if (FD_ISSET(sock, &readfds))
                {
                    if (sock == server)
                    {
                        sockaddr_in addr;
                        int len = sizeof(addr);
                        SOCKET newSock = accept(server, (sockaddr*)&addr, &len);
                        if (newSock != INVALID_SOCKET)
                        {
                            sockets.push_back(newSock);
                            LOG_INFO("New connection accepted");
                        }
                    }
                    else
                    {
                        char buffer[20480];
                        int bytes = recv(sock, buffer, sizeof(buffer), 0);
                        if (bytes <= 0)
                        {
                            closesocket(sock);
                            it = sockets.erase(it);
                            continue;
                        }

                        std::string msg(buffer, bytes);
                        LOG_INFO("WSARecv(): {}", msg);

                        // Very simplified parser
                        std::vector<std::string> parts;
                        size_t start = 0, pos;
                        while ((pos = msg.find("'", start)) != std::string::npos)
                        {
                            parts.push_back(msg.substr(start, pos - start));
                            start = pos + 1;
                        }
                        parts.push_back(msg.substr(start));

                        if (parts.size() == 10 && parts[5] == "I")
                        {
                            LOG_INFO("Execute CDNManager::CDNExecute()->DNMembership Success!!");

                            std::string account = parts[2];
                            std::string passwordHash = md5(parts[3]);

                            bool ok = db.executeLogin(account, passwordHash, cfg.db.DBTablePasswordName);
                            parts[3] = ok ? "0" : "1";

                            // rebuild message
                            msg.clear();
                            for (size_t i = 0; i < parts.size(); ++i)
                            {
                                msg += parts[i];
                                if (i < parts.size() - 1) msg += "'";
                            }
                        }

                        send(client, msg.c_str(), msg.size(), 0);
                        int rbytes = recv(client, buffer, sizeof(buffer), 0);
                        if (rbytes > 0)
                        {
                            std::string response(buffer, rbytes);
                            send(sock, response.c_str(), response.size(), 0);
                            LOG_INFO("WSASend(): {}", response);
                        }
                    }
                }
                ++it;
            }
        }
    }

    exit(EXIT_SUCCESS);
}