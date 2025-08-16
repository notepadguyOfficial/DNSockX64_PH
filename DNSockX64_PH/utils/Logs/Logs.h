#pragma once

#include <string>
#include <format>
#include <mutex>
#include <unordered_set>
#include <iostream>
#include <chrono>
#include <filesystem>

#ifdef _WIN32
	#include <Windows.h>
#endif

#define LOG(fmt, ...)       Logs::log(__FILE__, __LINE__, LOG_LEVEL::Info, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logs::log(__FILE__, __LINE__, LOG_LEVEL::Info, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) Logs::log(__FILE__, __LINE__, LOG_LEVEL::Debug, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logs::log(__FILE__, __LINE__, LOG_LEVEL::Error, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)  Logs::log(__FILE__, __LINE__, LOG_LEVEL::Warning, fmt, ##__VA_ARGS__)


enum class LOG_LEVEL
{
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

class Logs
{
public:
	static Logs& instance() {
		static Logs logs;
		return logs;
	}

	Logs& showFileName(bool show = true) { m_showFileName = show; return *this; }
	Logs& showLineNumber(bool show = true) { m_showLineNumber = show; return *this; }
	Logs& showTimeStamp(bool show = true) { m_showTimeStamp = show; return *this; }
	Logs& enableColors(bool enable = true) { m_enableColors = enable; return *this; }

	Logs& exclude(LOG_LEVEL level) { m_excludedLevels.insert(level); return *this; }
	Logs& include(LOG_LEVEL level) { m_excludedLevels.erase(level); return *this; }
	Logs& clearExclusions() { m_excludedLevels.clear(); return *this; }

	template<typename... Args>
	static void log(const char* file, int line, LOG_LEVEL level, const std::string& fmt, Args&&... args) {
		if constexpr (sizeof...(args) == 0) {
			instance().writeLog(file, line, level, fmt);
		}
		else {
			try
			{
				std::string formatted = std::vformat(fmt, std::make_format_args(args...));
				instance().writeLog(file, line, level, formatted);
			}
			catch (const std::exception&)
			{
				instance().writeLog(file, line, level, fmt + " [FORMAT ERROR]");
			}
		}
	}

private:
	Logs() = default;
	~Logs() = default;

	void writeLog(const char* file, int line, LOG_LEVEL level, const std::string& message);
	void writeToConsole(const std::string& formattedMessage, LOG_LEVEL level);
	std::string formatLogMessage(const char* file, int line, LOG_LEVEL level, const std::string& message);
	std::string getLevelString(LOG_LEVEL level);
	std::string getCurrentTimeString();

#ifdef _WIN32
	WORD getLevelColor(LOG_LEVEL level);
#endif

	bool m_showFileName = true;
	bool m_showLineNumber = true;
	bool m_showTimeStamp = false;
	bool m_enableColors = true;
	std::unordered_set<LOG_LEVEL> m_excludedLevels;

	std::mutex m_logMutex;
};