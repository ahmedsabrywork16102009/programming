#pragma once

#include "enumeration.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace process {

inline std::vector<char> getFirstLetters(std::string_view text, int pos = 1) {
    std::vector<char> vChars;
    if (pos < 1) return vChars;
    bool isFirst = true;
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] != ' ' && isFirst) {
            if (i + (static_cast<size_t>(pos) - 1) < text.length()) {
                vChars.push_back(text[i + (static_cast<size_t>(pos) - 1)]);
            }
        }
        isFirst = (text[i] == ' ');
    }
    return vChars;
}

inline std::string getLowerFirstLetters(std::string_view text) {
    std::string result(text);
    bool isFirst = true;
    for (size_t i = 0; i < result.length(); ++i) {
        if (result[i] != ' ' && isFirst) {
            result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
        }
        isFirst = (result[i] == ' ');
    }
    return result;
}

inline char invertChar(char letter) noexcept {
    return std::islower(static_cast<unsigned char>(letter))
               ? static_cast<char>(std::toupper(static_cast<unsigned char>(letter)))
               : static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));
}

inline std::string changeCharCase(std::string_view text, enumeration::enCharCase charCase) {
    std::string result(text);
    for (char &c : result) {
        if (charCase == enumeration::Upper) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        } else if (charCase == enumeration::Lower) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (charCase == enumeration::Invert) {
            c = invertChar(c);
        }
    }
    return result;
}

inline int countCapitalLetters(std::string_view text) noexcept {
    int count = 0;
    for (char c : text) {
        if (std::isupper(static_cast<unsigned char>(c))) {
            ++count;
        }
    }
    return count;
}

inline int countSmallLetters(std::string_view text) noexcept {
    int count = 0;
    for (char c : text) {
        if (std::islower(static_cast<unsigned char>(c))) {
            ++count;
        }
    }
    return count;
}

inline int countSpaces(std::string_view text) noexcept {
    int count = 0;
    for (char c : text) {
        if (c == ' ') {
            ++count;
        }
    }
    return count;
}

inline int countLetter(std::string_view text, char targetChar, bool matchCase = true) noexcept {
    int counter = 0;
    const unsigned char targetUpper = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(targetChar)));
    for (char c : text) {
        if (matchCase) {
            if (c == targetChar) ++counter;
        } else {
            if (static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(c))) == targetUpper) {
                ++counter;
            }
        }
    }
    return counter;
}

inline bool isVowel(char letter, bool matchCase = true) noexcept {
    char c = matchCase ? letter : static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            (matchCase && (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U')));
}

inline int countVowels(std::string_view text, bool matchCase = true) noexcept {
    int counter = 0;
    for (char c : text) {
        if (isVowel(c, matchCase)) {
            ++counter;
        }
    }
    return counter;
}

inline std::vector<char> getVowels(std::string_view text, bool matchCase = true) {
    std::vector<char> vVowels;
    for (char c : text) {
        if (isVowel(c, matchCase)) {
            vVowels.push_back(c);
        }
    }
    return vVowels;
}

inline std::vector<std::string> split(std::string_view text, char delimiter = ' ') {
    std::vector<std::string> tokens;
    if (text.empty()) return tokens;

    size_t start = 0;
    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == delimiter) {
            if (i > start) {
                tokens.emplace_back(text.substr(start, i - start));
            }
            start = i + 1;
        }
    }
    if (start < text.length()) {
        tokens.emplace_back(text.substr(start));
    }
    return tokens;
}

inline int countWords(std::string_view text, char delimiter = ' ') {
    if (text.empty()) return 0;
    int counter = 0;
    bool inWord = false;
    for (char c : text) {
        if (c != delimiter) {
            if (!inWord) {
                ++counter;
                inWord = true;
            }
        } else {
            inWord = false;
        }
    }
    return counter;
}

inline std::string trimLeft(std::string_view text, char delimiter = ' ') {
    size_t start = text.find_first_not_of(delimiter);
    return (start == std::string_view::npos) ? "" : std::string(text.substr(start));
}

inline std::string trimRight(std::string_view text, char delimiter = ' ') {
    size_t end = text.find_last_not_of(delimiter);
    return (end == std::string_view::npos) ? "" : std::string(text.substr(0, end + 1));
}

inline std::string trimAll(std::string_view text, char delimiter = ' ') {
    size_t start = text.find_first_not_of(delimiter);
    if (start == std::string_view::npos) return "";
    size_t end = text.find_last_not_of(delimiter);
    return std::string(text.substr(start, end - start + 1));
}

inline std::string trimString(std::string_view text, enumeration::enTrim trimType) {
    switch (trimType) {
    case enumeration::TrimLeft:
        return trimLeft(text);
    case enumeration::TrimRight:
        return trimRight(text);
    case enumeration::TrimAll:
        return trimAll(text);
    default:
        return std::string(text);
    }
}

inline std::string joinStrings(const std::vector<std::string> &vStrings,
                               std::string_view delimiter = " ",
                               bool isReverse = false) {
    if (vStrings.empty()) return "";
    std::string text;
    size_t totalLen = 0;
    for (const auto &s : vStrings) totalLen += s.length() + delimiter.length();
    text.reserve(totalLen);

    if (isReverse) {
        for (auto it = vStrings.rbegin(); it != vStrings.rend(); ++it) {
            text.append(*it);
            text.append(delimiter);
        }
    } else {
        for (const auto &s : vStrings) {
            text.append(s);
            text.append(delimiter);
        }
    }
    if (text.length() >= delimiter.length()) {
        text.resize(text.length() - delimiter.length());
    }
    return text;
}

