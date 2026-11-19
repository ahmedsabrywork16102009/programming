#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>
#include <thread>
#include <vector>

#include "../../../libraries/cpp_libraries/input.h"
#include "../../../libraries/cpp_libraries/process.h"

using namespace std;

static const string g_clientsFileName = "files/Clients.txt";
static const string g_usersFileName = "files/Users.txt";

enum enMainMinuChoice { eShowClientList = 1, eAddNewClient = 2, eDeleteClient = 3, eUpdateClient = 4, eFindClient = 5, eTransactions = 6, eManageUsers = 7, eLogOut = 8 };
enum enManageUsers { eShowUsersList = 1, eAddNewUser = 2, eDeleteUser = 3, eUpdateUser = 4, eFindUser = 5, eBackMainMenu = 6 };
enum enTransactions { eDeposit = 1, eWithdraw = 2, eTotalBalances = 3, eBackToMainMenu = 4 };

struct stClientData {
	string accountNumber;
	string pinCode;
	string fullName;
	string phone;
	double accountBalance;
};

struct stUser {
	string userName;
	string password;
	short permission;
};

stClientData convertClientLineDataToRecord(string text, string_view delimiter = "#//#") {
	stClientData clientData;

	vector<string> vTokens = process::splitByString(text, delimiter);

	clientData.accountNumber = vTokens[0];
	clientData.pinCode = vTokens[1];
	clientData.fullName = vTokens[2];
	clientData.phone = vTokens[3];
	clientData.accountBalance = stod(vTokens[4]);

	return clientData;
}
stUser convertUsersLineToRecord(string text, string_view delimiter = "#//#") {
	stUser user;

	vector<string> vTokens = process::splitByString(text, delimiter);

	user.userName = vTokens[0];
	user.password = vTokens[1];
	user.permission = stoi(vTokens[2]);

	return user;
}

string convertRecordToLine(const stUser &user, string_view seperator = "#//#") {
	string line = "";

	line.append(user.userName).append(seperator);
	line.append(user.password).append(seperator);
	line.append(to_string(user.permission));

	return line;
}

string convertRecordToLine(const stClientData &clientData, string_view seperator = "#//#") {
	string line = "";

	line.append(clientData.accountNumber).append(seperator);
	line.append(clientData.pinCode).append(seperator);
	line.append(clientData.fullName).append(seperator);
	line.append(clientData.phone).append(seperator);
	line.append(to_string(clientData.accountBalance));

	return line;
}

vector<stClientData> loadClientDataFromFile(const string &fileName) {
	vector<stClientData> vClients;
	fstream myFile;

	myFile.open(fileName, ios::in);

	if (myFile.is_open()) {
		string lineData;

		while (getline(myFile, lineData)) {
			vClients.push_back(convertClientLineDataToRecord(lineData));
		}

		myFile.close();
	}

	return vClients;
}

vector<stUser> loadUsersDataFromFile(const string &fileName) {
	vector<stUser> vUsers;
	fstream myFile;

	myFile.open(fileName, ios::in);

	if (myFile.is_open()) {
		string lineData;

		while (getline(myFile, lineData)) {
			vector<string> vTokens = process::splitByString(lineData, "#//#");
			if (vTokens.size() == 3 && !vTokens[0].empty() && !vTokens[1].empty()) {
				vUsers.push_back(convertUsersLineToRecord(lineData));
			}
		}

		myFile.close();
	}

	return vUsers;
}

