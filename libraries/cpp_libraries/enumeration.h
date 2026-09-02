#pragma once

namespace enumeration {

enum enNumberType { Odd = 1, Even = 2 };

enum enCharsType { Upper = 1, Lower = 2, Invert = 3 };

enum enTrimType { TrimLeft = 1, TrimRight = 2, TrimAll = 3 };

enum enLanguage { EN, AR };

enum enWeekdays {
  Sunday = 1,
  Monday = 2,
  Tuesday = 3,
  Wednesday = 4,
  Thursday = 5,
  Friday = 6,
  Saturday = 7
};

enum enMonths {
  January = 1,
  February = 2,
  March = 3,
  April = 4,
  May = 5,
  June = 6,
  July = 7,
  August = 8,
  September = 9,
  October = 10,
  November = 11,
  December = 12
};

enum enOperationType { sum = 1, subtract = 2, multiply = 3, divide = 4, mixOp = 5 };

} // namespace enumeration

namespace AS {
  using namespace enumeration;
}
