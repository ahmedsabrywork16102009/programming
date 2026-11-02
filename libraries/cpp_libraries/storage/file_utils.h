#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace storage {

// ===========================================================================
//  Declarations
// ===========================================================================

bool appendLineToFile(std::string_view fileName, std::string_view lineData);
bool writeLinesToFile(std::string_view fileName, const std::vector<std::string> &vLines);
std::vector<std::string> loadLinesFromFile(std::string_view fileName);

// ===========================================================================
//  Implementations
// ===========================================================================

inline bool appendLineToFile(std::string_view fileName, std::string_view lineData) {
    std::string path(fileName);
    std::ofstream file(path, std::ios::out | std::ios::app);
    if (!file.is_open()) return false;
    file << lineData << '\n';
    return true;
}

inline bool writeLinesToFile(std::string_view fileName, const std::vector<std::string> &vLines) {
    std::string path(fileName);
    std::ofstream file(path, std::ios::out);
    if (!file.is_open()) return false;
    for (const auto &line : vLines) {
        file << line << '\n';
    }
    return true;
}

inline std::vector<std::string> loadLinesFromFile(std::string_view fileName) {
    std::vector<std::string> vLines;
    std::string path(fileName);
    std::ifstream file(path);
    if (!file.is_open()) return vLines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            vLines.push_back(line);
        }
    }
    return vLines;
}

} // namespace storage

namespace AS {
    using namespace storage;
} // namespace AS
