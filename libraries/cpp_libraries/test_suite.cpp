#include "libraries/cpp_libraries/process.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace process;

    // --- 1. Math Utils Tests ---
    assert(checkNumberType(10) == Even);
    assert(checkNumberType(7) == Odd);
    assert(validateNumber(5, 1, 10) == true);
    assert(validateNumber(15, 1, 10) == false);

    for (int i = 0; i < 100; ++i) {
        int r = randInt(1, 6);
        assert(r >= 1 && r <= 6);
    }
    std::cout << "[PASS] Math utils tests passed!\n";

    // --- 2. String Utils Tests ---
    assert(trimAll("   hello world   ") == "hello world");
    assert(trimLeft("   hi") == "hi");
    assert(trimRight("hi   ") == "hi");

    auto tokens = split("apple,banana,orange", ',');
    assert(tokens.size() == 3);
    assert(tokens[0] == "apple" && tokens[1] == "banana" && tokens[2] == "orange");

    auto byStringTokens = splitByString("user#//#pass#//#email", "#//#");
    assert(byStringTokens.size() == 3);
    assert(byStringTokens[0] == "user" && byStringTokens[1] == "pass" && byStringTokens[2] == "email");

    assert(replaceAll("Hello World World", "World", "C++") == "Hello C++ C++");
    assert(toLower("HELLO") == "hello");
    assert(toUpper("hello") == "HELLO");
    assert(numberToText(105, EN) == "one Hundred five");
    std::cout << "[PASS] String utils tests passed!\n";

    // --- 3. DateTime Tests ---
    Date d1{1, 1, 2024};
    Date d2{10, 1, 2024};

    assert(d1 < d2);
    assert(d2 > d1);
    assert(d1 != d2);
    assert(isBefore(d1, d2));
    assert(isAfter(d2, d1));

    Date d3 = d1 + 9;
    assert(d3 == d2);
    assert(d3 - 9 == d1);

    long long diff = d2 - d1;
    assert(diff == 9);

    Date leapDate{28, 2, 2024}; // 2024 is leap year
    ++leapDate;
    assert(leapDate.day == 29 && leapDate.month == 2);
    ++leapDate;
    assert(leapDate.day == 1 && leapDate.month == 3);

    // Overlap test
    Period p1{Date{1, 5, 2024}, Date{10, 5, 2024}};
    Period p2{Date{5, 5, 2024}, Date{15, 5, 2024}};
    assert(isOverlap(p1, p2));
    assert(countOverlapDays(p1, p2) == 5); // 10 - 5 = 5 days diff

    // Robust convertToDate
    Date parsed = convertToDate("15/08/2026");
    assert(parsed.day == 15 && parsed.month == 8 && parsed.year == 2026);
    Date invalidParsed = convertToDate("not/a/date");
    assert(invalidParsed.year == 0);

    // isValidDate
    assert(isValidDate(Date{29, 2, 2024}) == true);
    assert(isValidDate(Date{29, 2, 2023}) == false);

    std::cout << "[PASS] DateTime tests passed!\n";
    std::cout << "\nALL TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}

