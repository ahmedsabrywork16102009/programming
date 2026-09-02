#pragma once

#include "enumeration.h"
#include "process.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace output {

using namespace enumeration;
using namespace process;

// --- Declarations ---
void printString(std::string_view string);
void printStringWithSuffix(std::string_view string, std::string_view suffix);
void printString(std::string_view prefix, std::string_view string);
void printString(std::string_view prefix, std::string_view string,
                 std::string_view suffix);
void printNumberType(int number,
                     std::string_view evenMessage = "Number is Even.",
                     std::string_view oddMessage = "Number is Odd.");
void printVector(const std::vector<int> &vNumbers,
                 std::string_view separator = " ");
void printVector(const std::vector<char> &vChars,
                 std::string_view separator = " ");
void printVector(const std::vector<std::string> &vTokens,
                 std::string_view separator = " ");
void printEachWordInLine(std::string_view text, char delimiter = ' ',
                         std::string_view separator = "\n");
std::string getWeekdayName(enWeekdays weekday) noexcept;
void resetScreen(bool clearScreen = true, bool color = true);
void printMonthCalendar(const enMonths month, const std::size_t year);

// =================================================================================
//  Outputs
// =================================================================================

inline void printString(std::string_view string) { std::cout << string; }

inline void printStringWithSuffix(std::string_view string,
                                  std::string_view suffix) {
  std::cout << string << suffix;
}

inline void printString(std::string_view prefix, std::string_view string) {
  std::cout << prefix << string;
}

inline void printString(std::string_view prefix, std::string_view string,
                        std::string_view suffix) {
  std::cout << prefix << string << suffix;
}

inline void printNumberType(int number, std::string_view evenMessage,
                            std::string_view oddMessage) {
  if (checkNumberType(number) == enNumberType::Even) {
    std::cout << evenMessage;
  } else {
    std::cout << oddMessage;
  }
}

inline void printVector(const std::vector<int> &vNumbers,
                        std::string_view separator) {
  for (size_t i = 0; i < vNumbers.size(); i++) {
    std::cout << vNumbers[i];
    if ((i + 1) != vNumbers.size()) {
      std::cout << separator;
    }
  }
}

inline void printVector(const std::vector<char> &vChars,
                        std::string_view separator) {
  for (size_t i = 0; i < vChars.size(); i++) {
    std::cout << vChars[i];
    if ((i + 1) != vChars.size()) {
      std::cout << separator;
    }
  }
}

inline void printVector(const std::vector<std::string> &vTokens,
                        std::string_view separator) {
  for (size_t i = 0; i < vTokens.size(); i++) {
    std::cout << vTokens[i];
    if ((i + 1) != vTokens.size()) {
      std::cout << separator;
    }
  }
}

inline void printEachWordInLine(std::string_view text, char delimiter,
                                std::string_view separator) {
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

inline void printMonthCalendar(const enMonths month, const std::size_t year) {

  size_t current = getWeekdayNumber(1, month, year) - 1;
  size_t numberOfDays = daysInMonth(month, year);

  printf("\n  ______________ %s ______________\n\n",
         getMonthName(month, true).c_str());

  std::cout << "  Sun  Mon  Tue  Wed  Thu  Fri  Sat\n";

  int i;
  for (i = 0; i < current; i++) {
    printf("     ");
  }

  for (int j = 1; j <= numberOfDays; j++) {
    printf("%5d", j);

    if (++i == 7) {
      i = 0;
      printf("\n");
    }
  }
  printf("\n  _________________________________\n");
}
inline void printYearCalendar(const std::size_t year) {
  const std::string calendarHeader = "Calendar - " + std::to_string(year);

  std::cout << "_______________________________________________\n";
  std::cout << std::setw(48) << calendarHeader << "\n";
  std::cout << "_______________________________________________\n\n";

  for (int month = 1; month <= 12; ++month) {
    printMonthCalendar(static_cast<enMonths>(month), year);
    std::cout << "\n";
  }
}

} // namespace output

namespace AS {
using namespace output;
}
