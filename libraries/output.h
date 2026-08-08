#pragma once

#include "enumeration.h"
#include "process.h"
#include <iostream>
#include <string_view>
#include <vector>
#include <iomanip>
#include <cstdlib>

namespace output {

using namespace enumeration;
using namespace process;

// --- Declarations ---
void printString(std::string_view string);
void printStringWithSuffix(std::string_view string, std::string_view suffix);
void printString(std::string_view prefix, std::string_view string);
void printString(std::string_view prefix, std::string_view string, std::string_view suffix);
void printNumberType(int number, std::string_view evenMessage = "Number is Even.", std::string_view oddMessage = "Number is Odd.");
void printVector(const std::vector<int> &vNumbers, std::string_view separator = " ");
void printVector(const std::vector<char> &vChars, std::string_view separator = " ");
void printVector(const std::vector<std::string> &vTokens, std::string_view separator = " ");
void printEachWordInLine(std::string_view text, char delimiter = ' ', std::string_view separator = "\n");
std::string getWeekdayName(enWeekdays weekday) noexcept;
void resetScreen(bool clearScreen = true, bool color = true);
void printMonthCalendar(const size_t year, const enMonths month);


// =================================================================================
//  Outputs
// =================================================================================

inline void printString(std::string_view string) {
  std::cout << string;
}

inline void printStringWithSuffix(std::string_view string, std::string_view suffix) {
  std::cout << string << suffix;
}

inline void printString(std::string_view prefix, std::string_view string) {
  std::cout << prefix << string;
}

inline void printString(std::string_view prefix, std::string_view string, std::string_view suffix) {
  std::cout << prefix << string << suffix;
}

inline void printNumberType(int number, std::string_view evenMessage, std::string_view oddMessage) {
  if (checkNumberType(number) == enNumberType::Even) {
    std::cout << evenMessage;
  } else {
    std::cout << oddMessage;
  }
}

inline void printVector(const std::vector<int> &vNumbers, std::string_view separator) {
  for (size_t i = 0; i < vNumbers.size(); i++) {
    std::cout << vNumbers[i];
    if ((i + 1) != vNumbers.size()) {
      std::cout << separator;
    }
  }
}

inline void printVector(const std::vector<char> &vChars, std::string_view separator) {
  for (size_t i = 0; i < vChars.size(); i++) {
    std::cout << vChars[i];
    if ((i + 1) != vChars.size()) {
      std::cout << separator;
    }
  }
}

inline void printVector(const std::vector<std::string> &vTokens, std::string_view separator) {
  for (size_t i = 0; i < vTokens.size(); i++) {
    std::cout << vTokens[i];
    if ((i + 1) != vTokens.size()) {
      std::cout << separator;
    }
  }
}

inline void printEachWordInLine(std::string_view text, char delimiter, std::string_view separator) {
  if (text.empty()) {
    return;
  }
  std::string word = "";
  for (auto c : text) {
    if (c != delimiter) {
      word.push_back(c);
    } else if (!word.empty()) {
      std::cout << word << separator;
      word.clear();
    }
  }
  if (!word.empty()) {
    std::cout << word << separator;
  }
}

inline std::string getWeekdayName(enWeekdays weekday) noexcept {
  switch (weekday) {
  case Saturday: return "Saturday";
  case Sunday: return "Sunday";
  case Monday: return "Monday";
  case Tuesday: return "Tuesday";
  case Wednesday: return "Wednesday";
  case Thursday: return "Thursday";
  case Friday: return "Friday";
  }
  return "Invalid weekday";
}

inline void resetScreen(bool clearScreen, bool color) {
  if (clearScreen) {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
  }
  if (color) {
#ifdef _WIN32
    std::system("color 07");
#endif
  }
}

inline void printMonthCalendar(const size_t year, const enMonths month) {
  size_t numberOfDaysInMonth = daysInMonth(year, month);
  enWeekdays startDay = getWeekdayNumber(1, month, year);
  size_t currentDayVal = static_cast<size_t>(startDay);

  std::cout << "\n\n\n___________________" << getMonth(month)
            << "___________________\n\n";
  std::cout << "\tSun Mon Tue Wed Thu Fri Sat\n";
  
  for (size_t i = 1; i < currentDayVal; ++i) {
    std::cout << "\t";
  }

  for (size_t day = 1; day <= numberOfDaysInMonth; day++) {
    std::cout << "\t" << std::setw(3) << std::right << day;
    if ((day + currentDayVal - 1) % 7 == 0) {
      std::cout << "\n";
    }
  }
  std::cout << "\n";
}

} // namespace output

namespace AS {
  using namespace output;
}
