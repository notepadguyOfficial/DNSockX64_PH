#include "Logs.h"

void Logs::writeLog(const char* file, int line, LOG_LEVEL level, const std::string& message)
{
    if (m_excludedLevels.find(level) != m_excludedLevels.end()) return;

    const std::lock_guard<std::mutex> lock(m_logMutex);
    std::string formattedMessage = formatLogMessage(file, line, level, message);
    writeToConsole(formattedMessage, level);
}

void Logs::writeToConsole(const std::string& formattedMessage, LOG_LEVEL level)
{
#ifdef _WIN32
    if (m_enableColors)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE)
        {
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            if (GetConsoleScreenBufferInfo(hConsole, &consoleInfo))
            {
                WORD savedAttributes = consoleInfo.wAttributes;

                size_t pos = 0;
                while (true)
                {
                    size_t start = formattedMessage.find('[', pos);
                    if (start == std::string::npos) break;
                    size_t end = formattedMessage.find(']', start);
                    if (end == std::string::npos) break;

                    std::cout << formattedMessage.substr(pos, start - pos);

                    std::cout << "[";

                    std::string inside = formattedMessage.substr(start + 1, end - start - 1);
                    WORD color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Cyan
                    if (inside == "DEBUG" || inside == "INFO" || inside == "WARN" || inside == "ERROR")
                        color = getLevelColor(level);

                    SetConsoleTextAttribute(hConsole, color);
                    std::cout << inside;

                    SetConsoleTextAttribute(hConsole, savedAttributes);
                    std::cout << "]";

                    pos = end + 1;
                }

                std::cout << formattedMessage.substr(pos) << std::endl;
                return;
            }
        }
    }
#endif
    std::cout << formattedMessage << std::endl;
}

std::string Logs::formatLogMessage(const char* file, int line, LOG_LEVEL level, const std::string& message)
{
    std::string result;

    if (m_showTimeStamp)
    {
        result += "[" + getCurrentTimeString() + "] ";
    }

    if (m_showFileName && file && std::strlen(file) > 0)
    {
        std::string filename = std::filesystem::path(file).filename().string();
        result += "[" + filename;
        if (m_showLineNumber && line > 0)
        {
            result += ":" + std::to_string(line);
        }
        result += "] ";
    }

    result += "[" + getLevelString(level) + "] ";
    result += message;

    return result;
}

std::string Logs::getLevelString(LOG_LEVEL level)
{
    switch (level)
    {
    case LOG_LEVEL::Debug:   return "DEBUG";
    case LOG_LEVEL::Info:    return "INFO";
    case LOG_LEVEL::Warning: return "WARN";
    case LOG_LEVEL::Error:   return "ERROR";
    default:                return "LOG";
    }
}

std::string Logs::getCurrentTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    struct tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t);
#else
    localtime_r(&time_t, &tm_buf);
#endif

    char buffer[9];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm_buf);
    return buffer;
}

#ifdef _WIN32
WORD Logs::getLevelColor(LOG_LEVEL level)
{
    switch (level)
    {
    case LOG_LEVEL::Debug:   return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY; // Magenta
    case LOG_LEVEL::Info:    return FOREGROUND_GREEN | FOREGROUND_INTENSITY; // Green
    case LOG_LEVEL::Warning: return FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY; // Yellow
    case LOG_LEVEL::Error:   return FOREGROUND_RED | FOREGROUND_INTENSITY; // Red
    default:                return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; // White
    }
}
#endif