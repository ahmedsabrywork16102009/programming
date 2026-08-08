#include "../../../libraries/enumeration.h"
#include "../../../libraries/input.h"
#include "../../../libraries/process.h"
#include "../../../libraries/output.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

using namespace std;
using namespace AS;

enum enChoice {
  eShowClientList = 1,
  eAddNewClient = 2,
  eDeleteClient = 3,
  eUpdateClient = 4,
  eFindClient = 5,
  eTransactions = 6,
  eExit = 7
};

enum enTransactions {
  eDeposit = 1,
  eWithdraw = 2,
  eTotalBalances = 3,
  eBackMainMenu = 4
};

struct stClientData {
  string accountNumber;
  string pinCode;
  string fullName;
  string phone;
  double accountBalance;
};

stClientData convertLineToRecord(string text, string_view delimiter = "#//#") {
  stClientData clientData;

  vector<string> vTokens = splitString(text, delimiter);

  clientData.accountNumber = vTokens[0];
  clientData.pinCode = vTokens[1];
  clientData.fullName = vTokens[2];
  clientData.phone = vTokens[3];
  clientData.accountBalance = stod(vTokens[4]);

  return clientData;
}

string convertRecordToLine(const stClientData &clientData,
                           string_view seperator = "#//#") {
  string line = "";

  line.append(clientData.accountNumber).append(seperator);
  line.append(clientData.pinCode).append(seperator);
  line.append(clientData.fullName).append(seperator);
  line.append(clientData.phone).append(seperator);
  line.append(to_string(clientData.accountBalance));

  return line;
}

vector<stClientData> loadDataFromFile(const string &fileName) {
  vector<stClientData> vClients;
  fstream myFile;

  myFile.open(fileName, ios::in);

  if (myFile.is_open()) {
    string lineData;

    while (getline(myFile, lineData)) {
      vClients.push_back(convertLineToRecord(lineData));
    }

    myFile.close();
  }

  return vClients;
}

void saveDataToFile(const string &fileName,
                    const vector<stClientData> &vClientData) {
  fstream myFile;
  myFile.open(fileName, ios::out);
  string lineData = "";

  if (myFile.is_open()) {
    for (const auto &clientData : vClientData) {
      lineData = convertRecordToLine(clientData);
      myFile << lineData << "\n";
    }

    myFile.close();
  }
}

int getIndexOfClientByAccountNumber(const vector<stClientData> &vClients,
                                    const string &accountNumber) {
  for (int i = 0; i < vClients.size(); i++) {
    if (vClients[i].accountNumber == accountNumber) {
      return i;
    }
  }

  return -1;
}

bool isClientFound(const vector<stClientData> &vClients,
                   const string &accountNumber) {
  return getIndexOfClientByAccountNumber(vClients, accountNumber) != -1;
}

void printClientDataVertical(const vector<stClientData> &vClients,
                             int indexOfClient) {
  cout << "Account Number  : " << vClients[indexOfClient].accountNumber << "\n";
  cout << "Pin Code        : " << vClients[indexOfClient].pinCode << "\n";
  cout << "Full Name       : " << vClients[indexOfClient].fullName << "\n";
  cout << "Phone Number    : " << vClients[indexOfClient].phone << "\n";
  cout << "Account Balance : " << vClients[indexOfClient].accountBalance
       << "\n";
}

void printClientDataVertical(const stClientData &clientData) {
  cout << "Account Number  : " << clientData.accountNumber << "\n";
  cout << "Pin Code        : " << clientData.pinCode << "\n";
  cout << "Full Name       : " << clientData.fullName << "\n";
  cout << "Phone Number    : " << clientData.phone << "\n";
  cout << "Account Balance : " << clientData.accountBalance << "\n";
}

void printFindResult(const vector<stClientData> &vClients, int index) {
  if (index != -1) {
    cout << "\nClient Found:\n";
    printClientDataVertical(vClients, index);
  } else {
    cout << "\nClient Not Found!\n";
  }
}

stClientData readNewClient(vector<stClientData> &vClients) {
  stClientData clientData;
  bool isRepeated = false;

  do {
    if (isRepeated) {
      cout << "Client with [" << clientData.accountNumber
           << "] is already exists, try again.\n";
    }
    isRepeated = true;

    clientData.accountNumber = readString("Enter Account Number: ");
  } while (isClientFound(vClients, clientData.accountNumber));

  clientData.pinCode = readString("Enter Pin Code: ");
  clientData.fullName = readString("Enter Full Name: ");
  clientData.phone = readString("Enter Phone: ");
  clientData.accountBalance = readNumber<double>("Enter Account Balance: ");

  return clientData;
}

