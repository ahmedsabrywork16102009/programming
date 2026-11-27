#pragma once

#include <charconv>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "../io/input.h"
#include "../io/output.h"
#include "../process.h"
#include "../storage/file_utils.h"

namespace bank {

	// =============================================================
	//  Shared Bank Data Types
	// =============================================================
	struct stClientData {
		std::string accountNumber;
		std::string pinCode;
		std::string fullName;
		std::string phone;
		double accountBalance;
	};

	struct stUser {
		std::string userName;
		std::string password;
		short permission;
	};

	// =============================================================
	//  Shared Conversion Helpers
	// =============================================================
	inline bool tryParseNumber(std::string_view text, double &value) {
		const auto first = text.find_first_not_of(" \t\n\r");
		if (first == std::string_view::npos) {
			return false;
		}
		const auto last = text.find_last_not_of(" \t\n\r");
		text = text.substr(first, last - first + 1);

		const char *begin = text.data();
		const char *end = begin + text.size();
		auto [ptr, ec] = std::from_chars(begin, end, value);
		return ec == std::errc{} && ptr == end;
	}

	bool nearlyEqual(double a, double b, double eps = 1e-9) { return std::fabs(a - b) <= eps; }

	inline bool tryParseNumber(std::string_view text, short &value) {
		try {
			std::size_t processedCharacters = 0;
			const int parsedValue = std::stoi(std::string(text), &processedCharacters);
			if (processedCharacters != text.size() || parsedValue < std::numeric_limits<short>::min() || parsedValue > std::numeric_limits<short>::max()) {
				return false;
			}
			value = static_cast<short>(parsedValue);
			return true;
		} catch (const std::invalid_argument &) {
			return false;
		} catch (const std::out_of_range &) {
			return false;
		}
	}

	template <typename T>
	bool convertLineToRecord(std::string_view text, T &record, std::string_view delimiter = "#//#") {
		const std::vector<std::string> vTokens = process::splitByString(std::string(text), delimiter);

		if constexpr (std::is_same_v<T, stClientData>) {
			if (vTokens.size() != 5) {
				return false;
			}

			double accountBalance = 0.0;
			if (!tryParseNumber(vTokens[4], accountBalance)) {
				return false;
			}

			record.accountNumber = vTokens[0];
			record.pinCode = vTokens[1];
			record.fullName = vTokens[2];
			record.phone = vTokens[3];
			record.accountBalance = accountBalance;
			return true;
		} else if constexpr (std::is_same_v<T, stUser>) {
			if (vTokens.size() != 3) {
				return false;
			}

			short permission = 0;
			if (!tryParseNumber(vTokens[2], permission)) {
				return false;
			}

			record.userName = vTokens[0];
			record.password = vTokens[1];
			record.permission = permission;
			return true;
		} else {
			return false;
		}
	}

	template <typename T>
	T convertLineToRecord(std::string_view text, std::string_view delimiter = "#//#") {
		T record{};
		convertLineToRecord(text, record, delimiter);
		return record;
	}

	inline stClientData convertClientLineToRecord(std::string_view text) { return convertLineToRecord<stClientData>(text); }

	inline stUser convertUserLineToRecord(std::string_view text) { return convertLineToRecord<stUser>(text); }

	inline stClientData convertClientLineDataToRecord(std::string_view text) { return convertLineToRecord<stClientData>(text); }

	template <typename T>
	std::string convertRecordToLine(const T &record, std::string_view separator = "#//#") {
		std::string line;

		if constexpr (std::is_same_v<T, stClientData>) {
			line.append(record.accountNumber).append(separator);
			line.append(record.pinCode).append(separator);
			line.append(record.fullName).append(separator);
			line.append(record.phone).append(separator);
			line.append(std::to_string(record.accountBalance));
		} else if constexpr (std::is_same_v<T, stUser>) {
			line.append(record.userName).append(separator);
			line.append(record.password).append(separator);
			line.append(std::to_string(record.permission));
		}

		return line;
	}

	inline std::string convertClientRecordToLine(const stClientData &clientData, std::string_view separator = "#//#") { return convertRecordToLine(clientData, separator); }

	inline std::string convertUserRecordToLine(const stUser &user, std::string_view separator = "#//#") { return convertRecordToLine<stUser>(user, separator); }

