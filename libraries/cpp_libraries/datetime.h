#pragma once

#include "enumeration.h"
#include "string_utils.h"

#include <cstddef>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>

namespace process {

struct stDate {
    std::size_t year = 0;
    std::size_t month = 0;
    std::size_t day = 0;

    stDate() = default;
    constexpr stDate(std::size_t d, std::size_t m, std::size_t y) noexcept
        : year(y), month(m), day(d) {}

    // Comparison operators (Fully portable across C++11/14/17/20/23)
    bool operator==(const stDate &other) const noexcept {
        return year == other.year && month == other.month && day == other.day;
    }
    bool operator!=(const stDate &other) const noexcept {
        return !(*this == other);
    }
    bool operator<(const stDate &other) const noexcept {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }
    bool operator<=(const stDate &other) const noexcept {
        return !(other < *this);
    }
    bool operator>(const stDate &other) const noexcept {
        return other < *this;
    }
    bool operator>=(const stDate &other) const noexcept {
        return !(*this < other);
    }

    stDate operator+(long long days) const;
    stDate operator-(long long days) const;
    long long operator-(const stDate &other) const;

    stDate &operator+=(long long days);
    stDate &operator-=(long long days);

    stDate &operator++();
    stDate operator++(int);
    stDate &operator--();
    stDate operator--(int);

