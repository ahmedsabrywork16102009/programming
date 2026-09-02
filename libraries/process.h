#pragma once

#include "enumeration.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace process {

using namespace enumeration;

// --- Numbers (Declarations) ---
template <typename T> enNumberType checkNumberType(T number);
int randomNumber(int from, int to);
int getRandomNumber(int from, int to);
template <typename T> constexpr bool validateNumber(T number, T from, T to);

// --- Strings (Declarations) ---
std::vector<char> getFirstLetterFromEachWord(std::string_view text,
                                             int pos = 1);
std::string getFirstLetterFromEachWordUpper(std::string text);
char invertCharType(char letter);
std::string changeCharType(std::string text, enCharsType charType);
int countCapitalLetters(std::string_view text);
int countSmallLetters(std::string_view text);
int countSpaces(std::string_view text);
int countLetterInString(std::string_view text, const char targetChar,
                        bool matchCase = true);
bool isVowel(char letter, bool matchCase = true);
int countVowelsInString(std::string_view text, bool matchCase = true);
std::vector<char> getVowelsInString(std::string_view text,
                                    bool matchCase = true);
std::vector<std::string> getTokensFromString(std::string_view text,
                                             char delimiter = ' ');
int countWords(std::string_view text, char delimiter = ' ');
std::string trimLeft(std::string text, char delimiter = ' ');
std::string trimRight(std::string text, char delimiter = ' ');
std::string trimAll(std::string text, char delimiter = ' ');
std::string trimString(std::string text, enTrimType trimType);
std::string joinStrings(const std::vector<std::string> &vStrings,
                        std::string_view delimiter = " ",
                        bool isReverse = false);
std::string removePunctuationFromTokens(std::string text);
std::vector<std::string> splitString(std::string text,
                                     std::string_view delimiter = "#//#");
std::string numberToText(int number, enLanguage language = EN);

// --- Date & Time (Declarations) ---
constexpr bool isLeapYear(size_t year) noexcept;
constexpr size_t daysInYear(size_t year) noexcept;
constexpr size_t hoursInYear(size_t year) noexcept;
constexpr size_t minutesInYear(size_t year) noexcept;
constexpr size_t secondsInYear(size_t year) noexcept;
constexpr size_t daysInMonth(size_t month, size_t year) noexcept;
constexpr size_t hoursInMonth(size_t month, size_t year) noexcept;
constexpr size_t minutesInMonth(size_t month, size_t year) noexcept;
constexpr size_t secondsInMonth(size_t month, size_t year) noexcept;
std::string joinDate(const size_t day, const size_t month,
                    const size_t year) noexcept;
constexpr enWeekdays getWeekdayNumber(const size_t day, const size_t month,
                                      const size_t year) noexcept;
std::string getMonthName(enMonths month, bool fullName = true) noexcept;

// =================================================================================
//  Numbers
// =================================================================================

template <typename T> enNumberType checkNumberType(T number) {
  static_assert(std::is_arithmetic<T>::value, "This is not number.");
  return (static_cast<int>(number) & 1) ? Odd : Even;
}

inline int randomNumber(int from, int to) {
  return rand() % (to - from + 1) + from;
}

inline int getRandomNumber(int from, int to) {
  if (from > to)
    std::swap(from, to);
  return randomNumber(from, to);
}

template <typename T> constexpr bool validateNumber(T number, T from, T to) {
  return ((number >= from) && (number <= to));
}

// =================================================================================
//  Strings
// =================================================================================

inline std::vector<char> getFirstLetterFromEachWord(std::string_view text,
                                                    int pos) {
  std::vector<char> vChars;
  bool isFirst = true;
  for (size_t i = 0; i < text.length(); i++) {
    if (text[i] != ' ' && isFirst) {
      if (i + (pos - 1) < text.length()) {
        vChars.push_back(text[i + (pos - 1)]);
      }
    }
    isFirst = (text[i] == ' ');
  }
  return vChars;
}

