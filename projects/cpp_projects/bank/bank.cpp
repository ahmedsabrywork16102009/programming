
#include "../../../libraries/cpp_libraries/domain/bank.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string_view>
#include <thread>
#include <vector>

#include "../../../libraries/cpp_libraries/io/input.h"
#include "../../../libraries/cpp_libraries/process.h"

using namespace std;

static const string g_clientsFileName = "files/Clients.txt";
static const string g_usersFileName = "files/Users.txt";

enum enMainMinuChoice { eShowClientList = 1, eAddNewClient = 2, eDeleteClient = 3, eUpdateClient = 4, eFindClient = 5, eTransactions = 6, eManageUsers = 7, eLogOut = 8 };
enum enManageUsers { eShowUsersList = 1, eAddNewUser = 2, eDeleteUser = 3, eUpdateUser = 4, eFindUser = 5, eBackMainMenu = 6 };
enum enTransactions { eDeposit = 1, eWithdraw = 2, eTotalBalances = 3, eBackToMainMenu = 4 };
enum stType { CLIENT_DATA = 0, USER_DATA = 1 };

bank::stClientData readNewClient(vector<bank::stClientData> &vClients) { return bank::readNewRecord(vClients); }

bank::stUser readNewUser(vector<bank::stUser> &vUsers) { return bank::readNewRecord(vUsers); }

bool updateClientData(bank::stClientData &clientData) { return bank::updateRecord(clientData); }

bool updateUserData(bank::stUser &userData) { return bank::updateRecord(userData); }

bool deleteClient(vector<bank::stClientData> &vClients, const string &accountNumber) {
	bool deleted = bank::deleteRecord(vClients, accountNumber, [](const bank::stClientData &, int) { return false; }, "Client");

	return deleted;
}