void saveDataToFile(const string &fileName, const vector<stClientData> &vClientData) {
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

void saveDataToFile(const string &fileName, const vector<stUser> &vUsers) {
	fstream myFile;
	myFile.open(fileName, ios::out);

	if (myFile.is_open()) {
		for (const auto &user : vUsers) {
			myFile << convertRecordToLine(user) << "\n";
		}

		myFile.close();
	}
}

int getIndexOfClientByAccountNumber(const vector<stClientData> &vClients, const string &accountNumber) {
	for (size_t i = 0; i < vClients.size(); i++) {
		if (vClients[i].accountNumber == accountNumber) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

int getIndexOfUserByUsername(const vector<stUser> &vUsers, const string &username) {
	for (size_t i = 0; i < vUsers.size(); i++) {
		if (vUsers[i].userName == username) {
			return static_cast<int>(i);
		}
	}

	return -1;
}

bool isClientFound(const vector<stClientData> &vClients, const string &accountNumber) { return getIndexOfClientByAccountNumber(vClients, accountNumber) != -1; }
bool isUserFound(const vector<stUser> &vUsers, const string &username) { return getIndexOfUserByUsername(vUsers, username) != -1; }

void printClientDataVertical(const vector<stClientData> &vClients, int indexOfClient) {
	cout << "Account Number  : " << vClients[indexOfClient].accountNumber << "\n";
	cout << "Pin Code        : " << vClients[indexOfClient].pinCode << "\n";
	cout << "Full Name       : " << vClients[indexOfClient].fullName << "\n";
	cout << "Phone Number    : " << vClients[indexOfClient].phone << "\n";
	cout << "Account Balance : " << vClients[indexOfClient].accountBalance << "\n";
}

void printClientDataVertical(const stClientData &clientData) {
	cout << "Account Number  : " << clientData.accountNumber << "\n";
	cout << "Pin Code        : " << clientData.pinCode << "\n";
	cout << "Full Name       : " << clientData.fullName << "\n";
	cout << "Phone Number    : " << clientData.phone << "\n";
	cout << "Account Balance : " << clientData.accountBalance << "\n";
}

void printUserDataVertical(const vector<stUser> &vUsers, int indexOfUser) {
	cout << "User Name       : " << vUsers[indexOfUser].userName << "\n";
	cout << "Password        : " << vUsers[indexOfUser].password << "\n";
	cout << "Permission      : " << vUsers[indexOfUser].permission << "\n";
}

void printUserDataVertical(const stUser &userData) {
	cout << "User Name       : " << userData.userName << "\n";
	cout << "Password        : " << userData.password << "\n";
	cout << "Permission      : " << userData.permission << "\n";
}

void printFindResult(const vector<stClientData> &vClients, int index) {
	if (index != -1) {
		cout << "\nClient Found:\n";
		printClientDataVertical(vClients, index);
	} else {
		cout << "\nClient Not Found!\n";
	}
}

void printFindResult(const vector<stUser> &vUsers, int index) {
	if (index != -1) {
		cout << "\nUser Found:\n";
		printUserDataVertical(vUsers, index);
	} else {
		cout << "\nUser Not Found!\n";
	}
}

stClientData readNewClient(vector<stClientData> &vClients) {
	stClientData clientData;
	bool isRepeated = false;

	do {
		if (isRepeated) {
			cout << "Client with [" << clientData.accountNumber << "] is already exists, try again.\n";
		}
		isRepeated = true;

		clientData.accountNumber = input::readString("Enter Account Number: ");
	} while (isClientFound(vClients, clientData.accountNumber));

	clientData.pinCode = input::readString("Enter Pin Code: ");
	clientData.fullName = input::readString("Enter Full Name: ");
	clientData.phone = input::readString("Enter Phone: ");
	clientData.accountBalance = input::readNumber<double>("Enter Account Balance: ");

	return clientData;
}

void whatPermissionToGive(short &permission) {
	permission = 0;

	if (input::readBool("Do you want to give permission to Show client list? [1] Yes [0] No: ")) permission |= 1 << 0;  // Bit 0: Show client list
	if (input::readBool("Do you want to give permission to Add new client? [1] Yes [0] No: ")) permission |= 1 << 1;    // Bit 1: Add new client
	if (input::readBool("Do you want to give permission to Delete client? [1] Yes [0] No: ")) permission |= 1 << 2;     // Bit 2: Delete client
	if (input::readBool("Do you want to give permission to Update client? [1] Yes [0] No: ")) permission |= 1 << 3;     // Bit 3: Update client
	if (input::readBool("Do you want to give permission to Find client? [1] Yes [0] No: ")) permission |= 1 << 4;       // Bit 4: Find client
	if (input::readBool("Do you want to give permission to Transactions? [1] Yes [0] No: ")) permission |= 1 << 5;      // Bit 5: Transactions
	if (input::readBool("Do you want to give permission to Manage users? [1] Yes [0] No: ")) permission |= 1 << 6;      // Bit 6: Manage users
}

stUser readNewUser(vector<stUser> &vUsers) {
	stUser userData;
	bool isRepeated = false;

	do {
		if (isRepeated) {
			cout << "User with [" << userData.userName << "] already exists, try again.\n";
		}
		isRepeated = true;

		userData.userName = input::readString("Enter Username: ");
	} while (isUserFound(vUsers, userData.userName));

	userData.password = input::readString("Enter Password: ");
	if (input::readBool("Do you want to give Full access permission? [1] Yes [0] No: ")) {
		userData.permission = -1;
	} else {
		cout << "Select the permissions to grant this user:-\n";

		whatPermissionToGive(userData.permission);
	}

	return userData;
}

bool updateClientData(stClientData &clientData) {
	printClientDataVertical(clientData);

	if (input::readBool("Are you sure you want to update this client [1] Yes [0] No: ")) {
		if (input::readBool("Do you want to update account number? ")) {
			clientData.accountNumber = input::readString("Enter Account Number: ");
		}

		if (input::readBool("Do you want to update pin code? ")) {
			clientData.pinCode = input::readString("Enter Pin Code: ");
		}

		if (input::readBool("Do you want to update full name? ")) {
			clientData.fullName = input::readString("Enter Full Name: ");
		}

		if (input::readBool("Do you want to update phone? ")) {
			clientData.phone = input::readString("Enter Phone: ");
		}

		if (input::readBool("Do you want to update account balance? ")) {
			clientData.accountBalance = input::readNumber<double>("Enter Account Balance: ");
		}
		return true;
	}

	return false;
}

bool updateUserData(stUser &userData) {
	printUserDataVertical(userData);

	if (input::readBool("Are you sure you want to update this user [1] Yes [0] No: ")) {
		if (input::readBool("Do you want to update username? ")) {
			userData.userName = input::readString("Enter Username: ");
		}

		if (input::readBool("Do you want to update password? ")) {
			userData.password = input::readString("Enter Password: ");
		}

		if (input::readBool("Do you want to update permission? ")) {
			if (input::readBool("Do you want to give Full access permission? [1] Yes [0] No: ")) {
				userData.permission = -1;
			} else {
				cout << "Select the permissions to grant this user:-\n";

				whatPermissionToGive(userData.permission);
			}
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

	if (input::readBool("Are you sure you want to delete this client [1] Yes [0] No: ")) {
		vClients.erase(vClients.begin() + indexOfClient);
		cout << "Client Deleted Successfully!\n";

		system("pause");
		return true;
	}
	cout << "Operation Cancelled!\n";

	system("pause");
	return false;
}

bool deleteUser(vector<stUser> &vUsers, const string &userName) {
	int indexOfUser = getIndexOfUserByUsername(vUsers, userName);

	if (indexOfUser == -1) {
		return false;
	}

	printUserDataVertical(vUsers, indexOfUser);

	if (input::readBool("Are you sure you want to delete this user [1] Yes [0] No: ")) {
		vUsers.erase(vUsers.begin() + indexOfUser);
		cout << "User Deleted Successfully!\n";

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
	cout << "    7. Manage Users.\n";
	cout << "    8. Exit.\n";
	cout << "===========================================\n";
}

void printClientsInTableView(const vector<stClientData> &vClients) {
	string_view line =
	    "---------------------------------------------------------"
	    "----------------------------";
	cout << "\n" << setw(45) << "Number of Clients: " << vClients.size() << "\n";
	cout << line << endl;
	cout << "| " << left << setw(10) << "Account Number" << " | " << left << setw(10) << "Pin Code" << " | " << left << setw(20) << "Full Name"
	     << " | " << left << setw(15) << "Phone" << " | " << left << setw(10) << "Balance" << " |\n";
	cout << line << endl;

	for (const auto &clientData : vClients) {
		cout << "| " << left << setw(14) << clientData.accountNumber << " | " << left << setw(10) << clientData.pinCode << " | " << left << setw(20) << clientData.fullName << " | " << left << setw(15) << clientData.phone << " | " << left << setw(10) << clientData.accountBalance << " |\n";
	}
	cout << line << "\n";

	system("pause");
}

void printUsersInTableView(const vector<stUser> &vUsers) {
	const string line(52, '-');
	cout << "\n" << setw(45) << "Number of Users: " << vUsers.size() << "\n";
	cout << line << "\n";
	cout << "| " << left << setw(16) << "User Name" << " | " << setw(14) << "Password" << " | " << setw(12) << "Permission" << " |\n";
	cout << line << "\n";

	for (const auto &userData : vUsers) {
		cout << "| " << left << setw(16) << userData.userName << " | " << setw(14) << userData.password << " | " << setw(12) << userData.permission << " |\n";
	}
	cout << line << "\n";

	system("pause");
}

void addNewClientScreen(vector<stClientData> &vClients) {
	do {
		vClients.push_back(readNewClient(vClients));
	} while (input::readBool("Do you want to add another client [1-Yes] [0-No]: "));

	system("pause");
}

void addNewUserScreen(vector<stUser> &vUsers) {
	do {
		vUsers.push_back(readNewUser(vUsers));
	} while (input::readBool("Do you want to add another user [1-Yes] [0-No]: "));
}

void updateClientScreen(vector<stClientData> &vClients) {
	int indexOfClient = getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	if (indexOfClient != -1) {
		updateClientData(vClients[indexOfClient]);
	} else {
		cout << "Client Not Found!\n";
	}

	system("pause");
}

void updateUserScreen(vector<stUser> &vUsers) {
	int indexOfUser = getIndexOfUserByUsername(vUsers, input::readString("Enter User Name: "));

	if (indexOfUser != -1) {
		updateUserData(vUsers[indexOfUser]);
	} else {
		cout << "User Not Found!\n";
	}

	system("pause");
}

void deleteClientScreen(vector<stClientData> &vClients) {
	if (input::readBool("Do you want to print data of this client before delete it? ")) {
		printClientsInTableView(vClients);

		system("pause");
	}

	output::resetScreen();

	int indexOfClient = getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	if (indexOfClient != -1) {
		deleteClient(vClients, vClients[indexOfClient].accountNumber);
	} else {
		cout << "Client Not Found!\n";
	}

	system("pause");
}

void deleteUserScreen(vector<stUser> &vUsers) {
	if (input::readBool("Do you want to print data of this user before delete it? ")) {
		printUsersInTableView(vUsers);
	}

	output::resetScreen();

	int indexOfUser = getIndexOfUserByUsername(vUsers, input::readString("Enter User Name: "));

	if (indexOfUser != -1) {
		deleteUser(vUsers, vUsers[indexOfUser].userName);
	} else {
		cout << "User Not Found!\n";
	}

}

void findClientScreen(vector<stClientData> &vClients) {
	printFindResult(vClients, getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: ")));

	system("pause");
}

void findUserScreen(vector<stUser> &vUsers) {
	printFindResult(vUsers, getIndexOfUserByUsername(vUsers, input::readString("Enter User Name: ")));

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

void deposit(vector<stClientData> &vClients, int indexOfClient, double depositValue) { vClients[indexOfClient].accountBalance += depositValue; }

void depositScreen(vector<stClientData> &vClients) {
	int indexOfClient = getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	double depositValue = 0;
	bool isFirstTime = 1;

	if (indexOfClient != -1) {
		if (isFirstTime) {
			printClientDataVertical(vClients, indexOfClient);

			isFirstTime = 0;
		}

		depositValue = input::readNumber<double>("Enter deposit value: ", 0.0, std::numeric_limits<double>::max());

		if (input::readBool("Are you sure you want to deposit this client [1] Yes [0] No: ")) {
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

void withdraw(vector<stClientData> &vClients, int indexOfClient, double withdrawValue) { vClients[indexOfClient].accountBalance -= withdrawValue; }

bool isClientHasEnoughBalance(vector<stClientData> &vClients, int indexOfClient, double withdrawValue) { return vClients[indexOfClient].accountBalance >= withdrawValue; }

void withdrawScreen(vector<stClientData> &vClients) {
	int indexOfClient = getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	double withdrawValue = 0;
	bool hasEnoughBalance = 0;
	bool isFirstTime = 1;

	if (indexOfClient != -1) {
		do {
			if (isFirstTime) {
				printClientDataVertical(vClients, indexOfClient);

				isFirstTime = 0;
			}

			withdrawValue = input::readNumber<double>("Enter withdraw value: ", 0.0, std::numeric_limits<double>::max());

			if ((hasEnoughBalance = isClientHasEnoughBalance(vClients, indexOfClient, withdrawValue))) {
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
	string_view line = "-----------------------------------------------------------------";
	cout << "\n" << setw(45) << "Number of Clients: " << vClients.size() << "\n";
	cout << line << endl;
	cout << "| " << left << setw(10) << "Account Number" << " | " << left << setw(20) << "Full Name" << " | " << left << setw(10) << "Balance"
	     << " |\n";
	cout << line << endl;

	for (const auto &clientData : vClients) {
		cout << "| " << left << setw(14) << clientData.accountNumber << " | " << left << setw(20) << clientData.fullName << left << setw(10) << clientData.accountBalance << " |\n";
	}
	cout << line << "\n";
	cout << "================================================================-\n";
	cout << "Total Balances: " << getTotalBalances(vClients) << " $\n";
	cout << "================================================================-\n";

	system("pause");
}

bool handleTransactionsMenuChoice(enTransactions choise, vector<stClientData> &vClients) {
	output::resetScreen();
	bool returnToMainMenu = false;

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

	case enTransactions::eBackToMainMenu:
		returnToMainMenu = input::readBool("Are you sure you want to go back to the main menu? [1] Yes [0] No: ");
		if (returnToMainMenu) {
			cout << "Returning to Main Menu...\n";
		} else {
			cout << "Returning to Transactions Menu...\n";
		}
		break;

	default:
		break;
	}

	saveDataToFile(g_clientsFileName, vClients);
	return returnToMainMenu;
}

void transactionsScreen(vector<stClientData> &vClients) {
	bool returnToMainMenu = false;

	while (!returnToMainMenu) {
		printTransactionsMenueScreen();

		enTransactions choise = static_cast<enTransactions>(input::readNumber<int>("Enter your choise: ", 1, 4));

		returnToMainMenu = handleTransactionsMenuChoice(choise, vClients);
	}
}

void printManageUsersMenuScreen() {
	cout << "===========================================\n";
	cout << "              MANAGE USERS MENU \n";
	cout << "===========================================\n";
	cout << "    1. Show Users List.\n";
	cout << "    2. Add New User.\n";
	cout << "    3. Delete User.\n";
	cout << "    4. Update User.\n";
	cout << "    5. Find User.\n";
	cout << "    6. Back to Main Menu.\n";
	cout << "===========================================\n";
}

bool handleManageUsersMenuChoice(enManageUsers choise, vector<stUser> &vUsers) {
	output::resetScreen();
	bool returnToMainMenu = false;

	switch (choise) {
	case enManageUsers::eShowUsersList:
		printUsersInTableView(vUsers);
		break;

	case enManageUsers::eAddNewUser:
		addNewUserScreen(vUsers);
		break;

	case enManageUsers::eDeleteUser:
		deleteUserScreen(vUsers);
		break;

	case enManageUsers::eUpdateUser:
		updateUserScreen(vUsers);
		break;

	case enManageUsers::eFindUser:
		findUserScreen(vUsers);
		break;

	case enManageUsers::eBackMainMenu:
		returnToMainMenu = true;
		break;

	default:
		break;
	}

	saveDataToFile(g_usersFileName, vUsers);
	return returnToMainMenu;
}

void manageUsersScreen() {
	bool returnToMainMenu = false;
	do {
		output::resetScreen();
		printManageUsersMenuScreen();
		vector<stUser> vUsers = loadUsersDataFromFile(g_usersFileName);

	 enManageUsers	choice = static_cast<enManageUsers>(input::readNumber<int>("Enter your choice: ", 1, 6));
		returnToMainMenu = handleManageUsersMenuChoice(choice, vUsers);
	} while (!returnToMainMenu);
}

bool handleMainMenuChoice(enMainMinuChoice choise, vector<stClientData> &vClients) {
	bool shouldLogOut = false;

	switch (choise) {
	case enMainMinuChoice::eShowClientList:
		output::resetScreen();
		printClientsInTableView(vClients);
		break;

	case enMainMinuChoice::eAddNewClient:
		output::resetScreen();
		addNewClientScreen(vClients);
		break;

	case enMainMinuChoice::eDeleteClient:
		output::resetScreen();
		deleteClientScreen(vClients);
		break;

	case enMainMinuChoice::eUpdateClient:
		output::resetScreen();
		updateClientScreen(vClients);
		break;

	case enMainMinuChoice::eFindClient:
		output::resetScreen();
		findClientScreen(vClients);
		break;

	case enMainMinuChoice::eTransactions:
		output::resetScreen();
		transactionsScreen(vClients);
		break;

	case enMainMinuChoice::eManageUsers:
		output::resetScreen();
		manageUsersScreen();
		break;

	case enMainMinuChoice::eLogOut:
		shouldLogOut = input::readBool("Are you sure you want to log out? [1] Yes [0] No: ");
		if (shouldLogOut) {
			cout << "Log Out Successfully!\n";
		} else {
			cout << "Log out cancelled. Returning to Main Menu...\n";
		}
		break;

	default:
		system("pause");
		break;
	}

	saveDataToFile(g_clientsFileName, vClients);
	return shouldLogOut;
}

void mainApp() {
	vector<stClientData> vClients;
	bool shouldLogOut = false;

	while (!shouldLogOut) {
		output::resetScreen();
		vClients = loadClientDataFromFile(g_clientsFileName);

		printMainMenueScreen();

		enMainMinuChoice choise = static_cast<enMainMinuChoice>(input::readNumber<int>("Enter your choise: ", 1, 8));

		shouldLogOut = handleMainMenuChoice(choise, vClients);
	}
}

void printLoginScreenLogo() {
	cout << "====================================\n";
	cout << "           LogIn Screen\n";
	cout << "====================================\n";
}

bool loginLogic(const vector<stUser> &vUsers) {
	do {
		output::resetScreen();
		printLoginScreenLogo();

		string userName = input::readString("Enter User Name: ");
		string password = input::readString("Enter Password: ");

		for (const auto &user : vUsers) {
			if ((user.userName == userName) && (user.password == password)) {
				cout << "Login Successful!\n";
				this_thread::sleep_for(chrono::seconds(1));

				mainApp();

				return false;
			}
		}
	} while (input::readBool("Login Failed!\n Do you want to try again? [1] Yes [0] No: "));

	return false;
}

void logIn() {
	vector<stUser> vUsers = loadUsersDataFromFile(g_usersFileName);

	do {
		output::resetScreen();

	} while (loginLogic(vUsers));
}

int main() {
	logIn();

	return 0;
}