inline std::string getFirstLetterFromEachWordUpper(std::string text) {
  bool isFirst = true;
  for (size_t i = 0; i < text.length(); i++) {
    if (text[i] != ' ' && isFirst) {
      text[i] = std::tolower(text[i]);
    }
    isFirst = (text[i] == ' ');
  }
  return text;
}

inline char invertCharType(char letter) {
  return (std::islower(letter)) ? std::toupper(letter) : std::tolower(letter);
}

inline std::string changeCharType(std::string text, enCharsType charType) {
  for (size_t i = 0; i < text.length(); i++) {
    if (charType == enCharsType::Upper) {
      text[i] = std::toupper(text[i]);
    } else if (charType == enCharsType::Lower) {
      text[i] = std::tolower(text[i]);
    } else if (charType == enCharsType::Invert) {
      text[i] = invertCharType(text[i]);
    }
  }
  return text;
}

inline int countCapitalLetters(std::string_view text) {
  int countOfCapital = 0;
  for (char c : text) {
    if (std::isupper(c)) {
      countOfCapital++;
    }
  }
  return countOfCapital;
}

inline int countSmallLetters(std::string_view text) {
  int countOfSmall = 0;
  for (char c : text) {
    if (std::islower(c)) {
      countOfSmall++;
    }
  }
  return countOfSmall;
}

inline int countSpaces(std::string_view text) {
  int countOfSpaces = 0;
  for (char c : text) {
    if (c == ' ') {
      countOfSpaces++;
    }
  }
  return countOfSpaces;
}

inline int countLetterInString(std::string_view text, const char targetChar,
                               bool matchCase) {
  int counter = 0;
  for (char c : text) {
    if (matchCase) {
      if (c == targetChar) {
        counter++;
      }
    } else {
      if (std::tolower(c) == std::tolower(targetChar)) {
        counter++;
      }
    }
  }
  return counter;
}

inline bool isVowel(char letter, bool matchCase) {
  if (!matchCase) {
    return (std::tolower(letter) == 'a' || std::tolower(letter) == 'e' ||
            std::tolower(letter) == 'i' || std::tolower(letter) == 'o' ||
            std::tolower(letter) == 'u');
  } else {
    return (letter == 'a' || letter == 'e' || letter == 'i' || letter == 'o' ||
            letter == 'u');
  }
}

inline int countVowelsInString(std::string_view text, bool matchCase) {
  int counter = 0;
  for (char c : text) {
    if (isVowel(c, matchCase)) {
      counter++;
    }
  }
  return counter;
}

inline std::vector<char> getVowelsInString(std::string_view text,
                                           bool matchCase) {
  std::vector<char> vVowels;
  for (char c : text) {
    if (isVowel(c, matchCase)) {
      vVowels.push_back(c);
    }
  }
  return vVowels;
}

inline std::vector<std::string> getTokensFromString(std::string_view text,
                                                    char delimiter) {
  if (text.empty()) {
    return {};
  }
  std::string word = "";
  std::vector<std::string> vTokens;
  for (auto c : text) {
    if (c != delimiter) {
      word.push_back(c);
    } else if (!word.empty()) {
      vTokens.push_back(word);
      word.clear();
    }
  }
  if (!word.empty()) {
    vTokens.push_back(word);
  }
  return vTokens;
}

inline int countWords(std::string_view text, char delimiter) {
  if (text.empty()) {
    return 0;
  }
  std::string word = "";
  int counter = 0;
  for (auto c : text) {
    if (c != delimiter) {
      word.push_back(c);
    } else if (!word.empty()) {
      counter++;
      word.clear();
    }
  }
  if (!word.empty()) {
    counter++;
  }
  return counter;
}

inline std::string trimLeft(std::string text, char delimiter) {
  size_t pos = 0;
  while (pos < text.length() && text[pos] == delimiter) {
    pos++;
  }
  return text.substr(pos);
}

inline std::string trimRight(std::string text, char delimiter) {
  if (text.empty())
    return text;
  size_t pos = text.length() - 1;
  while (pos > 0 && text[pos] == delimiter) {
    pos--;
  }
  if (pos == 0 && text[0] == delimiter)
    return "";
  return text.substr(0, pos + 1);
}

