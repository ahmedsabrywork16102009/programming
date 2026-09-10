#pragma once

#include "enumeration.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace process {

using namespace enumeration;

// --- Numbers (Declarations) ---
template <typename T> enNumber checkNumberType(T number);
int randInt(int from, int to);
template <typename T> constexpr bool validateNumber(T number, T from, T to);

// --- Strings (Declarations) ---
std::vector<char> getFirstLetters(std::string_view text, int pos = 1);
std::string getLowerFirstLetters(std::string text);
char invertChar(char letter);
std::string changeCharCase(std::string text, enCharCase charCase);
int countCapitalLetters(std::string_view text);
int countSmallLetters(std::string_view text);
int countSpaces(std::string_view text);
int countLetter(std::string_view text, const char targetChar,
                bool matchCase = true);
bool isVowel(char letter, bool matchCase = true);
int countVowels(std::string_view text, bool matchCase = true);
std::vector<char> getVowels(std::string_view text, bool matchCase = true);
std::vector<std::string> split(std::string_view text, char delimiter = ' ');
int countWords(std::string_view text, char delimiter = ' ');
std::string trimLeft(std::string text, char delimiter = ' ');
std::string trimRight(std::string text, char delimiter = ' ');
std::string trimAll(std::string text, char delimiter = ' ');
std::string trimString(std::string text, enTrim trimType);
std::string joinStrings(const std::vector<std::string> &vStrings,
                        std::string_view delimiter = " ",
                        bool isReverse = false);
std::string stripPunctuation(std::string text);
std::vector<std::string> splitByString(std::string text,
                                       std::string_view delimiter = "#//#");
std::string numberToText(int number, enLanguage language = EN);
std::string toLower(std::string_view text);
std::string replaceAll(std::string text, std::string_view stringToReplace,
                       std::string_view sReplaceTo, bool matchCase = true);

// --- Date & Time (Declarations) ---
struct stDate {
  std::size_t year;
  std::size_t month;
  std::size_t day;
};
struct stPeriod {
  stDate dateFrom;
  stDate dateTo;
};

constexpr bool isLeapYear(size_t year) noexcept;
constexpr size_t daysInYear(size_t year) noexcept;
constexpr size_t hoursInYear(size_t year) noexcept;
constexpr size_t minutesInYear(size_t year) noexcept;
constexpr size_t secondsInYear(size_t year) noexcept;
constexpr size_t daysInMonth(size_t month, size_t year) noexcept;
constexpr size_t hoursInMonth(size_t month, size_t year) noexcept;
constexpr size_t minutesInMonth(size_t month, size_t year) noexcept;
constexpr size_t secondsInMonth(size_t month, size_t year) noexcept;
std::string joinDate(size_t day, size_t month, size_t year) noexcept;
enWeekday getWeekday(size_t day, size_t month, size_t year) noexcept;
enWeekday getWeekday(stDate date) noexcept;
bool isLastDayInWeek(enWeekday weekday) noexcept;
bool isLastDayInWeek(const stDate &date) noexcept;
bool isWeekend(enWeekday weekday) noexcept;
bool isWeekend(size_t day, size_t month, size_t year) noexcept;
bool isWeekend(const stDate &date) noexcept;
bool isBusinessDay(enWeekday weekday) noexcept;
bool isBusinessDay(size_t day, size_t month, size_t year) noexcept;
bool isBusinessDay(const stDate &date) noexcept;
size_t daysLeftInWeek(stDate date);
size_t daysLeftInMonth(stDate date);
size_t daysLeftInYear(stDate date);
std::string getMonthName(enMonth month, bool fullName = true) noexcept;
std::string getWeekdayName(enWeekday weekday, bool shortName = false) noexcept;
size_t dayOfYear(size_t day, size_t month, size_t year);
stDate getDateFromDayOfYear(size_t year, size_t numberOfDays,
                            size_t daysAdd = 0);
