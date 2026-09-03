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
struct stDate {
  std::size_t year;
  std::size_t month;
  std::size_t day;
  std::size_t hour;
  std::size_t minute;
  std::size_t second;
  std::size_t milisecond;
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
std::string joinDate(const size_t day, const size_t month,
                     const size_t year) noexcept;
enWeekdays getWeekdayNumber(const size_t day, const size_t month,
                            const size_t year) noexcept;
std::string getMonthName(enMonths month, bool fullName = true) noexcept;
std::string getWeekdayName(enWeekdays weekday, bool shortName = false) noexcept;
size_t calculateDaysFromBeginingOfYear(size_t day, size_t month, size_t year);
stDate getDate(size_t year, size_t numberOfDays, size_t daysAdd = 0);
bool isDate1UpperDate2(stDate date1, stDate date2);
bool isDate1EqualDate2(stDate date1, stDate date2);
bool isLastDayInMonth(size_t day, size_t month, size_t year);
bool isLastMonthInYear(size_t day, size_t month, size_t year);
stDate increaseDate1Day(stDate date);
size_t calculateDaysFrom1_1_1(stDate date, bool includeCurrentDay = false);
long long calculateDate1BetweenDate2(stDate date1, stDate date2,
                                     bool includeCurrentDay = false,
                                     bool doYouWantDate1IsUpper = false);
stDate getSystemDate();
size_t calculateAgeWithDays(stDate date1);

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

constexpr enWeekdays getWeekdayNumber(stDate date) noexcept {
  int monthOfset = (14 - static_cast<int>(date.month)) / 12;
  int adjustedYear = static_cast<int>(date.year) - monthOfset;
  int adjustedMonth = static_cast<int>(date.month) + 12 * monthOfset - 2;
  return static_cast<enWeekdays>(
      ((static_cast<int>(date.day) + adjustedYear + adjustedYear / 4 -
        adjustedYear / 100 + adjustedYear / 400 + (31 * adjustedMonth) / 12) %
       7) +
      1); // adjusting to 1-7 mapping (Sunday=1)
}

inline bool isLastDayInWeek(enWeekdays weekday) noexcept {
  return weekday == Friday;
}

inline bool isLastDayInWeek(const stDate &date) noexcept {
  return isLastDayInWeek(getWeekdayNumber(date));
}

inline bool isItWeekend(enWeekdays weekday) noexcept {
  return weekday == Friday;
}

inline bool isItWeekend(const stDate &date) noexcept {
  return isItWeekend(getWeekdayNumber(date));
}

inline bool isItBusinessDay(enWeekdays weekday) noexcept {
  return !isItWeekend(weekday);
}

inline bool isItBusinessDay(const stDate &date) noexcept {
  return isItBusinessDay(getWeekdayNumber(date));
}

inline size_t calculateDaysUntilEndOfWeek(stDate date) {
  return static_cast<size_t>(enWeekdays::Friday - getWeekdayNumber(date)) + 1;
}

inline size_t calculateDaysUntilEndOfMonth(stDate date) {
  return daysInMonth(date.month, date.year) - date.day + 1;
}

inline size_t calculateDaysUntilEndOfYear(stDate date) {
  for (size_t i = 1; i < date.month; i++) {
    date.day += daysInMonth(i, date.year);
  }
  return daysInYear(date.year) - date.day + 1;
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

inline std::string getWeekdayName(enWeekdays weekday, bool shortName) noexcept {
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

inline size_t calculateDaysFromBeginingOfYear(size_t day, size_t month,
                                              size_t year) {
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

inline stDate getDate(size_t year, size_t numberOfDays, size_t daysAdd) {
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

inline bool isDate1UpperDate2(stDate date1, stDate date2) {
  return (date1.year > date2.year)
             ? true
             : ((date1.year == date2.year)
                    ? (date1.month > date2.month
                           ? true
                           : (date1.month == date2.month ? date1.day > date2.day
                                                         : false))
                    : false);
}

inline bool isDate1EqualDate2(stDate date1, stDate date2) {
  return (date1.year == date2.year)
             ? ((date1.month == date2.month)
                    ? ((date1.day == date2.day) ? true : false)
                    : false)
             : false;
}

inline bool isLastDayInMonth(size_t day, size_t month, size_t year) {
  return day == daysInMonth(month, year);
}

inline bool isLastMonthInYear(size_t day, size_t month, size_t year) {
  return month == 12;
}

inline stDate increaseDateByXUnits(stDate date, size_t numberOfDays = 1,
                                   size_t numberOfMonth = 0,
                                   size_t numberOfYears = 0,
                                   size_t numberOfDecades = 0,
                                   size_t numberOfCenturies = 0,
                                   size_t numberOfMillenniums = 0) {

  if (numberOfMillenniums != 0) {
    date.year += (numberOfMillenniums * 1000);
  }

  if (numberOfDecades != 0) {
    date.year += (numberOfDecades * 10);
  }

  if (numberOfCenturies != 0) {
    date.year += (numberOfCenturies * 100);
  }

  if (numberOfYears != 0) {
    date.year += numberOfYears;
  }

  if (numberOfMonth != 0) {
    date.month += numberOfMonth;

    while (date.month > 12) {
      date.month -= 12;
      date.year++;
    }
  }

  size_t maxDays = daysInMonth(date.month, date.year);
  if (date.day > maxDays) {
    date.day = maxDays;
  }

  if (numberOfDays != 0) {
    date.day += numberOfDays;

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

inline stDate decreaseDateByXUnits(stDate date, size_t numberOfDays = 1,
                                   size_t numberOfMonth = 0,
                                   size_t numberOfYears = 0,
                                   size_t numberOfDecades = 0,
                                   size_t numberOfCenturies = 0,
                                   size_t numberOfMillenniums = 0) {

  if (numberOfMillenniums != 0) {
    date.year -= (numberOfMillenniums * 1000);
  }

  if (numberOfDecades != 0) {
    date.year -= (numberOfDecades * 10);
  }

  if (numberOfCenturies != 0) {
    date.year -= (numberOfCenturies * 100);
  }

  if (numberOfYears != 0) {
    date.year -= numberOfYears;
  }

  if (numberOfMonth != 0) {
    long long tempMonth = (long long)date.month - numberOfMonth;

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

  if (numberOfDays != 0) {
    long long tempDay = (long long)date.day - numberOfDays;

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

inline size_t calculateDaysFrom1_1_1(stDate date, bool includeCurrentDay) {
  size_t daysBetween = 0;

  for (size_t year = 1; year < date.year; year++) {
    daysBetween += daysInYear(year);
  }

  for (size_t month = 1; month < date.month; month++) {
    daysBetween += daysInMonth(month, date.year);
  }

  daysBetween += date.day;

  if (!includeCurrentDay && daysBetween > 0) {
    daysBetween--;
  }

  return daysBetween;
}

inline long long calculateDate1BetweenDate2(stDate date1, stDate date2,
                                            bool includeCurrentDay,
                                            bool doYouWantDate1IsUpper) {
  long long d1 = calculateDaysFrom1_1_1(date1);
  long long d2 = calculateDaysFrom1_1_1(date2);

  long long diff;

  if (doYouWantDate1IsUpper) {
    diff = (d2 >= d1) ? (d2 - d1) : (d1 - d2);
  } else {
    diff = (d1 - d2);
  }

  return includeCurrentDay ? ((diff >= 0) ? diff + 1 : diff - 1) : diff;
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

inline size_t calculateAgeWithDays(stDate date1) {
  return calculateDaysFrom1_1_1(getSystemDate(), true) -
         calculateDaysFrom1_1_1(date1);
}




} // namespace process

namespace AS {
using namespace process;
}
