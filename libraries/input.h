#pragma once

#include "process.h"
#include <iostream>
#include <string>
#include <string_view>
#include <limits>

namespace input {

// --- Declarations ---
std::string readString(std::string_view message = "Enter text: ", std::string_view errorMessage = "Error: Invalid input! try again.\n");
template <typename T> T readNumber(std::string_view message = "Enter number: ", std::string_view errorMessage = "Error: This is not a number! try again.\n");
template <typename T> T readNumber(std::string_view message, T from, T to = std::numeric_limits<T>::max(), std::string_view typeErrorMessage = "Error: This is not a number! try again.\n", std::string_view rangeErrorMessage = "Error: This number is not in range! try again.\n");
bool readBool(std::string_view message = "Enter [0]False, [1]True : ", std::string_view typeErrorMessage = "Error: This is not a number! try again.\n", std::string_view rangeErrorMessage = "Error: This is not \"0\" or \"1\" ! try again.\n");


// =================================================================================
//  Inputs
// =================================================================================

inline std::string readString(std::string_view message, std::string_view errorMessage) {
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
T readNumber(std::string_view message, T from, T to, std::string_view typeErrorMessage, std::string_view rangeErrorMessage) {
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

inline bool readBool(std::string_view message, std::string_view typeErrorMessage, std::string_view rangeErrorMessage) {
  return readNumber<int>(message, 0, 1, typeErrorMessage, rangeErrorMessage) != 0;
}

} // namespace input

namespace AS {
  using namespace input;
}