bool deleteUser(vector<bank::stUser> &vUsers, const string &userName) {
	bool deleted = bank::deleteRecord(vUsers, userName, [](const bank::stUser &user, int index) { return index == 0 || user.userName == "admin"; }, "User");

	return deleted;
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

void printClientsInTableView(const vector<bank::stClientData> &vClients) { bank::printRecordsInTableView(vClients); }

void printUsersInTableView(const vector<bank::stUser> &vUsers) { bank::printRecordsInTableView(vUsers); }

void addNewClientScreen(vector<bank::stClientData> &vClients) {
	do {
		vClients.push_back(readNewClient(vClients));
	} while (input::readBool("Do you want to add another client [1-Yes] [0-No]: "));
}

void addNewUserScreen(vector<bank::stUser> &vUsers) {
	do {
		vUsers.push_back(readNewUser(vUsers));
	} while (input::readBool("Do you want to add another user [1-Yes] [0-No]: "));
}

void updateClientScreen(vector<bank::stClientData> &vClients) { bank::updateRecordScreen(vClients, "Enter Account Number: ", "Client"); }

void updateUserScreen(vector<bank::stUser> &vUsers) { bank::updateRecordScreen(vUsers, "Enter User Name: ", "User"); }

void deleteClientScreen(vector<bank::stClientData> &vClients) {
	if (input::readBool("Do you want to print data of this client before delete it? ")) {
		printClientsInTableView(vClients);
	}

	output::resetScreen();

	int indexOfClient = bank::getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	if (indexOfClient != -1) {
		deleteClient(vClients, vClients[indexOfClient].accountNumber);
	} else {
		cout << "Client Not Found!\n";
	}
}

void deleteUserScreen(vector<bank::stUser> &vUsers) {
	if (input::readBool("Do you want to print data of this user before delete it? ")) {
		printUsersInTableView(vUsers);
	}

	output::resetScreen();

	int indexOfUser = bank::getIndexOfUserByUsername(vUsers, input::readString("Enter User Name: "));

	if (indexOfUser != -1) {
		deleteUser(vUsers, vUsers[indexOfUser].userName);
	} else {
		cout << "User Not Found!\n";
	}
}

void findClientScreen(vector<bank::stClientData> &vClients) { bank::findRecordScreen(vClients, "Enter Account Number: ", "Client"); }

void findUserScreen(vector<bank::stUser> &vUsers) { bank::findRecordScreen(vUsers, "Enter User Name: ", "User"); }

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

void depositScreen(vector<bank::stClientData> &vClients) {
	int indexOfClient = bank::getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	double depositValue = 0;
	bool isFirstTime = 1;

	if (indexOfClient != -1) {
		if (isFirstTime) {
			bank::printClientDataVertical(vClients, indexOfClient);

			isFirstTime = 0;
		}

		depositValue = input::readNumber<double>("Enter deposit value: ", 0.0, numeric_limits<double>::max());

		if (input::readBool("Are you sure you want to deposit this client [1] Yes [0] No: ")) {
			bank::deposit(vClients, indexOfClient, depositValue);
			cout << "\nClient Deposit Successfully!\n";
		} else {
			cout << "Operation Cancelled!\n";
		}

	} else {
		cout << "Client Not Found!\n";
	}
}

void withdrawScreen(vector<bank::stClientData> &vClients) {
	int indexOfClient = bank::getIndexOfClientByAccountNumber(vClients, input::readString("Enter Account Number: "));

	double withdrawValue = 0;
	bool hasEnoughBalance = 0;
	bool isFirstTime = 1;

	if (indexOfClient != -1) {
		do {
			if (isFirstTime) {
				bank::printClientDataVertical(vClients, indexOfClient);

				isFirstTime = 0;
			}

			withdrawValue = input::readNumber<double>("Enter withdraw value: ", 0.0, numeric_limits<double>::max());

			if ((hasEnoughBalance = bank::isClientHasEnoughBalance(vClients, indexOfClient, withdrawValue))) {
				bank::withdraw(vClients, indexOfClient, withdrawValue);
				cout << "\nClient Withdraw Successfully!\n";
				break;
			} else {
				cout << "\nClient Doesn't Have Enough Balance!\n";
			}
		} while (!hasEnoughBalance);
	} else {
		cout << "Client Not Found!\n";
	}
}

void totalBalancesScreen(vector<bank::stClientData> &vClients) {
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
	cout << "Total Balances: " << bank::getTotalBalances(vClients) << " $\n";
	cout << "================================================================-\n";
}

bool handleTransactionsMenuChoice(enTransactions choise, vector<bank::stClientData> &vClients) {
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

	bank::saveDataToFile(g_clientsFileName, vClients);
	return returnToMainMenu;
}

void transactionsScreen(vector<bank::stClientData> &vClients) {
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

bool handleManageUsersMenuChoice(enManageUsers choise, vector<bank::stUser> &vUsers) {
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

	bank::saveDataToFile(g_usersFileName, vUsers);
	return returnToMainMenu;
}

void manageUsersScreen() {
	bool returnToMainMenu = false;
	do {
		output::resetScreen();
		printManageUsersMenuScreen();
		vector<bank::stUser> vUsers = bank::loadUsersDataFromFile(g_usersFileName);

		enManageUsers choice = static_cast<enManageUsers>(input::readNumber<int>("Enter your choice: ", 1, 6));
		returnToMainMenu = handleManageUsersMenuChoice(choice, vUsers);
	} while (!returnToMainMenu);
}

bool handleMainMenuChoice(enMainMinuChoice choise, vector<bank::stClientData> &vClients) {
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

		break;
	}

	bank::saveDataToFile(g_clientsFileName, vClients);
	return shouldLogOut;
}

bool hasPermission(short permission, short permissionMask) { return (permission & permissionMask) != 0; }

void mainApp(short permission) {
	vector<bank::stClientData> vClients;
	bool shouldLogOut = false;

	while (!shouldLogOut) {
		output::resetScreen();
		vClients = bank::loadClientDataFromFile(g_clientsFileName);

		printMainMenueScreen();

		enMainMinuChoice choise = static_cast<enMainMinuChoice>(input::readNumber<int>("Enter your choise: ", 1, 8));

		if (choise != enMainMinuChoice::eLogOut && !hasPermission(permission, static_cast<short>(1 << (choise - 1)))) {
			cout << "You don't have permission to perform this action.\n";
			output::resetScreen(true, true, 2);
			continue;
		}

		shouldLogOut = handleMainMenuChoice(choise, vClients);
	}
}

bool loginLogic(const vector<bank::stUser> &vUsers) {
	bool isPasswordCorrect = false;
	bool isFirstLoginAttempt = true;

	while (!isPasswordCorrect) {
		output::resetScreen();
		bank::printLoginScreenLogo();

		if (!isFirstLoginAttempt) {
			cout << "Incorrect Account Number or Pin Code. Please try again.\n";
		}
		isFirstLoginAttempt = false;

		string userName = input::readString("Enter User Name: ");
		string password = input::readString("Enter Password: ");

		for (const auto &user : vUsers) {
			if ((user.userName == userName) && (user.password == password)) {
				cout << "Login Successful!\n";
				this_thread::sleep_for(chrono::seconds(1));

				int indexOfUser = bank::getIndexOfUserByUsername(vUsers, userName);

				mainApp(vUsers[indexOfUser].permission);

				isPasswordCorrect = true;
				return false;
			}
		}
	}
	while (input::readBool("Login Failed!\n Do you want to try again? [1] Yes [0] No: "));

	return false;
}

void logIn() {
	vector<bank::stUser> vUsers = bank::loadUsersDataFromFile(g_usersFileName);

	do {
		output::resetScreen();

	} while (loginLogic(vUsers));
}

int main() {
	logIn();

	return 0;
}