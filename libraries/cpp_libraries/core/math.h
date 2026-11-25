#pragma once

#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "../enumeration.h"

namespace process {

	// =============================================================
	//  Declarations
	// =============================================================

	template <typename T>
	constexpr enumeration::enNumber checkNumberType(T number) noexcept;

	inline int randInt(int from, int to);

	template <typename T>
	constexpr bool validateNumber(T number, T from, T to) noexcept;

	// --- sumIntegerRange: overflow-safe sum of integer arithmetic sequence ---
	// Uses an extended integer type when the compiler supports it.
	// Throws std::overflow_error on arithmetic overflow.
	template <typename T1, typename T2>
	auto sumIntegerRange(T1 from, T1 to, T2 step);

	// --- sumFloatingRange: precision-safe sum of floating-point arithmetic sequence ---
	// Uses long double internally; adapts tolerance to the input types' epsilon.
	// Throws std::invalid_argument / std::overflow_error on bad input.
	template <typename T1, typename T2>
	long double sumFloatingRange(T1 from, T1 to, T2 step);

	// --- sumRangeWithStep: unified dispatcher (integral → sumIntegerRange, float → sumFloatingRange) ---
	// step defaults to 1. Static assertions reject bool and types wider than 64-bit.
	template <typename T1, typename T2 = T1>
	auto sumRangeWithStep(T1 from, T1 to, T2 step = 1);

	// --- productIntegerRange: overflow-safe product (Π) of integer arithmetic sequence ---
	// Iterates all terms and multiplies; short-circuits to 0 if any term is 0.
	// Uses an extended integer type when the compiler supports it.
	// Throws std::invalid_argument if step == 0 or direction is inconsistent.
	// Throws std::overflow_error on multiplication overflow.
	template <typename T1, typename T2>
	auto productIntegerRange(T1 from, T1 to, T2 step);

	// --- productFloatingRange: product (Π) of floating-point arithmetic sequence ---
	// Uses long double internally. Throws on non-finite inputs or overflow to inf.
	template <typename T1, typename T2>
	long double productFloatingRange(T1 from, T1 to, T2 step);

	// --- productRangeWithStep: unified dispatcher ---
	// step defaults to 1.
	template <typename T1, typename T2 = T1>
	auto productRangeWithStep(T1 from, T1 to, T2 step = 1);

	// --- divideIntegerRange: sequential integer division (left-to-right) ---
	// Computes: term1 / term2 / term3 / ...  (integer division, loses fraction).
	// Throws std::domain_error if any divisor (term2 onward) is 0.
	// Returns long long (division shrinks magnitude).
	template <typename T1, typename T2>
	long long divideIntegerRange(T1 from, T1 to, T2 step);

	// --- divideFloatingRange: sequential floating-point division (left-to-right) ---
	// Throws std::domain_error on division by exact 0.0.
	// Throws std::overflow_error if result becomes non-finite.
	template <typename T1, typename T2>
	long double divideFloatingRange(T1 from, T1 to, T2 step);

	// --- divideRangeWithStep: unified dispatcher ---
	// step defaults to 1.
	template <typename T1, typename T2 = T1>
	auto divideRangeWithStep(T1 from, T1 to, T2 step = 1);

	// --- powerIntegerRange: sequential integer exponentiation (left-to-right) ---
	// Computes: ((term1 ^ term2) ^ term3) ^ ...
	// Negative exponent on integer base → result is 0 (truncation), except base ±1.
	// Throws std::domain_error if base is 0 and exponent is negative (0^-n undefined).
	// Throws std::overflow_error on exponentiation overflow.
	// Uses an extended integer type when the compiler supports it.
	template <typename T1, typename T2>
	auto powerIntegerRange(T1 from, T1 to, T2 step);

	// --- powerFloatingRange: sequential floating-point exponentiation (left-to-right) ---
	// Computes: powl(powl(term1, term2), term3) ...
	// Throws std::domain_error if base < 0 with non-integer exponent (would produce NaN).
	// Throws std::overflow_error if result becomes non-finite.
	template <typename T1, typename T2>
	long double powerFloatingRange(T1 from, T1 to, T2 step);

