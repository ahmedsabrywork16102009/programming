# AGENTS.md - Workspace Rules for Ahmed Sabry's Programming Workspace

This file specifies project-specific guidelines, coding style, and architecture rules that Antigravity agents MUST strictly follow when working in this repository.

## 1. Developer Persona & Identity
- **Developer Name:** Ahmed Sabry (AS).
- **Primary Namespace:** `namespace AS` (which unifies `enumeration`, `process`, `input`, `output`, `clients`).

## 2. C++ Coding Style & Naming Conventions
Always write C++ code in Ahmed Sabry's signature style:
- **Structs:** MUST use `st` prefix in PascalCase: `stDate`, `stPeriod`, `stClientData`, `stRoundInfo`, `stGameInfo`.
- **Enums:** MUST use `en` prefix in PascalCase: `enNumber`, `enCharCase`, `enWeekday`, `enMonth`, `enChoice`.
- **Vectors:** MUST use `v` prefix in camelCase: `vStrings`, `vTokens`, `vChars`, `vDateString`.
- **Functions:** camelCase (e.g. `readString`, `checkNumberType`, `splitByString`, `daysBetween`).
- **Variables:** camelCase (e.g. `numberOfRounds`, `playerWinTimes`, `accountBalance`).
- **Constants:** ALL_CAPS / UPPER_SNAKE_CASE (e.g. `ROCK_DRAWING`).

## 3. Library Architecture (`libraries/cpp_libraries/`)
- All libraries are **Header-Only** (`.h`).
- Use `#pragma once` at the top of every header.
- Maintain visual section banners:
  ```cpp
  // ===========================================================================
  //  Section Title
  // ===========================================================================
  ```
- Clear declarations at top (`// --- Declarations ---`), implementations below with `inline`.
- Export symbols to `namespace AS`.
- Any refactoring must maintain **100% backward compatibility** with existing projects (`projects/cpp_projects/bank`, `projects/cpp_projects/math-game`, `projects/cpp_projects/rock-paper-scissor`).

## 4. Modern C++ with Ahmed's Style
- Write clean, modern, safe C++ (C++20/C++23) underneath while preserving Ahmed's naming conventions on the surface.
- Equip structs like `stDate` with operator overloads (`<=>`, `==`, `+`, `-`, `++`, `--`, `<<`).
- Use `std::string_view` for read-only parameters.
- Use `std::mt19937` instead of legacy `rand()`.
- Ensure optimal algorithms ($O(1)$ where applicable).
- No uncaught exceptions; use `try/catch` and validation guards.

