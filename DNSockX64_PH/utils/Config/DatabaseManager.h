#pragma once
#include <string>
#include <windows.h>
#include <sqlext.h>

class DBManager
{
public:
    DBManager(const std::string& ip, int port,
        const std::string& user,
        const std::string& password,
        const std::string& dbname)
        : m_ip(ip), m_port(port), m_user(user),
        m_password(password), m_dbname(dbname),
        m_env(SQL_NULL_HANDLE), m_conn(SQL_NULL_HANDLE)
    {
    }

    bool connect()
    {
        if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env) != SQL_SUCCESS)
            return false;
        SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

        if (SQLAllocHandle(SQL_HANDLE_DBC, m_env, &m_conn) != SQL_SUCCESS)
            return false;

        std::string connStr =
            "DRIVER={ODBC Driver 17 for SQL Server};SERVER=" + m_ip + "," + std::to_string(m_port) +
            ";DATABASE=" + m_dbname +
            ";UID=" + m_user +
            ";PWD=" + m_password + ";";

        SQLCHAR retConString[1024];
        SQLRETURN ret = SQLDriverConnectA(
            m_conn, NULL,
            (SQLCHAR*)connStr.c_str(),
            SQL_NTS, retConString, 1024, NULL, SQL_DRIVER_NOPROMPT);

        return (SQL_SUCCEEDED(ret));
    }

    bool executeLogin(const std::string& account,
        const std::string& passwordHash,
        const std::string& passColumn)
    {
        SQLHSTMT stmt;
        if (SQLAllocHandle(SQL_HANDLE_STMT, m_conn, &stmt) != SQL_SUCCESS)
            return false;

        std::string query = "SELECT * FROM DNMembership.Accounts WHERE AccountName=? AND " + passColumn + "=?";

        SQLPrepareA(stmt, (SQLCHAR*)query.c_str(), SQL_NTS);
        SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            0, 0, (SQLPOINTER)account.c_str(), 0, NULL);
        SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
            0, 0, (SQLPOINTER)passwordHash.c_str(), 0, NULL);

        SQLRETURN ret = SQLExecute(stmt);

        bool success = false;
        if (SQL_SUCCEEDED(ret))
        {
            if (SQLFetch(stmt) == SQL_SUCCESS)
                success = true;
        }

        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        return success;
    }

    ~DBManager()
    {
        if (m_conn != SQL_NULL_HANDLE) SQLDisconnect(m_conn);
        if (m_conn != SQL_NULL_HANDLE) SQLFreeHandle(SQL_HANDLE_DBC, m_conn);
        if (m_env != SQL_NULL_HANDLE) SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    }

private:
    std::string m_ip;
    int m_port;
    std::string m_user;
    std::string m_password;
    std::string m_dbname;

    SQLHENV m_env;
    SQLHDBC m_conn;
};