inline std::string trimAll(std::string text, char delimiter) {
  return trimLeft(trimRight(text, delimiter), delimiter);
}

inline std::string trimString(std::string text, enTrimType trimType) {
  switch (trimType) {
  case enTrimType::TrimLeft:
    return trimLeft(text);
  case enTrimType::TrimRight:
    return trimRight(text);
  case enTrimType::TrimAll:
    return trimAll(text);
  default:
    return text;
  }
}

inline std::string joinStrings(const std::vector<std::string> &vStrings,
                               std::string_view delimiter, bool isReverse) {
  if (vStrings.empty()) {
    return "";
  }
  std::string text = "";
  if (isReverse) {
    for (auto iter = vStrings.rbegin(); iter != vStrings.rend(); ++iter) {
      text.append(*iter);
      text.append(delimiter);
    }
  } else {
    for (auto iter = vStrings.begin(); iter != vStrings.end(); ++iter) {
      text.append(*iter);
      text.append(delimiter);
    }
  }
  if (text.length() >= delimiter.length()) {
    text.resize(text.length() - delimiter.length());
  }
  return text;
}

inline std::string removePunctuationFromTokens(std::string text) {
  std::string newString = "";
  for (auto c : text) {
    if (std::ispunct(c)) {
      continue;
    }
    newString.push_back(c);
  }
  return newString;
}

inline std::vector<std::string> splitString(std::string text,
                                            std::string_view delimiter) {
  if (text.empty()) {
    return {};
  }
  std::vector<std::string> vTokens;
  text = trimAll(text);
  size_t pos = 0;
  std::string subWord = "";
  while ((pos = text.find(delimiter)) != std::string::npos) {
    subWord = text.substr(0, pos);
    vTokens.push_back(subWord);
    text.erase(0, pos + delimiter.length());
  }
  if (!text.empty()) {
    vTokens.push_back(text);
  }
  return vTokens;
}

inline std::string numberToText(int number, enLanguage language) {
  if (number == 0)
    return "";

  switch (language) {
  case enLanguage::EN: {
    if (number >= 1 && number <= 19) {
      std::string arr[] = {"",        "one",       "two",      "three",
                           "four",    "five",      "six",      "seven",
                           "eight",   "nine",      "ten",      "eleven",
                           "twelve",  "thirteen",  "fourteen", "fifteen",
                           "sixteen", "seventeen", "eighteen", "nineteen"};
      return arr[number];
    }
    if (number >= 20 && number <= 99) {
      std::string arr[] = {"",      "",      "twenty",  "thirty", "forty",
                           "fifty", "sixty", "seventy", "eighty", "ninety"};
      std::string suffix = numberToText(number % 10, EN);
      return arr[number / 10] + ((suffix.empty()) ? "" : " " + suffix);
    }
    struct stScale {
      int limit;
      std::string name;
    };
    stScale scales[] = {{1000000000, " Billion"},
                        {1000000, " Million"},
                        {1000, " Thousand"},
                        {100, " Hundred"}};
    for (const auto &scale : scales) {
      if (number >= scale.limit) {
        std::string suffix = numberToText(number % scale.limit, EN);
        return numberToText(number / scale.limit, EN) + scale.name +
               ((suffix.empty()) ? "" : " " + suffix);
      }
    }
    break;
  }
  case enLanguage::AR: {
    if (number >= 1 && number <= 19) {
      std::string arr[] = {"",         "واحد",      "اثنان",      "ثلاثة",
                           "أربعة",    "خمسة",      "ستة",        "سبعة",
                           "ثمانية",   "تسعة",      "عشرة",       "أحد عشر",
                           "اثنا عشر", "ثلاثة عشر", "أربعة عشر",  "خمسة عشر",
                           "ستة عشر",  "سبعة عشر",  "ثمانية عشر", "تسعة عشر"};
      return arr[number];
    }
    if (number >= 20 && number <= 99) {
      std::string arr[] = {"",      "",     "عشرون", "ثلاثون", "أربعون",
                           "خمسون", "ستون", "سبعون", "ثمانون", "تسعون"};
      std::string suffix = numberToText(number % 10, AR);
      return arr[number / 10] + ((suffix.empty()) ? "" : " " + suffix);
    }
    struct stScale {
      int limit;
      std::string name;
    };
    stScale scales[] = {{1000000000, " مليار"},
                        {1000000, " مليون"},
                        {1000, " الف"},
                        {100, " مائة"}};
    for (const auto &scale : scales) {
      if (number >= scale.limit) {
        std::string suffix = numberToText(number % scale.limit, AR);
        return numberToText(number / scale.limit, AR) + scale.name +
               ((suffix.empty()) ? "" : " " + suffix);
      }
    }
    break;
  }
  }
  return "";
}

