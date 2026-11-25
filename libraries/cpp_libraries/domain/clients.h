#pragma once

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "../core/string.h"
#include "../enumeration.h"
#include "../io/input.h"
#include "../io/output.h"
#include "../storage/file_utils.h"

namespace clients {

	// =============================================================
	//  Structs
	// =============================================================

	struct stClientData {
		std::string accountNumber;
		std::string pinCode;
		std::string fullName;
		std::string phone;
		double accountBalance = 0.0;
	};

	// =============================================================
	//  Declarations
	// =============================================================

	stClientData readNewClient();
	void updateClientData(stClientData &clientData);

	std::string convertStructToLine(const stClientData &clientData, std::string_view separator = "#//#");

	stClientData convertLineToRecord(std::string_view text, std::string_view delimiter = "#//#");
	bool tryConvertLineToRecord(std::string_view text, stClientData &clientData, std::string_view delimiter = "#//#");

	void addLineDataToFile(std::string_view fileName, std::string_view lineData);
	void addClientsToFile(std::string_view fileName);

	std::vector<stClientData> loadDataFromFile(std::string_view fileName);
	void saveDataToFile(std::string_view fileName, const std::vector<stClientData> &vClients);

	void printAllClientsInTableView(const std::vector<stClientData> &vClients);
	int getIndexOfClientByAccountNumber(const std::vector<stClientData> &vClients, std::string_view accountNumber);
	void printClientDataVertical(const std::vector<stClientData> &vClients, int index);

	bool deleteClientData(std::vector<stClientData> &vClients, std::string_view accountNumber);
	bool editClientData(std::vector<stClientData> &vClients, std::string_view accountNumber);

	// =============================================================
	//  Implementations
	// =============================================================

	inline stClientData readNewClient() {
		stClientData clientData;
		clientData.accountNumber = input::readString("Enter Account Number: ");
		clientData.pinCode = input::readString("Enter Pin Code: ");
		clientData.fullName = input::readString("Enter Full Name: ");
		clientData.phone = input::readString("Enter Phone: ");
		clientData.accountBalance = input::readNumber<double>("Enter Account Balance: ");
		return clientData;
	}

	inline void updateClientData(stClientData &clientData) {
		if (input::readBool("Do you want to update account number? ")) clientData.accountNumber = input::readString("Enter Account Number: ");
		if (input::readBool("Do you want to update pin code? ")) clientData.pinCode = input::readString("Enter Pin Code: ");
		if (input::readBool("Do you want to update full name? ")) clientData.fullName = input::readString("Enter Full Name: ");
		if (input::readBool("Do you want to update phone? ")) clientData.phone = input::readString("Enter Phone: ");
		if (input::readBool("Do you want to update account balance? ")) clientData.accountBalance = input::readNumber<double>("Enter Account Balance: ");
	}

	inline std::string convertStructToLine(const stClientData &clientData, std::string_view separator) {
		std::string line = "";
		line.append(clientData.accountNumber).append(separator);
		line.append(clientData.pinCode).append(separator);
		line.append(clientData.fullName).append(separator);
		line.append(clientData.phone).append(separator);
		line.append(std::to_string(clientData.accountBalance));
		return line;
	}

	inline stClientData convertLineToRecord(std::string_view text, std::string_view delimiter) {
		stClientData clientData{};
		if (!tryConvertLineToRecord(text, clientData, delimiter)) {
			std::cerr << "Invalid client record data.\n";
		}
		return clientData;
	}

	inline bool tryConvertLineToRecord(std::string_view text, stClientData &clientData, std::string_view delimiter) {
		const std::vector<std::string> vTokens = process::splitByString(text, delimiter);
		if (vTokens.size() != 5) {
			return false;
		}

		double accountBalance = 0.0;
		try {
			std::size_t processedCharacters = 0;
			accountBalance = std::stod(vTokens[4], &processedCharacters);
			if (processedCharacters != vTokens[4].size()) {
				return false;
			}
		} catch (const std::invalid_argument &) {
			return false;
		} catch (const std::out_of_range &) {
			return false;
		}

		clientData.accountNumber = vTokens[0];
		clientData.pinCode = vTokens[1];
		clientData.fullName = vTokens[2];
		clientData.phone = vTokens[3];
		clientData.accountBalance = accountBalance;
		return true;
	}

