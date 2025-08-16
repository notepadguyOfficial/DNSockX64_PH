#pragma once

#include <string>
#include "../Ini/Parser.h"

struct InfoConfig
{
    int IocpMax{};
    int SQLMax{};
    int WorkerThreadMax{};
};

struct ConnectionConfig
{
    int ClientPort{};
    int ServerPort{};
};

struct DBConfig
{
    std::string DBIP;
    int DBPort{};
    std::string DBID;
    std::string DBName;
    std::string DBPassword;
    std::string DBTablePasswordName;
};

struct ResourceConfig
{
    std::string Path;
};


class Config
{
public:
    explicit Config(const std::string& filePath) : m_ini(filePath) {}

    bool load()
    {
        info.IocpMax = m_ini.get_int("Info", "IocpMax", 100);
        info.SQLMax = m_ini.get_int("Info", "SQLMax", 4);
        info.WorkerThreadMax = m_ini.get_int("Info", "WorkerThreadMax", 2);

        connection.ClientPort = m_ini.get_int("Connection", "ClientPort", 0);
        connection.ServerPort = m_ini.get_int("Connection", "ServerPort", 0);

        db.DBIP = m_ini.get_string("DB_DNMembership", "DBIP", "127.0.0.1");
        db.DBPort = m_ini.get_int("DB_DNMembership", "DBPort", 1433);
        db.DBID = m_ini.get_string("DB_DNMembership", "DBID", "");
        db.DBName = m_ini.get_string("DB_DNMembership", "DBName", "");
        db.DBPassword = m_ini.get_string("DB_DNMembership", "DBPassword", "");
        db.DBTablePasswordName = m_ini.get_string("DB_DNMembership", "DBTablePasswordName", "");

        resource.Path = m_ini.get_string("Resource", "Path", "");

        return true;
    }

    InfoConfig info;
    ConnectionConfig connection;
    DBConfig db;
    ResourceConfig resource;

private:
    Parser m_ini;
};