bool updateClientData(stClientData &clientData) {
  printClientDataVertical(clientData);

  if (readBool(
          "Are you sure you want to update this client [1] Yes [0] No: ")) {
    if (readBool("Do you want to update account number? ")) {
      clientData.accountNumber = readString("Enter Account Number: ");
    }

    if (readBool("Do you want to update pin code? ")) {
      clientData.pinCode = readString("Enter Pin Code: ");
    }

    if (readBool("Do you want to update full name? ")) {
      clientData.fullName = readString("Enter Full Name: ");
    }

    if (readBool("Do you want to update phone? ")) {
      clientData.phone = readString("Enter Phone: ");
    }

    if (readBool("Do you want to update account balance? ")) {
      clientData.accountBalance = readNumber<double>("Enter Account Balance: ");
    }
    return true;
  }

  return false;
}

bool deleteClient(vector<stClientData> &vClients, const string &accountNumber) {
  int indexOfClient = getIndexOfClientByAccountNumber(vClients, accountNumber);

  if (indexOfClient == -1) {
    return false;
  }

  printClientDataVertical(vClients, indexOfClient);

  if (readBool(
          "Are you sure you want to delete this client [1] Yes [0] No: ")) {
    vClients.erase(vClients.begin() + indexOfClient);
    cout << "Client Deleted Successfully!\n";

    system("pause");
    return true;
  }
  cout << "Operation Cancelled!\n";

  system("pause");
  return false;
}

void printMainMenueScreen() {
  cout << "===========================================\n";
  cout << "              MAIN MENUE \n";
  cout << "===========================================\n";
  cout << "    1. Show Client List.\n";
  cout << "    2. Add New Client.\n";
  cout << "    3. Delete Client.\n";
  cout << "    4. Update Client.\n";
  cout << "    5. Find Client.\n";
  cout << "    6. Transactions.\n";
  cout << "    7. Exit.\n";
  cout << "===========================================\n";
}

void printClientsInTableView(const vector<stClientData> &vClients) {
  string_view line = "---------------------------------------------------------"
                     "----------------------------";
  cout << "\n" << setw(45) << "Number of Clients: " << vClients.size() << "\n";
  cout << line << endl;
  cout << "| " << left << setw(10) << "Account Number" << " | " << left
       << setw(10) << "Pin Code" << " | " << left << setw(20) << "Full Name"
       << " | " << left << setw(15) << "Phone" << " | " << left << setw(10)
       << "Balance" << " |\n";
  cout << line << endl;

  for (const auto &clientData : vClients) {
    cout << "| " << left << setw(14) << clientData.accountNumber << " | "
         << left << setw(10) << clientData.pinCode << " | " << left << setw(20)
         << clientData.fullName << " | " << left << setw(15) << clientData.phone
         << " | " << left << setw(10) << clientData.accountBalance << " |\n";
  }
  cout << line << "\n";

  system("pause");
}

void addNewClientScreen(vector<stClientData> &vClients) {
  do {
    vClients.push_back(readNewClient(vClients));
  } while (readBool("Do you want to add another client [1-Yes] [0-No]: "));

  system("pause");
}

void updateClientScreen(vector<stClientData> &vClients) {
  int indexOfClient = getIndexOfClientByAccountNumber(
      vClients, readString("Enter Account Number: "));

  if (indexOfClient != -1) {
    updateClientData(vClients[indexOfClient]);
  } else {
    cout << "Client Not Found!\n";
  }

  system("pause");
}

void deleteClientScreen(vector<stClientData> &vClients) {
  int indexOfClient = getIndexOfClientByAccountNumber(
      vClients, readString("Enter Account Number: "));

  if (indexOfClient != -1) {
    deleteClient(vClients, vClients[indexOfClient].accountNumber);
  } else {
    cout << "Client Not Found!\n";
  }

  system("pause");
}

void findClientScreen(vector<stClientData> &vClients) {
  printFindResult(vClients,
                  getIndexOfClientByAccountNumber(
                      vClients, readString("Enter Account Number: ")));

  system("pause");
}

void exitScreen() {
  cout << "\n\n\t\t\t\t\tThank you for using our system!\n\n\n";

  system("pause");
}

void printTransactionsMenueScreen() {
  cout << "=============================================\n";
  cout << "             TRANSACTIONS MENU \n";
  cout << "=============================================\n";
  cout << "   1. Deposit.\n";
  cout << "   2. Withdraw.\n";
  cout << "   3. Total Balance.\n";
  cout << "   4. Main Menue.\n";
  cout << "=============================================\n";
}

void deposit(vector<stClientData> &vClients, int indexOfClient,
             double depositValue) {
  vClients[indexOfClient].accountBalance += depositValue;
}

void depositScreen(vector<stClientData> &vClients) {
  int indexOfClient = getIndexOfClientByAccountNumber(
      vClients, readString("Enter Account Number: "));

  double depositValue = 0;
  bool isFirstTime = 1;

  if (indexOfClient != -1) {
    if (isFirstTime) {
      printClientDataVertical(vClients, indexOfClient);

      isFirstTime = 0;
    }

    depositValue = readNumber<double>("Enter deposit value: ", 0.0, std::numeric_limits<double>::max());

    if (readBool(
            "Are you sure you want to deposit this client [1] Yes [0] No: ")) {
      deposit(vClients, indexOfClient, depositValue);
      cout << "\nClient Deposit Successfully!\n";
    } else {
      cout << "Operation Cancelled!\n";
    }

  } else {
    cout << "Client Not Found!\n";
  }

  system("pause");
}

