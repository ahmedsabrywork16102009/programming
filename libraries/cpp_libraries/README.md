# Ahmed Sabry C++ Libraries

These are **header-only** libraries organized by responsibility. Include only the header you need and call functions through their namespace, such as `bank::` or `storage::`, instead of using `using namespace`.

## Folder Structure

```text
programming/
└── libraries/
    └── cpp_libraries/
        ├── README.md
        ├── as.h
        ├── enumeration.h
        ├── process.h
        ├── core/
        │   ├── math.h
        │   ├── string.h
        │   └── datetime.h
        ├── io/
        │   ├── input.h
        │   └── output.h
        ├── storage/
        │   └── file_utils.h
        └── domain/
            ├── bank.h
            └── clients.h
```

## Library Location

The actual library root is:

```text
programming/libraries/cpp_libraries/
```

The `libraries` folder is not inside the bank project. The following path is incorrect:

```text
projects/cpp_projects/bank/libraries/cpp_libraries/
```

From the bank project folder, use this relative path:

```text
../../../libraries/cpp_libraries/
```

Add `cpp_libraries` itself to the compiler and IntelliSense include paths:

```cpp
#include "domain/bank.h"
```

## Root Files and Modules

### `as.h` - Library Aggregator

Includes the modules and provides aliases such as `AS::bank::` and `AS::storage::`.

### `enumeration.h` - Enumerations

Defines general enums for numbers, characters, months, weekdays, operations, and date formats.

Namespace: `enumeration::`

### `process.h` - Compatibility Interface

Combines `enumeration.h` and `core` tools for older projects.

Namespace: `process::`

### Input and Output

- `io/input.h`, namespace `input::`
- `io/output.h`, namespace `output::`

### `domain/clients.h` - Clients

Contains the client implementation in the `clients::` namespace.

## Core Utilities

### `core/math.h`

General arithmetic operations and number checks.

### `core/string.h`

Text splitting, replacement, and whitespace removal.

Example: `process::splitByString(...)`

### `core/datetime.h`

Date and period types, calendar calculations, and date operations.

## Input and Output

`io/input.h` reads and validates strings, numbers, choices, and dates.

`io/output.h` prints strings, tables, and dates and manages screen reset.

## Storage

`storage/file_utils.h` provides common text-file operations. It knows only text lines and file paths.

### Checked Operations

- `storage::loadLinesFromFileChecked(...)`
- `storage::writeLinesToFileChecked(...)`
- `storage::appendLineToFileChecked(...)`
- `storage::stFileResult`
- `storage::reportFileError(...)`

Example:

```cpp
#include "storage/file_utils.h"

std::vector<std::string> vLines;
const storage::stFileResult fileResult =
    storage::loadLinesFromFileChecked("files/Clients.txt", vLines);

if (!fileResult.succeeded) {
    storage::reportFileError(fileResult);
}
```

Legacy file functions remain available for compatibility.

## Domain Data

### `domain/clients.h`

Contains customer data types and operations and uses `storage` for loading and saving.

Namespace: `clients::`

### `domain/bank.h`

Contains bank client types, searches, balance operations, and CRUD operations shared by bank and ATM projects.

Namespace: `bank::`

Example:

```cpp
#include "domain/bank.h"

std::vector<bank::stClientData> vClients =
    bank::loadClientDataFromFile("files/Clients.txt");
```

## Dependency Direction

```text
process.h
  ├── enumeration.h
  └── core/{math,string,datetime}.h

io/input.h
  ├── core/datetime.h
  ├── enumeration.h
  └── io/output.h

io/output.h
  ├── core/{datetime,math}.h
  └── enumeration.h

storage/file_utils.h
  └── Independent file and text-line operations

domain/clients.h
  ├── core/string.h
  ├── io/{input,output}.h
  └── storage/file_utils.h

domain/bank.h
  ├── io/input.h
  ├── io/output.h
  └── storage/file_utils.h
```

## Where to Add New Code

- General algorithms: `core/`
- Terminal interaction: `io/`
- General file operations: `storage/`
- Customer or bank rules: `domain/`
- Existing module functionality: place it in that module's directory

## Usage

Include only the module you need:

```cpp
#include "domain/bank.h"
#include "storage/file_utils.h"
```

Or use the optional aggregator:

```cpp
#include "as.h"

auto vClients = AS::bank::loadClientDataFromFile("files/Clients.txt");
auto today = AS::process::getSystemDate();
AS::output::printDateDetails(today, "Today is: ");
```

Prefer library prefixes instead of `using namespace`.
