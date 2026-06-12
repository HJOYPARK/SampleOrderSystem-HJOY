#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

inline std::string currentDateTimeString() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

inline std::string makeBar(int filled, int total = 10, char on = L'\xDB', char off = L'\xB0') {
    std::string bar;
    for (int i = 0; i < total; ++i)
        bar += (i < filled) ? on : off;
    return bar;
}

inline void printSeparator() {
    std::cout << "================================================================\n";
}

inline char getChar(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line.empty() ? '\0' : static_cast<char>(std::toupper(static_cast<unsigned char>(line[0])));
}

inline std::string getString(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

inline int getInt(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    try { return std::stoi(line); } catch (...) { return -1; }
}