inline std::string stripPunctuation(std::string_view text) {
    std::string result;
    result.reserve(text.length());
    for (char c : text) {
        if (!std::ispunct(static_cast<unsigned char>(c))) {
            result.push_back(c);
        }
    }
    return result;
}

inline std::vector<std::string> splitByString(std::string_view text,
                                              std::string_view delimiter = "#//#") {
    std::string trimmed = trimAll(text);
    if (trimmed.empty()) return {};

    std::vector<std::string> tokens;
    if (delimiter.empty()) {
        tokens.emplace_back(trimmed);
        return tokens;
    }

    size_t start = 0;
    while (true) {
        size_t found = trimmed.find(delimiter, start);
        if (found == std::string::npos) {
            std::string rest = trimmed.substr(start);
            if (!rest.empty()) {
                tokens.push_back(std::move(rest));
            }
            break;
        }
        tokens.push_back(trimmed.substr(start, found - start));
        start = found + delimiter.length();
    }
    return tokens;
}

inline std::string toLower(std::string_view text) {
    std::string s(text);
    for (char &c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

inline std::string toUpper(std::string_view text) {
    std::string s(text);
    for (char &c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

inline std::string replaceAll(std::string_view text,
                              std::string_view stringToReplace,
                              std::string_view sReplaceTo,
                              bool matchCase = true) {
    if (stringToReplace.empty() || text.empty()) {
        return std::string(text);
    }

    std::string result;
    result.reserve(text.length());
    size_t pos = 0;

    if (!matchCase) {
        std::string s1Lower = toLower(text);
        std::string targetLower = toLower(stringToReplace);

        while (true) {
            size_t found = s1Lower.find(targetLower, pos);
            if (found == std::string::npos) {
                result.append(text.substr(pos));
                break;
            }
            result.append(text.substr(pos, found - pos));
            result.append(sReplaceTo);
            pos = found + stringToReplace.length();
        }
    } else {
        while (true) {
            size_t found = text.find(stringToReplace, pos);
            if (found == std::string_view::npos) {
                result.append(text.substr(pos));
                break;
            }
            result.append(text.substr(pos, found - pos));
            result.append(sReplaceTo);
            pos = found + stringToReplace.length();
        }
    }
    return result;
}

inline std::string numberToText(long long number, enumeration::enLanguage language = enumeration::EN) {
    if (number == 0) return "";

    if (number < 0) {
        return (language == enumeration::EN ? "minus " : "سالب ") + numberToText(-number, language);
    }

    switch (language) {
    case enumeration::EN: {
        if (number >= 1 && number <= 19) {
            static const std::string arr[] = {
                "",        "one",       "two",      "three",
                "four",    "five",      "six",      "seven",
                "eight",   "nine",      "ten",      "eleven",
                "twelve",  "thirteen",  "fourteen", "fifteen",
                "sixteen", "seventeen", "eighteen", "nineteen"
            };
            return arr[number];
        }
        if (number >= 20 && number <= 99) {
            static const std::string arr[] = {
                "", "", "twenty", "thirty", "forty",
                "fifty", "sixty", "seventy", "eighty", "ninety"
            };
            std::string suffix = numberToText(number % 10, enumeration::EN);
            return arr[number / 10] + (suffix.empty() ? "" : " " + suffix);
        }
        struct Scale {
            long long limit;
            const char* name;
        };
        static const Scale scales[] = {
            {1000000000LL, " Billion"},
            {1000000LL, " Million"},
            {1000LL, " Thousand"},
            {100LL, " Hundred"}
        };
        for (const auto &scale : scales) {
            if (number >= scale.limit) {
                std::string suffix = numberToText(number % scale.limit, enumeration::EN);
                return numberToText(number / scale.limit, enumeration::EN) + scale.name +
                       (suffix.empty() ? "" : " " + suffix);
            }
        }
        break;
    }
    case enumeration::AR: {
        if (number >= 1 && number <= 19) {
            static const std::string arr[] = {
                "",         "واحد",      "اثنان",      "ثلاثة",
                "أربعة",    "خمسة",      "ستة",        "سبعة",
                "ثمانية",   "تسعة",      "عشرة",       "أحد عشر",
                "اثنا عشر", "ثلاثة عشر", "أربعة عشر",  "خمسة عشر",
                "ستة عشر",  "سبعة عشر",  "ثمانية عشر", "تسعة عشر"
            };
            return arr[number];
        }
        if (number >= 20 && number <= 99) {
            static const std::string arr[] = {
                "", "", "عشرون", "ثلاثون", "أربعون",
                "خمسون", "ستون", "سبعون", "ثمانون", "تسعون"
            };
            std::string suffix = numberToText(number % 10, enumeration::AR);
            return arr[number / 10] + (suffix.empty() ? "" : " " + suffix);
        }
        struct Scale {
            long long limit;
            const char* name;
        };
        static const Scale scales[] = {
            {1000000000LL, " مليار"},
            {1000000LL, " مليون"},
            {1000LL, " الف"},
            {100LL, " مائة"}
        };
        for (const auto &scale : scales) {
            if (number >= scale.limit) {
                std::string suffix = numberToText(number % scale.limit, enumeration::AR);
                return numberToText(number / scale.limit, enumeration::AR) + scale.name +
                       (suffix.empty() ? "" : " " + suffix);
            }
        }
        break;
    }
    }
    return "";
}

} // namespace process