	// =============================================================
	//  Shared File Helpers
	// =============================================================
	template <typename T>
	std::vector<T> loadDataFromFile(std::string_view fileName) {
		std::vector<T> vRecords;
		std::vector<std::string> vLines;
		const storage::stFileResult fileResult = storage::loadLinesFromFileChecked(fileName, vLines, true);
		if (!fileResult.succeeded) {
			storage::reportFileError(fileResult);
			return vRecords;
		}

		for (std::size_t i = 0; i < vLines.size(); ++i) {
			const std::string &lineData = vLines[i];
			if (lineData.empty()) {
				continue;
			}

			T record;
			if (convertLineToRecord(lineData, record)) {
				vRecords.push_back(record);
			} else {
				std::cerr << "Invalid record in " << fileName << " at line " << i + 1 << ".\n";
			}
		}

		return vRecords;
	}

	inline std::vector<stClientData> loadClientDataFromFile(std::string_view fileName) { return loadDataFromFile<stClientData>(fileName); }

	inline std::vector<stUser> loadUsersDataFromFile(std::string_view fileName) { return loadDataFromFile<stUser>(fileName); }

	template <typename T>
	void saveDataToFile(std::string_view fileName, const std::vector<T> &vRecords) {
		std::vector<std::string> vLines;
		vLines.reserve(vRecords.size());
		for (const T &record : vRecords) {
			vLines.push_back(convertRecordToLine(record));
		}

		const storage::stFileResult fileResult = storage::writeLinesToFileChecked(fileName, vLines);
		storage::reportFileError(fileResult);
	}

	// =============================================================
	//  Shared Record Lookup Helpers
	// =============================================================
	template <typename T>
	std::string getRecordId(const T &record) {
		if constexpr (std::is_same_v<T, stClientData>) {
			return record.accountNumber;
		} else if constexpr (std::is_same_v<T, stUser>) {
			return record.userName;
		} else {
			return {};
		}
	}

	template <typename T>
	int getIndexById(const std::vector<T> &vRecords, std::string_view id) {
		for (std::size_t i = 0; i < vRecords.size(); ++i) {
			if (getRecordId(vRecords[i]) == id) {
				return static_cast<int>(i);
			}
		}

		return -1;
	}

	template <typename T>
	bool isRecordFound(const std::vector<T> &vRecords, std::string_view id) {
		return getIndexById(vRecords, id) != -1;
	}

	inline int getIndexOfClientByAccountNumber(const std::vector<stClientData> &vClients, std::string_view accountNumber) { return getIndexById(vClients, accountNumber); }

	inline int getIndexOfUserByUsername(const std::vector<stUser> &vUsers, std::string_view username) { return getIndexById(vUsers, username); }

	inline bool isClientFound(const std::vector<stClientData> &vClients, std::string_view accountNumber) { return isRecordFound(vClients, accountNumber); }

	inline bool isUserFound(const std::vector<stUser> &vUsers, std::string_view username) { return isRecordFound(vUsers, username); }

	// =============================================================
	//  Shared Balance Helpers
	// =============================================================
	inline void depositAmount(stClientData &clientData, double amount) { clientData.accountBalance += amount; }

	inline void deposit(std::vector<stClientData> &vClients, int indexOfClient, double amount) { depositAmount(vClients[indexOfClient], amount); }

	inline bool isClientHasEnoughBalance(const stClientData &clientData, double amount) { return clientData.accountBalance >= amount; }

	inline bool isClientHasEnoughBalance(const std::vector<stClientData> &vClients, int indexOfClient, double amount) { return isClientHasEnoughBalance(vClients[indexOfClient], amount); }

	inline void withdrawAmount(stClientData &clientData, double amount) {
		if (isClientHasEnoughBalance(clientData, amount)) {
			clientData.accountBalance -= amount;
		}
	}

	inline void withdraw(std::vector<stClientData> &vClients, int indexOfClient, double amount) { withdrawAmount(vClients[indexOfClient], amount); }

	inline double getTotalBalances(const std::vector<stClientData> &vClients) {
		double totalBalances = 0.0;

		for (const stClientData &clientData : vClients) {
			totalBalances += clientData.accountBalance;
		}

		return totalBalances;
	}

	inline void PrintBalanceScreen(const std::vector<stClientData> &vClients, int indexOfClient) { std::cout << "Account Balance : " << vClients[indexOfClient].accountBalance << "\n"; }

	inline void PrintBalanceScreen(const stClientData &clientData) { std::cout << "Account Balance : " << clientData.accountBalance << "\n"; }

	// =============================================================
	//  Shared Screens
	// =============================================================
	inline void printLoginScreenLogo() {
		std::cout << "====================================\n";
		std::cout << "           LogIn Screen\n";
		std::cout << "====================================\n";
	}