	// --- powerRangeWithStep: unified dispatcher ---
	// step defaults to 1.
	template <typename T1, typename T2 = T1>
	auto powerRangeWithStep(T1 from, T1 to, T2 step = 1);

	// =============================================================
	//  Internal Helpers
	// =============================================================

	// Counts terms in arithmetic sequence [start, end] with given increment.
	// Applies floating-point epsilon correction so near-exact endpoints are included.
	// Used by productFloatingRange, divideFloatingRange, powerFloatingRange.
	namespace detail {

		// Keep IntelliSense and compilers without a 128-bit extension on checked standard arithmetic.
#if defined(__SIZEOF_INT128__) && !defined(__INTELLISENSE__)
		using stWideInteger = __int128;
#else
		using stWideInteger = long long;
#endif

		inline bool addOverflow(stWideInteger left, stWideInteger right, stWideInteger &result) noexcept {
#if defined(__SIZEOF_INT128__) && !defined(__INTELLISENSE__)
			return __builtin_add_overflow(left, right, &result);
#else
			const stWideInteger maxValue = std::numeric_limits<stWideInteger>::max();
			const stWideInteger minValue = std::numeric_limits<stWideInteger>::min();
			if ((right > 0 && left > maxValue - right) || (right < 0 && left < minValue - right)) return true;
			result = left + right;
			return false;
#endif
		}

		inline bool subtractOverflow(stWideInteger left, stWideInteger right, stWideInteger &result) noexcept {
#if defined(__SIZEOF_INT128__) && !defined(__INTELLISENSE__)
			return __builtin_sub_overflow(left, right, &result);
#else
			const stWideInteger maxValue = std::numeric_limits<stWideInteger>::max();
			const stWideInteger minValue = std::numeric_limits<stWideInteger>::min();
			if ((right < 0 && left > maxValue + right) || (right > 0 && left < minValue + right)) return true;
			result = left - right;
			return false;
#endif
		}

		inline bool multiplyOverflow(stWideInteger left, stWideInteger right, stWideInteger &result) noexcept {
#if defined(__SIZEOF_INT128__) && !defined(__INTELLISENSE__)
			return __builtin_mul_overflow(left, right, &result);
#else
			const stWideInteger maxValue = std::numeric_limits<stWideInteger>::max();
			const stWideInteger minValue = std::numeric_limits<stWideInteger>::min();
			if (left == 0 || right == 0) {
				result = 0;
				return false;
			}
			if ((left == -1 && right == minValue) || (right == -1 && left == minValue)) return true;
			if (left > 0) {
				if ((right > 0 && left > maxValue / right) || (right < 0 && right < minValue / left)) return true;
			} else {
				if ((right > 0 && left < minValue / right) || (right < 0 && left < maxValue / right)) return true;
			}
			result = left * right;
			return false;
#endif
		}

		inline stWideInteger checkedAdd(stWideInteger left, stWideInteger right, const char *operation) {
			stWideInteger result;
			if (addOverflow(left, right, result)) throw std::overflow_error(operation);
			return result;
		}

		inline stWideInteger checkedSubtract(stWideInteger left, stWideInteger right, const char *operation) {
			stWideInteger result;
			if (subtractOverflow(left, right, result)) throw std::overflow_error(operation);
			return result;
		}

		inline stWideInteger checkedMultiply(stWideInteger left, stWideInteger right, const char *operation) {
			stWideInteger result;
			if (multiplyOverflow(left, right, result)) throw std::overflow_error(operation);
			return result;
		}

		template <typename T>
		inline stWideInteger toWideInteger(T value, const char *operation) {
#if !defined(__SIZEOF_INT128__) || defined(__INTELLISENSE__)
			if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T> && sizeof(T) >= sizeof(stWideInteger)) {
				if (value > static_cast<T>(std::numeric_limits<stWideInteger>::max())) throw std::overflow_error(operation);
			}
#endif
			return static_cast<stWideInteger>(value);
		}