bool isAfter(stDate date1, stDate date2);
bool isBefore(stDate date1, stDate date2);
bool isEqual(stDate date1, stDate date2);
bool isLastDayInMonth(size_t day, size_t month, size_t year);
bool isLastMonthInYear(size_t day, size_t month, size_t year);
stDate addUnits(stDate date, size_t days = 1, size_t months = 0,
                size_t years = 0, size_t decades = 0, size_t centuries = 0,
                size_t millenniums = 0);
stDate subUnits(stDate date, size_t days = 1, size_t months = 0,
                size_t years = 0, size_t decades = 0, size_t centuries = 0,
                size_t millenniums = 0);
size_t daysFromYearOne(stDate date, bool includeCurrentDay = false);
long long daysBetween(stDate date1, stDate date2,
                      bool includeCurrentDay = false,
                      bool absoluteDiff = false);
stDate getSystemDate();
size_t ageInDays(stDate birthDate);
size_t calculateVacationDays(stDate dateFrom, stDate dateTo,
                             bool includeWeekends = false);
stDate CalculateVacationReturnDate(stDate date, long long vacationDays,
                                          bool includeWeekends = false);
enDateCompare compareDate(stDate d1, stDate d2);
bool isOverlap(stDate date1From, stDate date1To, stDate date2From,
               stDate date2To);
bool isOverlap(stPeriod period1, stPeriod period2);
long long calculatePeriod(stDate date1, stDate date2,
                        bool includeLastDay = false);
long long calculatePeriod(stPeriod period, bool includeLastDay = false);
bool isDateInPeriod(stDate date, stDate dateStart, stDate dateEnd);
bool isDateInPeriod(stDate date, stPeriod period);
std::size_t countOverlapDays(stPeriod period1, stPeriod period2);
bool isValidDate(stDate date);
stDate convertToDate(std::string_view dateString);
std::string dateToString(stDate date, enDateFormat format = enDateFormat::basicFormat);
std::string dateToString(stDate date, std::string_view format);

// =================================================================================
//  Numbers
// =================================================================================

template <typename T> enNumber checkNumberType(T number) {
  static_assert(std::is_arithmetic<T>::value, "This is not number.");
  return (static_cast<int>(number) & 1) ? Odd : Even;
}

inline int randInt(int from, int to) {
  if (from > to)
    std::swap(from, to);
  return rand() % (to - from + 1) + from;
}

template <typename T> constexpr bool validateNumber(T number, T from, T to) {
  return ((number >= from) && (number <= to));
}

// =================================================================================
//  Strings
// =================================================================================