void withdraw(vector<stClientData> &vClients, int indexOfClient,
              double withdrawValue) {
  vClients[indexOfClient].accountBalance -= withdrawValue;
}

bool isClientHasEnoughBalance(vector<stClientData> &vClients, int indexOfClient,
                              double withdrawValue) {
  return vClients[indexOfClient].accountBalance >= withdrawValue;
}

void withdrawScreen(vector<stClientData> &vClients) {
  int indexOfClient = getIndexOfClientByAccountNumber(
      vClients, readString("Enter Account Number: "));

  double withdrawValue = 0;
  bool hasEnoughBalance = 0;
  bool isFirstTime = 1;

  if (indexOfClient != -1) {
    do {
      if (isFirstTime) {
        printClientDataVertical(vClients, indexOfClient);

        isFirstTime = 0;
      }

      withdrawValue = readNumber<double>("Enter withdraw value: ", 0.0, std::numeric_limits<double>::max());

      if ((hasEnoughBalance = isClientHasEnoughBalance(vClients, indexOfClient,
                                                       withdrawValue))) {
        withdraw(vClients, indexOfClient, withdrawValue);
        cout << "\nClient Withdraw Successfully!\n";
        break;
      } else {
        cout << "\nClient Doesn't Have Enough Balance!\n";
      }
    } while (!hasEnoughBalance);
  } else {
    cout << "Client Not Found!\n";
  }

  system("pause");
}

double getTotalBalances(const vector<stClientData> &vClients) {
  double totalBalances = 0.0;

  for (const auto &clientData : vClients) {
    totalBalances += clientData.accountBalance;
  }

  return totalBalances;
}

void totalBalancesScreen(vector<stClientData> &vClients) {
  string_view line =
      "-----------------------------------------------------------------";
  cout << "\n" << setw(45) << "Number of Clients: " << vClients.size() << "\n";
  cout << line << endl;
  cout << "| " << left << setw(10) << "Account Number" << " | " << left
       << setw(20) << "Full Name" << " | " << left << setw(10) << "Balance"
       << " |\n";
  cout << line << endl;

  for (const auto &clientData : vClients) {
    cout << "| " << left << setw(14) << clientData.accountNumber << " | "
         << left << setw(20) << clientData.fullName << left << setw(10)
         << clientData.accountBalance << " |\n";
  }
  cout << line << "\n";
  cout << "================================================================-\n";
  cout << "Total Balances: " << getTotalBalances(vClients) << " $\n";
  cout << "================================================================-\n";

  system("pause");
}

void mainMenuScreen() {
  cout << "      Welcome To Main Menu Screen!\n";
  system("pause");
}

void handleTransactionsMenuChoice(enTransactions choise,
                                  vector<stClientData> &vClients) {
  system("cls");

  switch (choise) {
  case enTransactions::eDeposit:
    depositScreen(vClients);
    break;

  case enTransactions::eWithdraw:
    withdrawScreen(vClients);
    break;

  case enTransactions::eTotalBalances:
    totalBalancesScreen(vClients);
    break;

  case enTransactions::eBackMainMenu:
    mainMenuScreen();
    break;

  default:
    break;
  }

  saveDataToFile("Clients.txt", vClients);
}

void transactionsScreen(vector<stClientData> &vClients) {
  enTransactions choise;

  do {
    printTransactionsMenueScreen();

    choise = static_cast<enTransactions>(
        readNumber<int>("Enter your choise: ", 1, 4));

    handleTransactionsMenuChoice(choise, vClients);
  } while (choise != enTransactions::eBackMainMenu);
}

void handleMainMenuChoice(enChoice choise, vector<stClientData> &vClients) {
  system("cls");

  switch (choise) {
  case enChoice::eShowClientList:
    printClientsInTableView(vClients);
    break;

  case enChoice::eAddNewClient:
    addNewClientScreen(vClients);
    break;

  case enChoice::eDeleteClient:
    deleteClientScreen(vClients);
    break;

  case enChoice::eUpdateClient:
    updateClientScreen(vClients);
    break;

  case enChoice::eFindClient:
    findClientScreen(vClients);
    break;

  case enChoice::eTransactions:
    transactionsScreen(vClients);
    break;

  case enChoice::eExit:
    exitScreen();
    break;

  default:
    system("pause");
    break;
  }

  saveDataToFile("Clients.txt", vClients);
}

int main() {
  vector<stClientData> vClients;
  enChoice choise;

  do {
    system("cls");
    vClients = loadDataFromFile("Clients.txt");

    printMainMenueScreen();

    choise =
        static_cast<enChoice>(readNumber<int>("Enter your choise: ", 1, 7));

    handleMainMenuChoice(choise, vClients);
  } while (choise != enChoice::eExit);

  return 0;
}