// =================================================================================
//  Date & Time
// =================================================================================

constexpr bool isLeapYear(size_t year) noexcept {
  return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}

constexpr size_t daysInYear(size_t year) noexcept {
  return isLeapYear(year) ? 366 : 365;
}

constexpr size_t hoursInYear(size_t year) noexcept {
  return daysInYear(year) * 24;
}

constexpr size_t minutesInYear(size_t year) noexcept {
  return hoursInYear(year) * 60;
}

constexpr size_t secondsInYear(size_t year) noexcept {
  return minutesInYear(year) * 60;
}

constexpr size_t daysInMonth(size_t month, size_t year) noexcept {
  if (!validateNumber<size_t>(month, 1, 12))
    return 0;
  if (month == 2) {
    return isLeapYear(year) ? 29 : 28;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11) {
    return 30;
  }
  return 31;
}

constexpr size_t hoursInMonth(size_t month, size_t year) noexcept {
  return daysInMonth(month, year) * 24;
}

constexpr size_t minutesInMonth(size_t month, size_t year) noexcept {
  return hoursInMonth(month, year) * 60;
}

constexpr size_t secondsInMonth(size_t month, size_t year) noexcept {
  return minutesInMonth(month, year) * 60;
}

inline std::string joinDate(const size_t day, const size_t month,
                           const size_t year) noexcept {
  return std::to_string(day) + "/" + std::to_string(month) + "/" +
         std::to_string(year);
}

constexpr enWeekdays getWeekdayNumber(const size_t day, const size_t month,
                                      const size_t year) noexcept {
  int monthOffset = (14 - static_cast<int>(month)) / 12;
  int adjustedYear = static_cast<int>(year) - monthOffset;
  int adjustedMonth = static_cast<int>(month) + 12 * monthOffset - 2;
  return static_cast<enWeekdays>(
      ((static_cast<int>(day) + adjustedYear + adjustedYear / 4 -
        adjustedYear / 100 + adjustedYear / 400 + (31 * adjustedMonth) / 12) %
       7) +
      1); // adjusting to 1-7 mapping (Sunday=1)
}

inline std::string getMonthName(const enMonths month, bool fullName) noexcept {
  if (fullName) {
    switch (month) {
    case January:
      return "January";
    case February:
      return "February";
    case March:
      return "March";
    case April:
      return "April";
    case May:
      return "May";
    case June:
      return "June";
    case July:
      return "July";
    case August:
      return "August";
    case September:
      return "September";
    case October:
      return "October";
    case November:
      return "November";
    case December:
      return "December";
    }
  } else {
    switch (month) {
    case January:
      return "Jan";
    case February:
      return "Feb";
    case March:
      return "Mar";
    case April:
      return "Apr";
    case May:
      return "May";
    case June:
      return "Jun";
    case July:
      return "Jul";
    case August:
      return "Aug";
    case September:
      return "Sep";
    case October:
      return "Oct";
    case November:
      return "Nov";
    case December:
      return "Dec";
    }
  }

  return "Invalid month";
}

} // namespace process

namespace AS {
using namespace process;
}