		inline stWideInteger rangeTermCount(stWideInteger start, stWideInteger end, stWideInteger increment, const char *operation) {
			stWideInteger distance = checkedSubtract(end, start, operation);
			if (distance == std::numeric_limits<stWideInteger>::min() && increment == -1) throw std::overflow_error(operation);
			return checkedAdd(distance / increment, 1, operation);
		}

		struct stRangeInfo {
			bool valid;         // false → empty range (direction mismatch or step==0)
			long double count;  // number of terms
			long double last;   // value of the last term
		};

		inline stRangeInfo floatingRangeInfo(long double start, long double end, long double increment, long double epsilon1, long double epsilon2) {
			stRangeInfo info{false, 0.0L, 0.0L};

			if (increment == 0.0L) return info;

			if ((start > end && increment > 0.0L) || (start < end && increment < 0.0L)) return info;

			long double distance = end - start;
			if (!std::isfinite(distance)) throw std::overflow_error("range overflow");

			long double quotient = distance / increment;
			if (!std::isfinite(quotient) || quotient < 0.0L) throw std::overflow_error("invalid range");

			long double inputEpsilon = epsilon1 > epsilon2 ? epsilon1 : epsilon2;
			long double tolerance = 4.0L * inputEpsilon * (quotient > 1.0L ? quotient : 1.0L);
			long double nearest = std::round(quotient);

			if (std::abs(quotient - nearest) <= tolerance) quotient = nearest;

			info.count = std::floor(quotient) + 1.0L;
			info.last = std::fma(info.count - 1.0L, increment, start);

			if (!std::isfinite(info.last)) throw std::overflow_error("last term overflow");

			info.valid = true;
			return info;
		}