    friend std::ostream &operator<<(std::ostream &os, const stDate &d) {
        return os << d.day << '/' << d.month << '/' << d.year;
    }
};

using Date = stDate;

struct stPeriod {
    stDate dateFrom;
    stDate dateTo;
};

using Period = stPeriod;

// --- Calendar Math ---

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
    if (month < 1 || month > 12) return 0;
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

inline std::string joinDate(size_t day, size_t month, size_t year) noexcept {
    return std::to_string(day) + "/" + std::to_string(month) + "/" + std::to_string(year);
}

inline enumeration::enWeekday getWeekday(size_t day, size_t month, size_t year) noexcept {
    int monthOffset = (14 - static_cast<int>(month)) / 12;
    int adjustedYear = static_cast<int>(year) - monthOffset;
    int adjustedMonth = static_cast<int>(month) + 12 * monthOffset - 2;
    int weekdayIndex = (static_cast<int>(day) + adjustedYear + adjustedYear / 4 -
                        adjustedYear / 100 + adjustedYear / 400 +
                        (31 * adjustedMonth) / 12 + 1) % 7;
    return static_cast<enumeration::enWeekday>(weekdayIndex + 1);
}

inline enumeration::enWeekday getWeekday(const stDate &date) noexcept {
    return getWeekday(date.day, date.month, date.year);
}

inline bool isLastDayInWeek(enumeration::enWeekday weekday) noexcept {
    return weekday == enumeration::Friday;
}

inline bool isLastDayInWeek(const stDate &date) noexcept {
    return isLastDayInWeek(getWeekday(date));
}

inline bool isWeekend(enumeration::enWeekday weekday) noexcept {
    return (weekday == enumeration::Friday || weekday == enumeration::Saturday);
}

inline bool isWeekend(size_t day, size_t month, size_t year) noexcept {
    return isWeekend(getWeekday(day, month, year));
}

inline bool isWeekend(const stDate &date) noexcept {
    return isWeekend(getWeekday(date));
}

inline bool isBusinessDay(enumeration::enWeekday weekday) noexcept {
    return !isWeekend(weekday);
}

inline bool isBusinessDay(size_t day, size_t month, size_t year) noexcept {
    return isBusinessDay(getWeekday(day, month, year));
}

inline bool isBusinessDay(const stDate &date) noexcept {
    return isBusinessDay(getWeekday(date));
}

inline size_t daysLeftInWeek(const stDate &date) noexcept {
    return static_cast<size_t>(enumeration::Friday - getWeekday(date)) + 1;
}

inline size_t daysLeftInMonth(const stDate &date) noexcept {
    return daysInMonth(date.month, date.year) - date.day + 1;
}

inline size_t daysLeftInYear(stDate date) noexcept {
    for (size_t i = 1; i < date.month; ++i) {
        date.day += daysInMonth(i, date.year);
    }
    return daysInYear(date.year) - date.day + 1;
}

inline std::string getMonthName(enumeration::enMonth month, bool fullName = true) noexcept {
    if (fullName) {
        switch (month) {
        case enumeration::January:   return "January";
        case enumeration::February:  return "February";
        case enumeration::March:     return "March";
        case enumeration::April:     return "April";
        case enumeration::May:       return "May";
        case enumeration::June:      return "June";
        case enumeration::July:      return "July";
        case enumeration::August:    return "August";
        case enumeration::September: return "September";
        case enumeration::October:   return "October";
        case enumeration::November:  return "November";
        case enumeration::December:  return "December";
        }
    } else {
        switch (month) {
        case enumeration::January:   return "Jan";
        case enumeration::February:  return "Feb";
        case enumeration::March:     return "Mar";
        case enumeration::April:     return "Apr";
        case enumeration::May:       return "May";
        case enumeration::June:      return "Jun";
        case enumeration::July:      return "Jul";
        case enumeration::August:    return "Aug";
        case enumeration::September: return "Sep";
        case enumeration::October:   return "Oct";
        case enumeration::November:  return "Nov";
        case enumeration::December:  return "Dec";
        }
    }
    return "Invalid month";
}

inline std::string getWeekdayName(enumeration::enWeekday weekday, bool shortName = false) noexcept {
    if (shortName) {
        switch (weekday) {
        case enumeration::Saturday:  return "Sat";
        case enumeration::Sunday:    return "Sun";
        case enumeration::Monday:    return "Mon";
        case enumeration::Tuesday:   return "Tue";
        case enumeration::Wednesday: return "Wed";
        case enumeration::Thursday:  return "Thu";
        case enumeration::Friday:    return "Fri";
        }
    }
    switch (weekday) {
    case enumeration::Saturday:  return "Saturday";
    case enumeration::Sunday:    return "Sunday";
    case enumeration::Monday:    return "Monday";
    case enumeration::Tuesday:   return "Tuesday";
    case enumeration::Wednesday: return "Wednesday";
    case enumeration::Thursday:  return "Thursday";
    case enumeration::Friday:    return "Friday";
    }
    return "Invalid weekday";
}

inline size_t dayOfYear(size_t day, size_t month, size_t year) noexcept {
    if (month > 12 || month < 1 || day < 1 || day > daysInMonth(month, year)) {
        return 0;
    }
    size_t totalDays = 0;
    for (size_t i = 1; i < month; ++i) {
        totalDays += daysInMonth(i, year);
    }
    return totalDays + day;
}

inline stDate getDateFromDayOfYear(size_t year, size_t numberOfDays, size_t daysAdd = 0) {
    stDate date;
    date.year = year;
    date.month = 1;
    date.day = numberOfDays + daysAdd;

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

// Retain legacy comparative functions for backward compatibility:
inline bool isAfter(const stDate &date1, const stDate &date2) noexcept {
    return date1 > date2;
}

inline bool isEqual(const stDate &date1, const stDate &date2) noexcept {
    return date1 == date2;
}

inline bool isBefore(const stDate &date1, const stDate &date2) noexcept {
    return date1 < date2;
}

inline bool isLastDayInMonth(size_t day, size_t month, size_t year) noexcept {
    return day == daysInMonth(month, year);
}

inline bool isLastMonthInYear(size_t, size_t month, size_t) noexcept {
    return month == 12;
}

inline stDate addUnits(stDate date, size_t days = 1, size_t months = 0, size_t years = 0,
                       size_t decades = 0, size_t centuries = 0, size_t millenniums = 0) {
    if (millenniums != 0) date.year += (millenniums * 1000);
    if (centuries != 0)   date.year += (centuries * 100);
    if (decades != 0)     date.year += (decades * 10);
    if (years != 0)       date.year += years;

    if (months != 0) {
        date.month += months;
        while (date.month > 12) {
            date.month -= 12;
            date.year++;
        }
    }

    size_t maxDays = daysInMonth(date.month, date.year);
    if (date.day > maxDays) {
        date.day = maxDays;
    }

    if (days != 0) {
        date.day += days;
        size_t tempDays;
        while (date.day > (tempDays = daysInMonth(date.month, date.year))) {
            date.day -= tempDays;
            date.month++;
            while (date.month > 12) {
                date.month -= 12;
                date.year++;
            }
        }
    }
    return date;
}

inline stDate subUnits(stDate date, size_t days = 1, size_t months = 0, size_t years = 0,
                       size_t decades = 0, size_t centuries = 0, size_t millenniums = 0) {
    if (millenniums != 0) date.year -= (millenniums * 1000);
    if (centuries != 0)   date.year -= (centuries * 100);
    if (decades != 0)     date.year -= (decades * 10);
    if (years != 0)       date.year -= years;

    if (months != 0) {
        long long tempMonth = static_cast<long long>(date.month) - months;
        while (tempMonth < 1) {
            tempMonth += 12;
            date.year--;
        }
        date.month = static_cast<size_t>(tempMonth);
    }

    size_t maxDays = daysInMonth(date.month, date.year);
    if (date.day > maxDays) {
        date.day = maxDays;
    }

    if (days != 0) {
        long long tempDay = static_cast<long long>(date.day) - days;
        while (tempDay < 1) {
            date.month--;
            while (date.month < 1) {
                date.month += 12;
                date.year--;
            }
            tempDay += daysInMonth(date.month, date.year);
        }
        date.day = static_cast<size_t>(tempDay);
    }
    return date;
}

// Operators implementation for stDate
inline stDate stDate::operator+(long long days) const {
    if (days >= 0) return addUnits(*this, static_cast<size_t>(days));
    return subUnits(*this, static_cast<size_t>(-days));
}

inline stDate stDate::operator-(long long days) const {
    if (days >= 0) return subUnits(*this, static_cast<size_t>(days));
    return addUnits(*this, static_cast<size_t>(-days));
}

inline stDate &stDate::operator+=(long long days) {
    *this = *this + days;
    return *this;
}

inline stDate &stDate::operator-=(long long days) {
    *this = *this - days;
    return *this;
}

inline stDate &stDate::operator++() {
    return *this += 1;
}

inline stDate stDate::operator++(int) {
    stDate temp = *this;
    ++(*this);
    return temp;
}

inline stDate &stDate::operator--() {
    return *this -= 1;
}

inline stDate stDate::operator--(int) {
    stDate temp = *this;
    --(*this);
    return temp;
}

inline size_t daysFromYearOne(const stDate &date, bool includeLastDay = false) noexcept {
    size_t total = 0;
    for (size_t y = 1; y < date.year; ++y) {
        total += daysInYear(y);
    }
    for (size_t m = 1; m < date.month; ++m) {
        total += daysInMonth(m, date.year);
    }
    total += date.day;
    if (!includeLastDay && total > 0) {
        total--;
    }
    return total;
}

inline long long daysBetween(const stDate &date1, const stDate &date2,
                             bool includeLastDay = false,
                             bool absoluteDiff = false) noexcept {
    long long d1 = static_cast<long long>(daysFromYearOne(date1));
    long long d2 = static_cast<long long>(daysFromYearOne(date2));
    long long diff = absoluteDiff ? (d2 >= d1 ? d2 - d1 : d1 - d2) : (d1 - d2);
    return includeLastDay ? ((diff >= 0) ? diff + 1 : diff - 1) : diff;
}

inline long long stDate::operator-(const stDate &other) const {
    return daysBetween(*this, other, false, false);
}

inline long long daysBetween(const stPeriod &period, bool includeLastDay = false,
                             bool absoluteDiff = false) noexcept {
    return daysBetween(period.dateFrom, period.dateTo, includeLastDay, absoluteDiff);
}

inline stDate getSystemDate() {
    time_t t = std::time(nullptr);
    std::tm *ltm = std::localtime(&t);
    return stDate{
        static_cast<size_t>(ltm->tm_mday),
        static_cast<size_t>(ltm->tm_mon + 1),
        static_cast<size_t>(ltm->tm_year + 1900)
    };
}

inline size_t ageInDays(const stDate &birthDate) {
    return daysFromYearOne(getSystemDate(), true) - daysFromYearOne(birthDate);
}

inline size_t calculateVacationDays(stDate dateFrom, stDate dateTo, bool includeWeekends = false) {
    size_t vacationDays = 0;
    while (dateFrom < dateTo) {
        if (includeWeekends || isBusinessDay(dateFrom)) {
            ++vacationDays;
        }
        dateFrom = addUnits(dateFrom, 1);
    }
    return vacationDays;
}

inline stDate CalculateVacationReturnDate(stDate date, long long vacationDays, bool includeWeekends = false) {
    stDate result = date;
    while (vacationDays > 0) {
        if (includeWeekends || isBusinessDay(result)) {
            --vacationDays;
        }
        result = addUnits(result, 1);
    }
    if (!includeWeekends) {
        while (!isBusinessDay(result)) {
            result = addUnits(result, 1);
        }
    }
    return result;
}

inline enumeration::enDateCompare compareDate(const stDate &d1, const stDate &d2) noexcept {
    if (d1 > d2) return enumeration::Date1_After;
    if (d1 < d2) return enumeration::Date1_Before;
    return enumeration::Date1_Equal;
}

inline bool isOverlap(const stDate &date1From, const stDate &date1To,
                      const stDate &date2From, const stDate &date2To) noexcept {
    return !(date1To < date2From || date2To < date1From);
}

inline bool isOverlap(const stPeriod &period1, const stPeriod &period2) noexcept {
    return isOverlap(period1.dateFrom, period1.dateTo, period2.dateFrom, period2.dateTo);
}

inline long long calculatePeriod(const stDate &date1, const stDate &date2, bool includeLastDay = false) noexcept {
    return daysBetween(date1, date2, includeLastDay, true);
}

inline long long calculatePeriod(const stPeriod &period, bool includeLastDay = false) noexcept {
    return calculatePeriod(period.dateFrom, period.dateTo, includeLastDay);
}

inline bool isDateInPeriod(const stDate &date, const stDate &dateStart, const stDate &dateEnd) noexcept {
    return !(date < dateStart || date > dateEnd);
}

inline bool isDateInPeriod(const stDate &date, const stPeriod &period) noexcept {
    return isDateInPeriod(date, period.dateFrom, period.dateTo);
}

/**
 * @brief Calculates the number of overlapping days between two periods in O(1) time.
 */
inline std::size_t countOverlapDays(const stPeriod &period1, const stPeriod &period2) noexcept {
    if (!isOverlap(period1, period2)) {
        return 0;
    }
    stDate overlapStart = (period1.dateFrom > period2.dateFrom) ? period1.dateFrom : period2.dateFrom;
    stDate overlapEnd = (period1.dateTo < period2.dateTo) ? period1.dateTo : period2.dateTo;

    if (overlapStart > overlapEnd) {
        return 0;
    }
    return static_cast<std::size_t>(daysBetween(overlapStart, overlapEnd, false, true));
}

inline bool isValidDate(const stDate &date) noexcept {
    return (date.month >= 1 && date.month <= 12) &&
           (date.day >= 1 && date.day <= daysInMonth(date.month, date.year));
}

inline stDate convertToDate(std::string_view dateString) {
    std::vector<std::string> v = split(dateString, '/');
    if (v.size() != 3) {
        return stDate{};
    }
    try {
        stDate d;
        d.day = static_cast<std::size_t>(std::stoul(v[0]));
        d.month = static_cast<std::size_t>(std::stoul(v[1]));
        d.year = static_cast<std::size_t>(std::stoul(v[2]));
        return d;
    } catch (...) {
        return stDate{};
    }
}

inline std::string dateToString(const stDate &date,
                                enumeration::enDateFormat format = enumeration::enDateFormat::basicFormat) {
    switch (format) {
    case enumeration::DD_MM_YYYYWithForwaredSlash:
        return std::to_string(date.day) + "/" + std::to_string(date.month) + "/" + std::to_string(date.year);
    case enumeration::YYYY_MM_DDWithForwaredSlash:
        return std::to_string(date.year) + "/" + std::to_string(date.month) + "/" + std::to_string(date.day);
    case enumeration::MM_DD_YYYYWithForwaredSlash:
        return std::to_string(date.month) + "/" + std::to_string(date.day) + "/" + std::to_string(date.year);
    case enumeration::DD_MM_YYYYWithHyphen:
        return std::to_string(date.day) + "-" + std::to_string(date.month) + "-" + std::to_string(date.year);
    case enumeration::YYYY_MM_DDWithHyphen:
        return std::to_string(date.year) + "-" + std::to_string(date.month) + "-" + std::to_string(date.day);
    case enumeration::MM_DD_YYYYWithHyphen:
        return std::to_string(date.month) + "-" + std::to_string(date.day) + "-" + std::to_string(date.year);
    case enumeration::basicFormat:
        return "Day: " + std::to_string(date.day) +
               ", Month: " + std::to_string(date.month) +
               ", Year: " + std::to_string(date.year);
    }
    return "";
}

inline std::string dateToString(const stDate &date, std::string_view format) {
    std::string result(format);
    result = replaceAll(result, "DD", std::to_string(date.day));
    result = replaceAll(result, "MM", std::to_string(date.month));
    result = replaceAll(result, "YYYY", std::to_string(date.year));
    return result;
}

} // namespace process