	// =============================================================
	//  Generic CRUD and Printing
	// =============================================================
	template <typename T>
	void printRecordDataVertical(const T &record) {
		if constexpr (std::is_same_v<T, stClientData>) {
			std::cout << "Account Number  : " << record.accountNumber << "\n";
			std::cout << "Pin Code        : " << record.pinCode << "\n";
			std::cout << "Full Name       : " << record.fullName << "\n";
			std::cout << "Phone Number    : " << record.phone << "\n";
			std::cout << "Account Balance : " << record.accountBalance << "\n";
		} else if constexpr (std::is_same_v<T, stUser>) {
			std::cout << "User Name       : " << record.userName << "\n";
			std::cout << "Password        : " << record.password << "\n";
			std::cout << "Permission      : " << record.permission << "\n";
		}
	}

	template <typename T>
	void printRecordDataVertical(const std::vector<T> &vRecords, int index) {
		printRecordDataVertical(vRecords[index]);
	}

	template <typename T>
	void printFindResult(const std::vector<T> &vRecords, int index, std::string_view recordName) {
		if (index != -1) {
			std::cout << "\n" << recordName << " Found:\n";
			printRecordDataVertical(vRecords[index]);
		} else {
			std::cout << "\n" << recordName << " Not Found!\n";
		}
	}

	inline void printFindResult(const std::vector<stClientData> &vClients, int index) { printFindResult(vClients, index, "Client"); }

	inline void printFindResult(const std::vector<stUser> &vUsers, int index) { printFindResult(vUsers, index, "User"); }

	inline void printClientDataVertical(const stClientData &clientData) { printRecordDataVertical(clientData); }

	inline void printUserDataVertical(const stUser &userData) { printRecordDataVertical(userData); }

	inline void printClientDataVertical(const std::vector<stClientData> &vClients, int index) { printRecordDataVertical(vClients, index); }

	inline void printUserDataVertical(const std::vector<stUser> &vUsers, int index) { printRecordDataVertical(vUsers, index); }

	inline void whatPermissionToGive(short &permission) {
		permission = 0;

		if (input::readBool("Do you want to give permission to Show client list? [1] Yes [0] No: ")) permission |= 1 << 0;
		if (input::readBool("Do you want to give permission to Add new client? [1] Yes [0] No: ")) permission |= 1 << 1;
		if (input::readBool("Do you want to give permission to Delete client? [1] Yes [0] No: ")) permission |= 1 << 2;
		if (input::readBool("Do you want to give permission to Update client? [1] Yes [0] No: ")) permission |= 1 << 3;
		if (input::readBool("Do you want to give permission to Find client? [1] Yes [0] No: ")) permission |= 1 << 4;
		if (input::readBool("Do you want to give permission to Transactions? [1] Yes [0] No: ")) permission |= 1 << 5;
		if (input::readBool("Do you want to give permission to Manage users? [1] Yes [0] No: ")) permission |= 1 << 6;
	}

	template <typename T>
	T readNewRecord(const std::vector<T> &vRecords) {
		T record{};
		bool isRepeated = false;

		do {
			if (isRepeated) {
				std::cout << "Record [" << getRecordId(record) << "] already exists, try again.\n";
			}
			isRepeated = true;

			if constexpr (std::is_same_v<T, stClientData>) {
				record.accountNumber = input::readString("Enter Account Number: ");
			} else if constexpr (std::is_same_v<T, stUser>) {
				record.userName = input::readString("Enter Username: ");
			}
		} while (isRecordFound(vRecords, getRecordId(record)));

		if constexpr (std::is_same_v<T, stClientData>) {
			record.pinCode = input::readString("Enter Pin Code: ");
			record.fullName = input::readString("Enter Full Name: ");
			record.phone = input::readString("Enter Phone: ");
			record.accountBalance = input::readNumber<double>("Enter Account Balance: ");
		} else if constexpr (std::is_same_v<T, stUser>) {
			record.password = input::readString("Enter Password: ");
			if (input::readBool("Do you want to give Full access permission? [1] Yes [0] No: ")) {
				record.permission = -1;
			} else {
				std::cout << "Select the permissions to grant this user:-\n";
				whatPermissionToGive(record.permission);
			}
		}

		return record;
	}

