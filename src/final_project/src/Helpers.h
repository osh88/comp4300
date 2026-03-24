#pragma once

#include <SFML/System/String.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#ifdef __linux__
    #include <unistd.h>
    #include <libgen.h> // For dirname
#elif __APPLE__
    #include <mach-o/dyld.h> // For _NSGetExecutablePath
    #include <libgen.h> // For dirname
#elif _WIN32
    #include <windows.h>
#endif

#include <filesystem> // Required for std::filesystem
namespace fs = std::filesystem; // Alias for convenience

class Helpers {
public:

#ifdef __linux__
    static std::string getExecutablePath() {
        std::vector<char> buffer(1024);
        
        ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (len != -1) {
            return std::string(buffer.data(), len);
        }

        return "";
    }

#elif __APPLE__
    static std::string getExecutablePath() {
        std::vector<char> buffer(1024);
        
        uint32_t size = buffer.size();
        if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
            return std::string(buffer.data());
        } else {
            buffer.resize(size); // Resize if buffer was too small
            _NSGetExecutablePath(buffer.data(), &size);
            return std::string(buffer.data());
        }
        
        return "";
    }

#elif _WIN32
    static std::string getExecutablePath() {
        // Determine the required buffer size
        DWORD bufferSize = MAX_PATH;
        std::vector<char> buffer(bufferSize);

        // Call GetModuleFileName to get the path
        DWORD result = GetModuleFileNameA(NULL, buffer.data(), bufferSize);

        // Handle potential errors or insufficient buffer size
        while (result == bufferSize && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            bufferSize *= 2; // Double the buffer size
            buffer.resize(bufferSize);
            result = GetModuleFileNameA(NULL, buffer.data(), bufferSize);
        }

        if (result == 0) {
            // Handle error, e.g., print an error message or throw an exception
            std::cerr << "Error getting executable path: " << GetLastError() << std::endl;
            return "";
        }

        return std::string(buffer.data());
    }
#endif

    static std::string getExecutableDirectory() {
        std::string execPath = getExecutablePath();
        if (!execPath.empty()) {
            return std::filesystem::path(execPath).parent_path().string();
            //return dirname(const_cast<char*>(execPath.c_str())); // dirname modifies the string
        }
        return "";
    }

    static std::vector<std::string> getDirectoryItems(const std::string& path) {
        std::vector<std::string> items;
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                items.push_back(entry.path().filename().string());
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error accessing directory " << path << ": " << e.what() << std::endl;
        }
        return items;
    }

    static sf::String S(const std::string & s) {
        return sf::String::fromUtf8(s.begin(), s.end());
    }
};