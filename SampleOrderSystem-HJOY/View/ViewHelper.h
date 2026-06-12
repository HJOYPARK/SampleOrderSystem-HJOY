#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>

// UTF-8 display width: Korean/CJK = 2 columns, ASCII = 1 column
inline int utf8DisplayWidth(const std::string& s) {
    int width = 0;
    for (size_t i = 0; i < s.size(); ) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        uint32_t cp = 0;
        int len = 1;
        if      (c < 0x80) { cp = c;            len = 1; }
        else if (c < 0xE0) { cp = c & 0x1F;     len = 2; }
        else if (c < 0xF0) { cp = c & 0x0F;     len = 3; }
        else                { cp = c & 0x07;     len = 4; }
        for (int k = 1; k < len && i + k < s.size(); ++k)
            cp = (cp << 6) | (static_cast<unsigned char>(s[i+k]) & 0x3F);
        // CJK Unified, Korean, etc. are double-width
        if ((cp >= 0x1100 && cp <= 0x115F) ||   // Hangul Jamo
            (cp >= 0x2E80 && cp <= 0x9FFF) ||   // CJK block
            (cp >= 0xAC00 && cp <= 0xD7AF) ||   // Hangul syllables
            (cp >= 0xF900 && cp <= 0xFAFF) ||   // CJK compat
            (cp >= 0xFF00 && cp <= 0xFF60))      // Fullwidth forms
            width += 2;
        else
            width += 1;
        i += len;
    }
    return width;
}

// Pad string to target display width (right-pad with spaces)
inline std::string padRight(const std::string& s, int targetDisplayWidth) {
    int dw = utf8DisplayWidth(s);
    int pad = std::max(0, targetDisplayWidth - dw);
    return s + std::string(pad, ' ');
}

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

// progress: 0.0 ~ 1.0
inline std::string makeBar(double progress, int total = 10) {
    int filled = static_cast<int>(progress * total);
    if (filled > total) filled = total;
    std::string bar;
    for (int i = 0; i < total; ++i)
        bar += (i < filled) ? "█" : "░";
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
