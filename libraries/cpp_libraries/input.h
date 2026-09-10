#pragma once

#include "enumeration.h"
#include "process.h"
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace input {

// --- Declarations ---
std::string readString(
    std::string_view message = "Enter text: ",
    std::string_view errorMessage = "Error: Invalid input! try again.\n");
template <typename T>
T readNumber(std::string_view message = "Enter number: ",
             std::string_view errorMessage =
                 "Error: This is not a number! try again.\n");
template <typename T>
T readNumber(std::string_view message, T from,
             T to = std::numeric_limits<T>::max(),
             std::string_view typeErrorMessage =
                 "Error: This is not a number! try again.\n",
             std::string_view rangeErrorMessage =
                 "Error: This number is not in range! try again.\n");
bool readBool(std::string_view message = "Enter [0]False, [1]True : ",
              std::string_view typeErrorMessage =
                  "Error: This is not a number! try again.\n",
              std::string_view rangeErrorMessage =
                  "Error: This is not \"0\" or \"1\" ! try again.\n");
process::stDate readDate();
process::stPeriod readPeriod();
std::string readDateString(
    std::string_view message = "Enter Date: ",
    std::string_view errorMessage = "Error: Invalid Date!\n");

// =================================================================================
//  Inputs
// =================================================================================

inline std::string readString(std::string_view message,
                              std::string_view errorMessage) {
  std::string text = "";
  bool hasFailed = false;
  do {
    std::cout << message;
    std::getline(std::cin >> std::ws, text);
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << errorMessage;
      hasFailed = true;
    } else {
      hasFailed = false;
    }
  } while (hasFailed);
  return text;
}

template <typename T>
T readNumber(std::string_view message, std::string_view errorMessage) {
  T number;
  bool hasFailed = false;
  do {
    std::cout << message;
    std::cin >> number;
    if (std::cin.fail()) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << errorMessage;
      hasFailed = true;
    } else {
      hasFailed = false;
    }
  } while (hasFailed);
  return number;
}

template <typename T>
T readNumber(std::string_view message, T from, T to,
             std::string_view typeErrorMessage,
             std::string_view rangeErrorMessage) {
  T number;
  bool isOutOfRange = false;
  if (from > to) {
    std::swap(from, to);
  }
  do {
    number = readNumber<T>(message, typeErrorMessage);
    if (number < from || number > to) {
      isOutOfRange = true;
      std::cout << rangeErrorMessage;
    } else {
      isOutOfRange = false;
    }
  } while (isOutOfRange);
  return number;
}

inline bool readBool(std::string_view message,
                     std::string_view typeErrorMessage,
                     std::string_view rangeErrorMessage) {
  return readNumber<int>(message, 0, 1, typeErrorMessage, rangeErrorMessage) !=
         0;
}

inline process::stDate readDate() {
  process::stDate date;

  date.year =
      readNumber<size_t>("Enter Year: ", 0, std::numeric_limits<size_t>::max(),
                         "Error: Invalid Input!\n", "Error: Not in range!\n");

  date.month =
      readNumber<size_t>("Enter Month: ", 1, 12, "Error: Invalid Input!\n",
                         "Error: Not in range!\n");

  size_t maxDays = process::daysInMonth(date.month, date.year);

  date.day =
      readNumber<size_t>("Enter Day: ", 1, maxDays, "Error: Invalid Input!\n",
                         "Error: Not in range for this month/year!\n");

  return date;
}

inline process::stPeriod readPeriod() {
  process::stPeriod period;
  period.dateFrom = readDate();
  period.dateTo = readDate();
  return period;
}

inline std::string readDateString(std::string_view message,
                                 std::string_view errorMessage) {
  return readString(message, errorMessage);
}

} // namespace input

namespace AS {
using namespace input;
}
