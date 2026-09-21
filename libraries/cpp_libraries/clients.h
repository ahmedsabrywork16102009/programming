#pragma once

#include "input.h"
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "enumeration.h"
#include "input.h"
#include "output.h"
#include "process.h"

namespace clients {
struct stClientData {
  std::string accountNumber;
  std::string pinCode;
  std::string fullName;
  std::string phone;
  double accountBalance;
};
stClientData readNewClient();
void updateClientData(stClientData &clientData);
std::string convertStructToLine(const stClientData &clientData,
                                std::string_view separator = "#//#");
stClientData convertLineToRecord(std::string text,
                                 std::string_view delimiter = "#//#");
void addLineDataToFile(const std::string &fileName,
                       const std::string &lineData);
void addClientsToFile(const std::string &fileName);
std::vector<stClientData> loadDataFromFile(const std::string &fileName);
void printAllClientsInTableView(const std::vector<stClientData> &vClients);
int getIndexOfClientByAccountNumber(const std::vector<stClientData> &vClients,
                                    const std::string &accountNumber);
void printClientDataVertical(const std::vector<stClientData> &vClients,
                             int index);
void saveDataToFile(const std::string &fileName,
                    const std::vector<stClientData> &clientData);
bool deleteClientData(std::vector<stClientData> &vClients,
                      const std::string &accountNumber);
bool editClientData(std::vector<stClientData> &vClients,
                    const std::string &accountNumber);

inline stClientData readNewClient() {
  stClientData clientData;
  clientData.accountNumber = input::readString("Enter Account Number: ");
  clientData.pinCode = input::readString("Enter Pin Code: ");
  clientData.fullName = input::readString("Enter Full Name: ");
  clientData.phone = input::readString("Enter Phone: ");
  clientData.accountBalance =
      input::readNumber<double>("Enter Account Balance: ");
  return clientData;
}

inline void updateClientData(stClientData &clientData) {
  if (input::readBool("Do you want to update account number? "))
    clientData.accountNumber = input::readString("Enter Account Number: ");
  if (input::readBool("Do you want to update pin code? "))
    clientData.pinCode = input::readString("Enter Pin Code: ");
  if (input::readBool("Do you want to update full name? "))
    clientData.fullName = input::readString("Enter Full Name: ");
  if (input::readBool("Do you want to update phone? "))
    clientData.phone = input::readString("Enter Phone: ");
  if (input::readBool("Do you want to update account balance? "))
    clientData.accountBalance =
        input::readNumber<double>("Enter Account Balance: ");
}

inline std::string convertStructToLine(const stClientData &clientData,
                                       std::string_view separator) {
  std::string line = "";
  line.append(clientData.accountNumber).append(separator);
  line.append(clientData.pinCode).append(separator);
  line.append(clientData.fullName).append(separator);
  line.append(clientData.phone).append(separator);
  line.append(std::to_string(clientData.accountBalance));
  return line;
}

inline stClientData convertLineToRecord(std::string text,
                                        std::string_view delimiter) {
  stClientData clientData;
  std::vector<std::string> vTokens = process::splitByString(text, delimiter);
  if (vTokens.size() >= 5) {
    clientData.accountNumber = vTokens[0];
    clientData.pinCode = vTokens[1];
    clientData.fullName = vTokens[2];
    clientData.phone = vTokens[3];
    clientData.accountBalance = stod(vTokens[4]);
  }
  return clientData;
}

inline void addLineDataToFile(const std::string &fileName,
                              const std::string &lineData) {
  std::fstream myFile;
  myFile.open(fileName, std::ios::out | std::ios::app);
  if (myFile.is_open()) {
    myFile << lineData << "\n";
    myFile.close();
  }
}

inline void addClientsToFile(const std::string &fileName) {
  bool isAddNewClient = true;
  while (isAddNewClient) {
    addLineDataToFile(fileName, convertStructToLine(readNewClient()));
    isAddNewClient = input::readBool(
        "Do you want to add another client? [0]False, [1]True : ");
  }
}

inline std::vector<stClientData> loadDataFromFile(const std::string &fileName) {
  std::vector<stClientData> vClients;
  std::fstream myFile;
  myFile.open(fileName, std::ios::in);
  if (myFile.is_open()) {
    std::string lineData;
    while (getline(myFile, lineData)) {
      if (!lineData.empty())
        vClients.push_back(convertLineToRecord(lineData));
    }
    myFile.close();
  }
  return vClients;
}

inline void
printAllClientsInTableView(const std::vector<stClientData> &vClients) {
  std::cout << "\n"
            << std::setw(45) << "Number of Clients: " << vClients.size()
            << "\n";
  std::cout
      << "------------------------------------------------------------------\n";
  std::cout << std::left << std::setw(10) << "Account Number" << " | "
            << std::setw(10) << "Pin Code"
            << " | " << std::setw(20) << "Full Name" << " | " << std::setw(15)
            << "Phone"
            << " | " << std::setw(10) << "Balance" << "\n";
  std::cout
      << "------------------------------------------------------------------\n";
  for (const auto &c : vClients) {
    std::cout << std::left << std::setw(10) << c.accountNumber << " | "
              << std::setw(10) << c.pinCode << " | " << std::setw(20)
              << c.fullName << " | " << std::setw(15) << c.phone << " | "
              << std::setw(10) << c.accountBalance << "\n";
  }
  std::cout
      << "------------------------------------------------------------------\n";
}

inline int
getIndexOfClientByAccountNumber(const std::vector<stClientData> &vClients,
                                const std::string &accountNumber) {
  for (size_t i = 0; i < vClients.size(); i++) {
    if (vClients[i].accountNumber == accountNumber)
      return static_cast<int>(i);
  }
  return -1;
}

inline void printClientDataVertical(const std::vector<stClientData> &vClients,
                                    int index) {
  if (index >= 0 && index < static_cast<int>(vClients.size())) {
    std::cout << "Account Number  : " << vClients[index].accountNumber << "\n";
    std::cout << "Pin Code        : " << vClients[index].pinCode << "\n";
    std::cout << "Full Name       : " << vClients[index].fullName << "\n";
    std::cout << "Phone Number    : " << vClients[index].phone << "\n";
    std::cout << "Account Balance : " << vClients[index].accountBalance << "\n";
  }
}

inline void saveDataToFile(const std::string &fileName,
                           const std::vector<stClientData> &clientData) {
  std::fstream myFile;
  myFile.open(fileName, std::ios::out);
  if (myFile.is_open()) {
    for (size_t i = 0; i < clientData.size(); i++)
      myFile << convertStructToLine(clientData[i]) << "\n";
    myFile.close();
  }
}

inline bool deleteClientData(std::vector<stClientData> &vClients,
                             const std::string &accountNumber) {
  int idx = getIndexOfClientByAccountNumber(vClients, accountNumber);
  if (idx == -1)
    return false;
  vClients.erase(vClients.begin() + idx);
  return true;
}

inline bool editClientData(std::vector<stClientData> &vClients,
                           const std::string &accountNumber) {
  int idx = getIndexOfClientByAccountNumber(vClients, accountNumber);
  if (idx == -1)
    return false;
  updateClientData(vClients[idx]);
  return true;
}

} // namespace clients

namespace AS {
using namespace clients;
}