	template <typename T>
	bool updateRecord(T &record) {
		printRecordDataVertical(record);

		if (!input::readBool("Are you sure you want to update this record [1] Yes [0] No: ")) {
			return false;
		}

		if constexpr (std::is_same_v<T, stClientData>) {
			if (input::readBool("Do you want to update account number? ")) record.accountNumber = input::readString("Enter Account Number: ");
			if (input::readBool("Do you want to update pin code? ")) record.pinCode = input::readString("Enter Pin Code: ");
			if (input::readBool("Do you want to update full name? ")) record.fullName = input::readString("Enter Full Name: ");
			if (input::readBool("Do you want to update phone? ")) record.phone = input::readString("Enter Phone: ");
			if (input::readBool("Do you want to update account balance? ")) record.accountBalance = input::readNumber<double>("Enter Account Balance: ");
		} else if constexpr (std::is_same_v<T, stUser>) {
			if (input::readBool("Do you want to update username? ")) record.userName = input::readString("Enter Username: ");
			if (input::readBool("Do you want to update password? ")) record.password = input::readString("Enter Password: ");
			if (input::readBool("Do you want to update permission? ")) {
				if (input::readBool("Do you want to give Full access permission? [1] Yes [0] No: ")) {
					record.permission = -1;
				} else {
					std::cout << "Select the permissions to grant this user:-\n";
					whatPermissionToGive(record.permission);
				}
			}
		}

		return true;
	}

	template <typename T, typename TProtectedPredicate>
	bool deleteRecord(std::vector<T> &vRecords, std::string_view id, TProtectedPredicate isProtected, std::string_view recordName) {
		const int index = getIndexById(vRecords, id);
		if (index == -1) {
			return false;
		}

		printRecordDataVertical(vRecords[index]);

		if (isProtected(vRecords[index], index)) {
			std::cout << "Operation Cancelled!\n";
			return false;
		}

		if (input::readBool("Are you sure you want to delete this " + std::string(recordName) + " [1] Yes [0] No: ")) {
			vRecords.erase(vRecords.begin() + index);
			std::cout << recordName << " Deleted Successfully!\n";
			return true;
		}

		std::cout << "Operation Cancelled!\n";
		return false;
	}

	template <typename T>
	void addNewRecords(std::vector<T> &vRecords, std::string_view recordName) {
		do {
			vRecords.push_back(readNewRecord(vRecords));
		} while (input::readBool("Do you want to add another " + std::string(recordName) + " [1-Yes] [0-No]: "));
	}

	template <typename T>
	void updateRecordScreen(std::vector<T> &vRecords, std::string_view prompt, std::string_view recordName) {
		const int index = getIndexById(vRecords, input::readString(std::string(prompt)));

		if (index != -1) {
			updateRecord(vRecords[index]);
		} else {
			std::cout << recordName << " Not Found!\n";
		}

		output::resetScreen();
	}

	template <typename T, typename TProtectedPredicate>
	void deleteRecordScreen(std::vector<T> &vRecords, std::string_view prompt, std::string_view recordName, TProtectedPredicate isProtected) {
		const int index = getIndexById(vRecords, input::readString(std::string(prompt)));

		if (index != -1) {
			deleteRecord(vRecords, getRecordId(vRecords[index]), isProtected, recordName);
		} else {
			std::cout << recordName << " Not Found!\n";
		}

		output::resetScreen();
	}

	template <typename T>
	void findRecordScreen(const std::vector<T> &vRecords, std::string_view prompt, std::string_view recordName) {
		printFindResult(vRecords, getIndexById(vRecords, input::readString(std::string(prompt))), recordName);
	}

	template <typename T>
	void printRecordsInTableView(const std::vector<T> &vRecords) {
		if constexpr (std::is_same_v<T, stClientData>) {
			const std::string line(85, '-');
			std::cout << "\n" << std::setw(45) << "Number of Clients: " << vRecords.size() << "\n";
			std::cout << line << "\n";
			std::cout << "| " << std::left << std::setw(14) << "Account Number"
			          << " | " << std::setw(10) << "Pin Code"
			          << " | " << std::setw(20) << "Full Name"
			          << " | " << std::setw(15) << "Phone"
			          << " | " << std::setw(10) << "Balance" << " |\n";
			std::cout << line << "\n";

			for (const stClientData &record : vRecords) {
				std::cout << "| " << std::left << std::setw(14) << record.accountNumber << " | " << std::setw(10) << record.pinCode << " | " << std::setw(20) << record.fullName << " | " << std::setw(15) << record.phone << " | " << std::setw(10) << record.accountBalance << " |\n";
			}
			std::cout << line << "\n";
		} else if constexpr (std::is_same_v<T, stUser>) {
			const std::string line(52, '-');
			std::cout << "\n" << std::setw(45) << "Number of Users: " << vRecords.size() << "\n";
			std::cout << line << "\n";
			std::cout << "| " << std::left << std::setw(16) << "User Name" << " | " << std::setw(14) << "Password" << " | " << std::setw(12) << "Permission" << " |\n";
			std::cout << line << "\n";

			for (const stUser &record : vRecords) {
				std::cout << "| " << std::left << std::setw(16) << record.userName << " | " << std::setw(14) << record.password << " | " << std::setw(12) << record.permission << " |\n";
			}
			std::cout << line << "\n";
		}
	}

}  // namespace bank