	inline void addLineDataToFile(std::string_view fileName, std::string_view lineData) { storage::reportFileError(storage::appendLineToFileChecked(fileName, lineData)); }

	inline void addClientsToFile(std::string_view fileName) {
		bool isAddNewClient = true;
		while (isAddNewClient) {
			addLineDataToFile(fileName, convertStructToLine(readNewClient()));
			isAddNewClient = input::readBool("Do you want to add another client? [0]False, [1]True : ");
		}
	}

	inline std::vector<stClientData> loadDataFromFile(std::string_view fileName) {
		std::vector<stClientData> vClients;
		std::vector<std::string> vLines;
		const storage::stFileResult fileResult = storage::loadLinesFromFileChecked(fileName, vLines, true);
		if (!fileResult.succeeded) {
			storage::reportFileError(fileResult);
			return vClients;
		}

		vClients.reserve(vLines.size());
		for (std::size_t i = 0; i < vLines.size(); ++i) {
			if (vLines[i].empty()) {
				continue;
			}

			stClientData clientData{};
			if (tryConvertLineToRecord(vLines[i], clientData)) {
				vClients.push_back(clientData);
			} else {
				std::cerr << "Invalid client record in " << fileName << " at line " << i + 1 << ".\n";
			}
		}
		return vClients;
	}

	inline void saveDataToFile(std::string_view fileName, const std::vector<stClientData> &vClients) {
		std::vector<std::string> vLines;
		vLines.reserve(vClients.size());
		for (const auto &c : vClients) {
			vLines.push_back(convertStructToLine(c));
		}
		storage::reportFileError(storage::writeLinesToFileChecked(fileName, vLines));
	}

	inline void printAllClientsInTableView(const std::vector<stClientData> &vClients) {
		std::cout << "\n" << std::setw(45) << "Number of Clients: " << vClients.size() << "\n";
		std::cout << "------------------------------------------------------------------\n";
		std::cout << std::left << std::setw(10) << "Account Number" << " | " << std::setw(10) << "Pin Code" << " | " << std::setw(20) << "Full Name" << " | " << std::setw(15) << "Phone" << " | " << std::setw(10) << "Balance" << "\n";
		std::cout << "------------------------------------------------------------------\n";
		for (const auto &c : vClients) {
			std::cout << std::left << std::setw(10) << c.accountNumber << " | " << std::setw(10) << c.pinCode << " | " << std::setw(20) << c.fullName << " | " << std::setw(15) << c.phone << " | " << std::setw(10) << c.accountBalance << "\n";
		}
		std::cout << "------------------------------------------------------------------\n";
	}

	inline int getIndexOfClientByAccountNumber(const std::vector<stClientData> &vClients, std::string_view accountNumber) {
		for (size_t i = 0; i < vClients.size(); i++) {
			if (vClients[i].accountNumber == accountNumber) return static_cast<int>(i);
		}
		return -1;
	}

	inline void printClientDataVertical(const std::vector<stClientData> &vClients, int index) {
		if (index >= 0 && index < static_cast<int>(vClients.size())) {
			std::cout << "Account Number  : " << vClients[index].accountNumber << "\n";
			std::cout << "Pin Code        : " << vClients[index].pinCode << "\n";
			std::cout << "Full Name       : " << vClients[index].fullName << "\n";
			std::cout << "Phone Number    : " << vClients[index].phone << "\n";
			std::cout << "Account Balance : " << vClients[index].accountBalance << "\n";
		}
	}

	inline bool deleteClientData(std::vector<stClientData> &vClients, std::string_view accountNumber) {
		int idx = getIndexOfClientByAccountNumber(vClients, accountNumber);
		if (idx == -1) return false;
		vClients.erase(vClients.begin() + idx);
		return true;
	}

	inline bool editClientData(std::vector<stClientData> &vClients, std::string_view accountNumber) {
		int idx = getIndexOfClientByAccountNumber(vClients, accountNumber);
		if (idx == -1) return false;
		updateClientData(vClients[idx]);
		return true;
	}

}  // namespace clients
