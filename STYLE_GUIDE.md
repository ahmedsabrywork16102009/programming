# Ahmed Sabry (AS) Code Style Guide

This guide defines the standard for organizing code, libraries, and projects in this workspace.

## Workspace Structure

```text
programming/
├── libraries/
│   └── cpp_libraries/
│       ├── core/
│       ├── io/
│       ├── storage/
│       ├── domain/
│       ├── as.h
│       ├── enumeration.h
│       └── process.h
├── projects/
│   ├── cpp_projects/
│   ├── python_projects/
│   ├── chrome_extintion/
│   └── web_projects/
├── learn/
├── STYLE_GUIDE.md
└── AGENTS.md
```

## Naming Conventions

| Type | Rule | Prefix | Examples |
| :--- | :--- | :--- | :--- |
| Struct | PascalCase | `st` | `stDate`, `stPeriod`, `stClientData` |
| Enum | PascalCase | `en` | `enWeekday`, `enMonth`, `enChoice` |
| Vector | camelCase | `v` | `vStrings`, `vTokens`, `vChars` |
| Enum Value | PascalCase or `e` | `e` | `Odd`, `Even`, `eDeposit` |
| Function | camelCase | None | `readString`, `checkNumberType` |
| Variable | camelCase | None | `numberOfRounds`, `accountBalance` |
| Constant | UPPER_SNAKE_CASE | None | `ROCK_DRAWING`, `PROCESS_H` |
| Namespace | lowercase | None | `process`, `input`, `output` |
| Master Namespace | uppercase | None | `namespace AS` |

## C++ Library Design

1. Use header-only `.h` files.
2. Use `#pragma once` at the start of every header.
3. Place declarations before implementations.
4. Use visual section banners.
5. Use `inline` for header-only implementations.
6. Export public symbols into `namespace AS`.
7. Preserve backward compatibility with existing projects.

## Modern C++ Standards

- Use C++20 or C++23 where supported.
- Use `std::string_view` for read-only parameters.
- Use `std::mt19937` instead of `rand()`.
- Use `reserve()` for string and vector aggregation.
- Use `try/catch` and validation guards.
- Never add `using namespace` at global scope inside a header.
