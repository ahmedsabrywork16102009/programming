#pragma once

#include "enumeration.h"
#include <random>
#include <type_traits>
#include <utility>

namespace process {

/**
 * @brief Checks if an integral number is Odd or Even.
 * Enforces integral types at compile time via static_assert.
 */
template <typename T>
constexpr enumeration::enNumber checkNumberType(T number) noexcept {
    static_assert(std::is_integral<T>::value, "checkNumberType requires an integral type.");
    return (number & 1) ? enumeration::Odd : enumeration::Even;
}

/**
 * @brief Generates a pseudo-random integer in the range [from, to] (inclusive).
 * Uses high-quality Mersenne Twister (std::mt19937) instead of rand().
 */
inline int randInt(int from, int to) {
    if (from > to) {
        std::swap(from, to);
    }
    // thread_local ensures thread-safety and avoids repeated expensive seeding
    thread_local std::mt19937 rng([]() {
        std::random_device rd;
        return rd();
    }());
    std::uniform_int_distribution<int> dist(from, to);
    return dist(rng);
}

/**
 * @brief Validates if a number lies within the range [from, to] (inclusive).
 */
template <typename T>
constexpr bool validateNumber(T number, T from, T to) noexcept {
    return (number >= from) && (number <= to);
}

} // namespace process