		// Safe integer power using fast exponentiation.
		// Throws overflow_error on overflow.
		// Handles negative exponents for integers (truncation rules).
		inline stWideInteger intPowWide(stWideInteger base, stWideInteger exp) {
			// 0 ^ negative → undefined
			if (base == 0 && exp < 0) throw std::domain_error("powerIntegerRange: 0 raised to negative exponent");

			// any^0 == 1
			if (exp == 0) return 1;

			// 0^positive == 0
			if (base == 0) return 0;
			if (base == 1) return 1;

			// negative exponent: integer truncation rules
			// |base| > 1 → 1/|base|^n rounds to 0
			// base == -1 → result is (-1)^|exp|
			if (exp < 0) {
				if (base == -1) return (exp % 2 == 0) ? 1 : -1;
				return 0;  // |base| > 1 → truncates to 0
			}

			stWideInteger result = 1;
			while (exp > 0) {
				if (exp & 1) {
					if (multiplyOverflow(result, base, result)) throw std::overflow_error("powerIntegerRange: overflow");
				}
				exp >>= 1;
				if (exp > 0) {
					if (multiplyOverflow(base, base, base)) throw std::overflow_error("powerIntegerRange: overflow during squaring");
				}
			}
			return result;
		}

	}  // namespace detail

	// =============================================================
	//  Implementations — Existing (sum)
	// =============================================================

	template <typename T>
	constexpr enumeration::enNumber checkNumberType(T number) noexcept {
		static_assert(std::is_integral<T>::value, "checkNumberType requires an integral type.");
		return (number & 1) ? enumeration::Odd : enumeration::Even;
	}

	inline int randInt(int from, int to) {
		if (from > to) {
			std::swap(from, to);
		}
		thread_local std::mt19937 rng([]() {
			std::random_device rd;
			return rd();
		}());
		std::uniform_int_distribution<int> dist(from, to);
		return dist(rng);
	}

	template <typename T>
	constexpr bool validateNumber(T number, T from, T to) noexcept {
		return (number >= from) && (number <= to);
	}

	template <typename T1, typename T2>
	auto sumIntegerRange(T1 from, T1 to, T2 step) {
		using ResultType = std::conditional_t<sizeof(T1) >= 8 || sizeof(T2) >= 8, detail::stWideInteger, long long>;

		detail::stWideInteger start = detail::toWideInteger(from, "sumIntegerRange: input overflow");
		detail::stWideInteger end = detail::toWideInteger(to, "sumIntegerRange: input overflow");
		detail::stWideInteger increment = detail::toWideInteger(step, "sumIntegerRange: step overflow");

		if (increment == 0) return ResultType(0);

		if ((start > end && increment > 0) || (start < end && increment < 0)) return ResultType(0);

		detail::stWideInteger count = detail::rangeTermCount(start, end, increment, "sumIntegerRange: range overflow");
		detail::stWideInteger offset, last, endpoints, result;

		if (detail::multiplyOverflow(count - 1, increment, offset) || detail::addOverflow(start, offset, last)) throw std::overflow_error("sumIntegerRange: last term overflow");

		if (detail::addOverflow(start, last, endpoints)) throw std::overflow_error("sumIntegerRange: endpoint sum overflow");

		detail::stWideInteger factor = count;

		if (factor % 2 == 0)
			factor /= 2;
		else
			endpoints /= 2;

		if (detail::multiplyOverflow(factor, endpoints, result)) throw std::overflow_error("sumIntegerRange: sum overflow");

		if constexpr (std::is_same_v<ResultType, long long>) {
			if (result > static_cast<detail::stWideInteger>(std::numeric_limits<long long>::max()) || result < static_cast<detail::stWideInteger>(std::numeric_limits<long long>::min())) throw std::overflow_error("sumIntegerRange: result overflow");
			return static_cast<long long>(result);
		} else {
			return result;
		}
	}

	template <typename T1, typename T2>
	long double sumFloatingRange(T1 from, T1 to, T2 step) {
		long double start = from;
		long double end = to;
		long double increment = step;

		if (!std::isfinite(start) || !std::isfinite(end) || !std::isfinite(increment)) throw std::invalid_argument("sumFloatingRange: non-finite input");

		if (increment == 0.0L) return 0.0L;

		if ((start > end && increment > 0) || (start < end && increment < 0)) return 0.0L;

		long double distance = end - start;

		if (!std::isfinite(distance)) throw std::overflow_error("sumFloatingRange: range overflow");

		long double quotient = distance / increment;

		if (!std::isfinite(quotient) || quotient < 0) throw std::overflow_error("sumFloatingRange: invalid range");

		long double epsilon1 = std::numeric_limits<T1>::epsilon();
		long double epsilon2 = std::numeric_limits<T2>::epsilon();
		long double inputEpsilon = epsilon1 > epsilon2 ? epsilon1 : epsilon2;

		long double maxReliableQuotient = 1.0L / (16.0L * inputEpsilon);

		if (quotient >= maxReliableQuotient) throw std::overflow_error("sumFloatingRange: too many terms");

		long double tolerance = 4.0L * inputEpsilon * (quotient > 1.0L ? quotient : 1.0L);
		long double nearest = std::round(quotient);

		if (std::abs(quotient - nearest) <= tolerance) quotient = nearest;

		long double count = std::floor(quotient) + 1.0L;
		long double last = std::fma(count - 1.0L, increment, start);

		if (!std::isfinite(last)) throw std::overflow_error("sumFloatingRange: last term overflow");

		long double average = start / 2.0L + last / 2.0L;
		long double result = std::fma(count, average, 0.0L);

		if (!std::isfinite(result)) throw std::overflow_error("sumFloatingRange: sum overflow");

		return result;
	}

	template <typename T1, typename T2>
	auto sumRangeWithStep(T1 from, T1 to, T2 step) {
		static_assert(std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value, "sumRangeWithStep: T1 and T2 must be arithmetic types.");
		static_assert(!std::is_same<T1, bool>::value && !std::is_same<T2, bool>::value, "sumRangeWithStep: bool is not a valid numeric type.");
		static_assert(!std::is_integral<T1>::value || sizeof(T1) <= 8, "sumRangeWithStep: integer types wider than 64-bit are not supported.");
		static_assert(!std::is_integral<T2>::value || sizeof(T2) <= 8, "sumRangeWithStep: integer types wider than 64-bit are not supported.");

		if constexpr (std::is_integral<T1>::value && std::is_integral<T2>::value)
			return sumIntegerRange(from, to, step);
		else
			return sumFloatingRange(from, to, step);
	}

	// =============================================================
	//  Implementations — Product
	// =============================================================

	template <typename T1, typename T2>
	auto productIntegerRange(T1 from, T1 to, T2 step) {
		using ResultType = std::conditional_t<sizeof(T1) >= 8 || sizeof(T2) >= 8, detail::stWideInteger, long long>;

		detail::stWideInteger start = detail::toWideInteger(from, "productIntegerRange: input overflow");
		detail::stWideInteger end = detail::toWideInteger(to, "productIntegerRange: input overflow");
		detail::stWideInteger increment = detail::toWideInteger(step, "productIntegerRange: step overflow");

		if (increment == 0) throw std::invalid_argument("productIntegerRange: step cannot be zero");

		if ((start > end && increment > 0) || (start < end && increment < 0)) throw std::invalid_argument("productIntegerRange: step direction inconsistent with range");

		detail::stWideInteger count = detail::rangeTermCount(start, end, increment, "productIntegerRange: range overflow");
		detail::stWideInteger result = 1;
		detail::stWideInteger term = start;

		for (detail::stWideInteger i = 0; i < count; ++i) {
			if (term == 0) return ResultType(0);  // any factor is 0 → product is 0

			if (detail::multiplyOverflow(result, term, result)) throw std::overflow_error("productIntegerRange: overflow");
			if (i + 1 < count) term = detail::checkedAdd(term, increment, "productIntegerRange: range overflow");
		}

		if constexpr (std::is_same_v<ResultType, long long>) {
			if (result > static_cast<detail::stWideInteger>(std::numeric_limits<long long>::max()) || result < static_cast<detail::stWideInteger>(std::numeric_limits<long long>::min())) throw std::overflow_error("productIntegerRange: result overflow");
			return static_cast<long long>(result);
		} else {
			return result;
		}
	}

	template <typename T1, typename T2>
	long double productFloatingRange(T1 from, T1 to, T2 step) {
		long double start = from;
		long double end = to;
		long double increment = step;

		if (!std::isfinite(start) || !std::isfinite(end) || !std::isfinite(increment)) throw std::invalid_argument("productFloatingRange: non-finite input");

		if (increment == 0.0L) throw std::invalid_argument("productFloatingRange: step cannot be zero");

		if ((start > end && increment > 0.0L) || (start < end && increment < 0.0L)) throw std::invalid_argument("productFloatingRange: step direction inconsistent with range");

		long double epsilon1 = std::numeric_limits<T1>::epsilon();
		long double epsilon2 = std::numeric_limits<T2>::epsilon();
		auto info = detail::floatingRangeInfo(start, end, increment, epsilon1, epsilon2);

		long double result = 1.0L;
		long double term = start;

		for (long double i = 0.0L; i < info.count; i += 1.0L, term += increment) {
			result *= term;
			if (!std::isfinite(result)) throw std::overflow_error("productFloatingRange: overflow to infinity");
		}

		return result;
	}

	template <typename T1, typename T2>
	auto productRangeWithStep(T1 from, T1 to, T2 step) {
		static_assert(std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value, "productRangeWithStep: T1 and T2 must be arithmetic types.");
		static_assert(!std::is_same<T1, bool>::value && !std::is_same<T2, bool>::value, "productRangeWithStep: bool is not a valid numeric type.");
		static_assert(!std::is_integral<T1>::value || sizeof(T1) <= 8, "productRangeWithStep: integer types wider than 64-bit are not supported.");
		static_assert(!std::is_integral<T2>::value || sizeof(T2) <= 8, "productRangeWithStep: integer types wider than 64-bit are not supported.");

		if constexpr (std::is_integral<T1>::value && std::is_integral<T2>::value)
			return productIntegerRange(from, to, step);
		else
			return productFloatingRange(from, to, step);
	}

	// =============================================================
	//  Implementations — Divide
	// =============================================================

	template <typename T1, typename T2>
	long long divideIntegerRange(T1 from, T1 to, T2 step) {
		detail::stWideInteger start = detail::toWideInteger(from, "divideIntegerRange: input overflow");
		detail::stWideInteger end = detail::toWideInteger(to, "divideIntegerRange: input overflow");
		detail::stWideInteger increment = detail::toWideInteger(step, "divideIntegerRange: step overflow");

		if (increment == 0) throw std::invalid_argument("divideIntegerRange: step cannot be zero");

		if ((start > end && increment > 0) || (start < end && increment < 0)) throw std::invalid_argument("divideIntegerRange: step direction inconsistent with range");

		detail::stWideInteger count = detail::rangeTermCount(start, end, increment, "divideIntegerRange: range overflow");

		// result starts as first term
		detail::stWideInteger result = start;
		detail::stWideInteger term = start;

		// divide by each subsequent term
		for (detail::stWideInteger i = 1; i < count; ++i) {
			term = detail::checkedAdd(term, increment, "divideIntegerRange: range overflow");
			if (term == 0) throw std::domain_error("divideIntegerRange: division by zero (a term in the sequence is 0)");
			if (result == std::numeric_limits<detail::stWideInteger>::min() && term == -1) throw std::overflow_error("divideIntegerRange: result overflow");
			result /= term;
		}

		if (result > static_cast<detail::stWideInteger>(std::numeric_limits<long long>::max()) || result < static_cast<detail::stWideInteger>(std::numeric_limits<long long>::min())) throw std::overflow_error("divideIntegerRange: result overflow");

		return static_cast<long long>(result);
	}

	template <typename T1, typename T2>
	long double divideFloatingRange(T1 from, T1 to, T2 step) {
		long double start = from;
		long double end = to;
		long double increment = step;

		if (!std::isfinite(start) || !std::isfinite(end) || !std::isfinite(increment)) throw std::invalid_argument("divideFloatingRange: non-finite input");

		if (increment == 0.0L) throw std::invalid_argument("divideFloatingRange: step cannot be zero");

		if ((start > end && increment > 0.0L) || (start < end && increment < 0.0L)) throw std::invalid_argument("divideFloatingRange: step direction inconsistent with range");

		long double epsilon1 = std::numeric_limits<T1>::epsilon();
		long double epsilon2 = std::numeric_limits<T2>::epsilon();
		auto info = detail::floatingRangeInfo(start, end, increment, epsilon1, epsilon2);

		// result starts as first term
		long double result = start;
		long double term = start + increment;

		for (long double i = 1.0L; i < info.count; i += 1.0L, term += increment) {
			if (term == 0.0L) throw std::domain_error("divideFloatingRange: division by zero (a term in the sequence is 0)");
			result /= term;
			if (!std::isfinite(result)) throw std::overflow_error("divideFloatingRange: result became non-finite");
		}

		return result;
	}

	template <typename T1, typename T2>
	auto divideRangeWithStep(T1 from, T1 to, T2 step) {
		static_assert(std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value, "divideRangeWithStep: T1 and T2 must be arithmetic types.");
		static_assert(!std::is_same<T1, bool>::value && !std::is_same<T2, bool>::value, "divideRangeWithStep: bool is not a valid numeric type.");
		static_assert(!std::is_integral<T1>::value || sizeof(T1) <= 8, "divideRangeWithStep: integer types wider than 64-bit are not supported.");
		static_assert(!std::is_integral<T2>::value || sizeof(T2) <= 8, "divideRangeWithStep: integer types wider than 64-bit are not supported.");

		if constexpr (std::is_integral<T1>::value && std::is_integral<T2>::value)
			return divideIntegerRange(from, to, step);
		else
			return divideFloatingRange(from, to, step);
	}

	// =============================================================
	//  Implementations — Power
	// =============================================================

	template <typename T1, typename T2>
	auto powerIntegerRange(T1 from, T1 to, T2 step) {
		using ResultType = std::conditional_t<sizeof(T1) >= 8 || sizeof(T2) >= 8, detail::stWideInteger, long long>;

		detail::stWideInteger start = detail::toWideInteger(from, "powerIntegerRange: input overflow");
		detail::stWideInteger end = detail::toWideInteger(to, "powerIntegerRange: input overflow");
		detail::stWideInteger increment = detail::toWideInteger(step, "powerIntegerRange: step overflow");

		if (increment == 0) throw std::invalid_argument("powerIntegerRange: step cannot be zero");

		if ((start > end && increment > 0) || (start < end && increment < 0)) throw std::invalid_argument("powerIntegerRange: step direction inconsistent with range");

		detail::stWideInteger count = detail::rangeTermCount(start, end, increment, "powerIntegerRange: range overflow");

		// result starts as first term; subsequent terms are exponents (left-to-right)
		detail::stWideInteger result = start;
		detail::stWideInteger term = start;

		for (detail::stWideInteger i = 1; i < count; ++i) {
			term = detail::checkedAdd(term, increment, "powerIntegerRange: range overflow");
			// 0 ^ negative exponent is undefined
			if (result == 0 && term < 0) throw std::domain_error("powerIntegerRange: 0 raised to negative exponent");
			result = detail::intPowWide(result, term);
		}

		if constexpr (std::is_same_v<ResultType, long long>) {
			if (result > static_cast<detail::stWideInteger>(std::numeric_limits<long long>::max()) || result < static_cast<detail::stWideInteger>(std::numeric_limits<long long>::min())) throw std::overflow_error("powerIntegerRange: result overflow");
			return static_cast<long long>(result);
		} else {
			return result;
		}
	}

	template <typename T1, typename T2>
	long double powerFloatingRange(T1 from, T1 to, T2 step) {
		long double start = from;
		long double end = to;
		long double increment = step;

		if (!std::isfinite(start) || !std::isfinite(end) || !std::isfinite(increment)) throw std::invalid_argument("powerFloatingRange: non-finite input");

		if (increment == 0.0L) throw std::invalid_argument("powerFloatingRange: step cannot be zero");

		if ((start > end && increment > 0.0L) || (start < end && increment < 0.0L)) throw std::invalid_argument("powerFloatingRange: step direction inconsistent with range");

		long double epsilon1 = std::numeric_limits<T1>::epsilon();
		long double epsilon2 = std::numeric_limits<T2>::epsilon();
		auto info = detail::floatingRangeInfo(start, end, increment, epsilon1, epsilon2);

		// result starts as first term; subsequent terms are exponents (left-to-right)
		long double result = start;
		long double term = start + increment;

		for (long double i = 1.0L; i < info.count; i += 1.0L, term += increment) {
			// negative base with non-integer exponent produces NaN
			if (result < 0.0L) {
				long double rounded = std::round(term);
				if (std::abs(term - rounded) > 4.0L * std::numeric_limits<long double>::epsilon()) throw std::domain_error("powerFloatingRange: negative base raised to non-integer exponent (result would be complex)");
			}
			// base 0 with negative exponent is undefined
			if (result == 0.0L && term < 0.0L) throw std::domain_error("powerFloatingRange: 0 raised to negative exponent");

			result = std::powl(result, term);

			if (std::isnan(result)) throw std::domain_error("powerFloatingRange: result is NaN");
			if (!std::isfinite(result)) throw std::overflow_error("powerFloatingRange: result became infinite");
		}

		return result;
	}

	template <typename T1, typename T2>
	auto powerRangeWithStep(T1 from, T1 to, T2 step) {
		static_assert(std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value, "powerRangeWithStep: T1 and T2 must be arithmetic types.");
		static_assert(!std::is_same<T1, bool>::value && !std::is_same<T2, bool>::value, "powerRangeWithStep: bool is not a valid numeric type.");
		static_assert(!std::is_integral<T1>::value || sizeof(T1) <= 8, "powerRangeWithStep: integer types wider than 64-bit are not supported.");
		static_assert(!std::is_integral<T2>::value || sizeof(T2) <= 8, "powerRangeWithStep: integer types wider than 64-bit are not supported.");

		if constexpr (std::is_integral<T1>::value && std::is_integral<T2>::value)
			return powerIntegerRange(from, to, step);
		else
			return powerFloatingRange(from, to, step);
	}

}  // namespace process

namespace AS {
	using namespace process;
}  // namespace AS
