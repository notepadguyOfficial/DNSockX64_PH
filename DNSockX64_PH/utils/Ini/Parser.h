#pragma once

#include <string>
#include <Windows.h>

class Parser {
public:
	explicit Parser(const std::string& path)
		: m_filePath(path) { }

    std::string get_string(const std::string& section, const std::string& key, const std::string& defaultValue = "")
    {
        char buffer[512] = { 0 };
        GetPrivateProfileStringA(
            section.c_str(),
            key.c_str(),
            defaultValue.c_str(),
            buffer,
            sizeof(buffer),
            m_filePath.c_str()
        );
        return std::string(buffer);
    }

    int get_int(const std::string& section, const std::string& key, int defaultValue = 0)
    {
        return GetPrivateProfileIntA(section.c_str(), key.c_str(), defaultValue, m_filePath.c_str());
    }

    bool write_string(const std::string& section, const std::string& key, const std::string& value)
    {
        return WritePrivateProfileStringA(section.c_str(), key.c_str(), value.c_str(), m_filePath.c_str());
    }

    bool write_int(const std::string& section, const std::string& key, int value)
    {
        return write_string(section, key, std::to_string(value));
    }

private:
	std::string m_filePath;
};