inline std::vector<char> getFirstLetters(std::string_view text, int pos) {
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

inline std::string getLowerFirstLetters(std::string text) {
  bool isFirst = true;
  for (size_t i = 0; i < text.length(); i++) {
    if (text[i] != ' ' && isFirst) {
      text[i] = std::tolower(text[i]);
    }
    isFirst = (text[i] == ' ');
  }
  return text;
}

inline char invertChar(char letter) {
  return (std::islower(letter)) ? std::toupper(letter) : std::tolower(letter);
}

inline std::string changeCharCase(std::string text, enCharCase charCase) {
  for (size_t i = 0; i < text.length(); i++) {
    if (charCase == enCharCase::Upper) {
      text[i] = std::toupper(text[i]);
    } else if (charCase == enCharCase::Lower) {
      text[i] = std::tolower(text[i]);
    } else if (charCase == enCharCase::Invert) {
      text[i] = invertChar(text[i]);
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

inline int countLetter(std::string_view text, const char targetChar,
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

inline int countVowels(std::string_view text, bool matchCase) {
  int counter = 0;
  for (char c : text) {
    if (isVowel(c, matchCase)) {
      counter++;
    }
  }
  return counter;
}

inline std::vector<char> getVowels(std::string_view text, bool matchCase) {
  std::vector<char> vVowels;
  for (char c : text) {
    if (isVowel(c, matchCase)) {
      vVowels.push_back(c);
    }
  }
  return vVowels;
}

inline std::vector<std::string> split(std::string_view text, char delimiter) {
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

inline std::string trimString(std::string text, enTrim trimType) {
  switch (trimType) {
  case enTrim::TrimLeft:
    return trimLeft(text);
  case enTrim::TrimRight:
    return trimRight(text);
  case enTrim::TrimAll:
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

inline std::string stripPunctuation(std::string text) {
  std::string newString = "";
  for (auto c : text) {
    if (std::ispunct(c)) {
      continue;
    }
    newString.push_back(c);
  }
  return newString;
}

inline std::vector<std::string> splitByString(std::string text,
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

inline std::string toLower(std::string_view text) {
  std::string s(text);
  for (char &c : s) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  return s;
}

inline std::string replaceAll(std::string text,
                               std::string_view stringToReplace,
                               std::string_view sReplaceTo,
                               bool matchCase) {
  if (stringToReplace.empty() || text.empty()) {
    return text;
  }

  size_t pos = 0;

  if (!matchCase) {
    std::string s1Lower = toLower(text);
    std::string targetLower = toLower(stringToReplace);
    std::string replaceLower = toLower(sReplaceTo);

    while ((pos = s1Lower.find(targetLower, pos)) != std::string::npos) {
      text.replace(pos, stringToReplace.length(), sReplaceTo);
      s1Lower.replace(pos, stringToReplace.length(), replaceLower);
      pos += sReplaceTo.length();
    }
  } else {
    while ((pos = text.find(stringToReplace, pos)) != std::string::npos) {
      text.replace(pos, stringToReplace.length(), sReplaceTo);
      pos += sReplaceTo.length();
    }
  }

  return text;
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

inline std::string joinDate(size_t day, size_t month, size_t year) noexcept {
  return std::to_string(day) + "/" + std::to_string(month) + "/" +
         std::to_string(year);
}

inline enWeekday getWeekday(size_t day, size_t month, size_t year) noexcept {
  int monthOfset = (14 - static_cast<int>(month)) / 12;
  int adjustedYear = static_cast<int>(year) - monthOfset;
  int adjustedMonth = static_cast<int>(month) + 12 * monthOfset - 2;
  return static_cast<enWeekday>(
      (((static_cast<int>(day) + adjustedYear + adjustedYear / 4 -
         adjustedYear / 100 + adjustedYear / 400 + (31 * adjustedMonth) / 12 +
         1) %
        7) +
       1));
}

inline enWeekday getWeekday(stDate date) noexcept {
  return getWeekday(date.day, date.month, date.year);
}

inline bool isLastDayInWeek(enWeekday weekday) noexcept {
  return weekday == Friday;
}

inline bool isLastDayInWeek(const stDate &date) noexcept {
  return isLastDayInWeek(getWeekday(date));
}

inline bool isWeekend(enWeekday weekday) noexcept {
  return (weekday == Friday || weekday == Saturday);
}

inline bool isWeekend(size_t day, size_t month, size_t year) noexcept {
  return isWeekend(getWeekday(day, month, year));
}

inline bool isWeekend(const stDate &date) noexcept {
  return isWeekend(getWeekday(date));
}

inline bool isBusinessDay(enWeekday weekday) noexcept {
  return !isWeekend(weekday);
}

inline bool isBusinessDay(const stDate &date) noexcept {
  return isBusinessDay(getWeekday(date));
}

inline bool isBusinessDay(size_t day, size_t month, size_t year) noexcept {
  return isBusinessDay(getWeekday(day, month, year));
}

inline size_t daysLeftInWeek(stDate date) {
  return static_cast<size_t>(enWeekday::Friday - getWeekday(date)) + 1;
}

inline size_t daysLeftInMonth(stDate date) {
  return daysInMonth(date.month, date.year) - date.day + 1;
}

inline size_t daysLeftInYear(stDate date) {
  for (size_t i = 1; i < date.month; i++) {
    date.day += daysInMonth(i, date.year);
  }
  return daysInYear(date.year) - date.day + 1;
}

inline std::string getMonthName(enMonth month, bool fullName) noexcept {
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

inline std::string getWeekdayName(enWeekday weekday, bool shortName) noexcept {
  if (shortName) {
    switch (weekday) {
    case Saturday:
      return "Sat";
    case Sunday:
      return "Sun";
    case Monday:
      return "Mon";
    case Tuesday:
      return "Tue";
    case Wednesday:
      return "Wed";
    case Thursday:
      return "Thu";
    case Friday:
      return "Fri";
    }
  }
  switch (weekday) {
  case Saturday:
    return "Saturday";
  case Sunday:
    return "Sunday";
  case Monday:
    return "Monday";
  case Tuesday:
    return "Tuesday";
  case Wednesday:
    return "Wednesday";
  case Thursday:
    return "Thursday";
  case Friday:
    return "Friday";
  }
  return "Invalid weekday";
}

inline size_t dayOfYear(size_t day, size_t month, size_t year) {
  if (month > 12 || month < 1 || day < 1 || day > daysInMonth(month, year)) {
    return 0;
  }

  size_t totalDays = 0;

  for (size_t i = 1; i < month; i++) {
    totalDays += daysInMonth(i, year);
  }

  totalDays += day;

  return totalDays;
}

inline stDate getDateFromDayOfYear(size_t year, size_t numberOfDays,
                                   size_t daysAdd) {
  stDate date;

  date.month = 1;
  date.day = numberOfDays + daysAdd;
  date.year = year;

  size_t tempDaysInYear;
  while (date.day > (tempDaysInYear = daysInYear(date.year))) {
    date.day -= tempDaysInYear;
    date.year++;
  }

  size_t tempDaysInMonth;
  while (date.day > (tempDaysInMonth = daysInMonth(date.month, date.year))) {
    date.day -= tempDaysInMonth;
    date.month++;
  }

  return date;
}

inline bool isAfter(stDate date1, stDate date2) {
  return (date1.year > date2.year)
             ? true
             : ((date1.year == date2.year)
                    ? (date1.month > date2.month
                           ? true
                           : (date1.month == date2.month ? date1.day > date2.day
                                                         : false))
                    : false);
}

inline bool isEqual(stDate date1, stDate date2) {
  return (date1.year == date2.year)
             ? ((date1.month == date2.month)
                    ? ((date1.day == date2.day) ? true : false)
                    : false)
             : false;
}

inline bool isBefore(stDate date1, stDate date2) {
  return !isAfter(date1, date2) && !isEqual(date1, date2);
}

inline bool isLastDayInMonth(size_t day, size_t month, size_t year) {
  return day == daysInMonth(month, year);
}

inline bool isLastMonthInYear(size_t day, size_t month, size_t year) {
  return month == 12;
}

inline stDate addUnits(stDate date, size_t days, size_t months, size_t years,
                       size_t decades, size_t centuries, size_t millenniums) {

  if (millenniums != 0) {
    date.year += (millenniums * 1000);
  }

  if (decades != 0) {
    date.year += (decades * 10);
  }

  if (centuries != 0) {
    date.year += (centuries * 100);
  }

  if (years != 0) {
    date.year += years;
  }

  if (months != 0) {
    date.month += months;

    while (date.month > 12) {
      date.month -= 12;
      date.year++;
    }
  }

  size_t maxDays = daysInMonth(date.month, date.year);
  if (date.day > maxDays) {
    date.day = maxDays;
  }

  if (days != 0) {
    date.day += days;

    size_t tempDaysInMonth;
    while (date.day > (tempDaysInMonth = daysInMonth(date.month, date.year))) {
      date.day -= tempDaysInMonth;
      date.month++;

      while (date.month > 12) {
        date.month -= 12;
        date.year++;
      }
    }
  }

  return date;
}

inline stDate subUnits(stDate date, size_t days, size_t months, size_t years,
                       size_t decades, size_t centuries, size_t millenniums) {

  if (millenniums != 0) {
    date.year -= (millenniums * 1000);
  }

  if (decades != 0) {
    date.year -= (decades * 10);
  }

  if (centuries != 0) {
    date.year -= (centuries * 100);
  }

  if (years != 0) {
    date.year -= years;
  }

  if (months != 0) {
    long long tempMonth = (long long)date.month - months;

    while (tempMonth < 1) {
      tempMonth += 12;
      date.year--;
    }

    date.month = (size_t)tempMonth;
  }

  size_t maxDays = daysInMonth(date.month, date.year);
  if (date.day > maxDays) {
    date.day = maxDays;
  }

  if (days != 0) {
    long long tempDay = (long long)date.day - days;

    while (tempDay < 1) {
      date.month--;

      while (date.month < 1) {
        date.month += 12;
        date.year--;
      }

      tempDay += daysInMonth(date.month, date.year);
    }

    date.day = (size_t)tempDay;
  }

  return date;
}

inline size_t daysFromYearOne(stDate date, bool includeLastDay) {
  size_t daysBetween = 0;

  for (size_t year = 1; year < date.year; year++) {
    daysBetween += daysInYear(year);
  }

  for (size_t month = 1; month < date.month; month++) {
    daysBetween += daysInMonth(month, date.year);
  }

  daysBetween += date.day;

  if (!includeLastDay && daysBetween > 0) {
    daysBetween--;
  }

  return daysBetween;
}

inline long long daysBetween(stDate date1, stDate date2, bool includeLastDay,
                             bool absoluteDiff) {
  long long d1 = daysFromYearOne(date1);
  long long d2 = daysFromYearOne(date2);

  long long diff;

  if (absoluteDiff) {
    diff = (d2 >= d1) ? (d2 - d1) : (d1 - d2);
  } else {
    diff = (d1 - d2);
  }

  return includeLastDay ? ((diff >= 0) ? diff + 1 : diff - 1) : diff;
}

inline long long daysBetween(stPeriod period, bool includeLastDay,
                             bool absoluteDiff) {
  return daysBetween(period.dateFrom, period.dateTo, includeLastDay,
                     absoluteDiff);
}

inline stDate getSystemDate() {
  stDate date;

  time_t t = time(0);
  tm *ltm = localtime(&t);

  date.day = ltm->tm_mday;
  date.month = ltm->tm_mon + 1;
  date.year = ltm->tm_year + 1900;

  return date;
}

inline size_t ageInDays(stDate birthDate) {
  return daysFromYearOne(getSystemDate(), true) - daysFromYearOne(birthDate);
}

inline size_t calculateVacationDays(stDate dateFrom, stDate dateTo,
                                    bool includeWeekends) {
  size_t vacationDays = 0;
  while (isBefore(dateFrom, dateTo)) {
    if (includeWeekends || isBusinessDay(dateFrom)) {
      vacationDays++;
    }
    dateFrom = addUnits(dateFrom, 1);
  }
  return vacationDays;
}

inline stDate CalculateVacationReturnDate(stDate date, long long vacationDays,
                                          bool includeWeekends) {
  stDate dateResult = date;

  while (vacationDays > 0) {
    if (includeWeekends || isBusinessDay(dateResult)) {
      vacationDays--;
    }

    dateResult = addUnits(dateResult, 1);
  }

  if (!includeWeekends) {
    while (!isBusinessDay(dateResult)) {
      dateResult = addUnits(dateResult, 1);
    }
  }

  return dateResult;
}

inline enDateCompare compareDate(stDate d1, stDate d2) {
  if (isAfter(d1, d2))
    return enDateCompare::Date1_After;
  if (isBefore(d1, d2))
    return enDateCompare::Date1_Before;
  return enDateCompare::Date1_Equal;
}

inline bool isOverlap(stDate date1From, stDate date1To, stDate date2From,
                      stDate date2To) {
  return !(isBefore(date1To, date2From) || isBefore(date2To, date1From));
}

inline bool isOverlap(stPeriod period1, stPeriod period2) {
  return isOverlap(period1.dateFrom, period1.dateTo, period2.dateFrom,
                   period2.dateTo);
}

inline long long calculatePeriod(stDate date1, stDate date2, bool includeLastDay) {
  return daysBetween(date1, date2, includeLastDay, true);
}

inline long long calculatePeriod(stPeriod period, bool includeLastDay) {
  return calculatePeriod(period.dateFrom, period.dateTo, includeLastDay);
}

inline bool isDateInPeriod(stDate date, stDate dateStart, stDate dateEnd) {
  return !(isBefore(date, dateStart) || isAfter(date, dateEnd));
}

inline bool isDateInPeriod(stDate date, stPeriod period) {
  return isDateInPeriod(date, period.dateFrom, period.dateTo);
}

inline std::size_t countOverlapDays(stPeriod period1, stPeriod period2) {
  std::size_t overlapDays = 0;

  std::size_t period1Length = calculatePeriod(period1, true);
  std::size_t period2Length = calculatePeriod(period2, true);

  if (!isOverlap(period1, period2)) {
    return 0;
  }

  if (period1Length > period2Length) {
    while (isBefore(period2.dateFrom, period2.dateTo)) {
      if (isDateInPeriod(period2.dateFrom, period1)) {
        overlapDays++;
      }

      period2.dateFrom = addUnits(period2.dateFrom, 1);
    }
  } else {
    while (isBefore(period1.dateFrom, period1.dateTo)) {
      if (isDateInPeriod(period1.dateFrom, period2)) {
        overlapDays++;
      }

      period1.dateFrom = addUnits(period1.dateFrom, 1);
    }
  }

  return overlapDays;
}

inline bool isValidDate(stDate date) {
  return (date.year >= 0) && (date.month >= 1 && date.month <= 12) &&
         (date.day >= 1 && date.day <= daysInMonth(date.month, date.year));
}

inline stDate convertToDate(std::string_view dateString) {
  stDate date{};
  std::vector<std::string> vDateString = split(dateString, '/');

  if (vDateString.size() != 3) {
    return date;
  }

  date.day = static_cast<std::size_t>(std::stoi(vDateString[0]));
  date.month = static_cast<std::size_t>(std::stoi(vDateString[1]));
  date.year = static_cast<std::size_t>(std::stoi(vDateString[2]));

  return date;
}

inline std::string dateToString(stDate date, enDateFormat format) {
  switch (format) {
  case enDateFormat::DD_MM_YYYYWithForwaredSlash:
    return std::to_string(date.day) + "/" + std::to_string(date.month) + "/" +
           std::to_string(date.year);
  case enDateFormat::YYYY_MM_DDWithForwaredSlash:
    return std::to_string(date.year) + "/" + std::to_string(date.month) + "/" +
           std::to_string(date.day);
  case enDateFormat::MM_DD_YYYYWithForwaredSlash:
    return std::to_string(date.month) + "/" + std::to_string(date.day) + "/" +
           std::to_string(date.year);
  case enDateFormat::DD_MM_YYYYWithHyphen:
    return std::to_string(date.day) + "-" + std::to_string(date.month) + "-" +
           std::to_string(date.year);
  case enDateFormat::YYYY_MM_DDWithHyphen:
    return std::to_string(date.year) + "-" + std::to_string(date.month) + "-" +
           std::to_string(date.day);
  case enDateFormat::MM_DD_YYYYWithHyphen:
    return std::to_string(date.month) + "-" + std::to_string(date.day) + "-" +
           std::to_string(date.year);
  case enDateFormat::basicFormat:
    return "Day: " + std::to_string(date.day) + ", Month: " + std::to_string(date.month) +
           ", Year: " + std::to_string(date.year);
  }

  return "";
}

inline std::string dateToString(stDate date, std::string_view format) {
  std::string result(format);

  result = replaceAll(result, "DD", std::to_string(date.day));
  result = replaceAll(result, "MM", std::to_string(date.month));
  result = replaceAll(result, "YYYY", std::to_string(date.year));

  return result;
}

} // namespace process

namespace AS {
using namespace process;
}
