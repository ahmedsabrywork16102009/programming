#pragma once

namespace enumeration {

enum enNumber { Odd = 1, Even = 2 };

enum enCharCase { Upper = 1, Lower = 2, Invert = 3 };

enum enTrim { TrimLeft = 1, TrimRight = 2, TrimAll = 3 };

enum enLanguage { EN, AR };

enum enWeekday {
  Saturday = 1,
  Sunday = 2,
  Monday = 3,
  Tuesday = 4,
  Wednesday = 5,
  Thursday = 6,
  Friday = 7,
};

enum enMonth {
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

enum enOperation { Sum = 1, Subtract = 2, Multiply = 3, Divide = 4, MixOp = 5 };

} // namespace enumeration

namespace AS {
  using namespace enumeration;
}
