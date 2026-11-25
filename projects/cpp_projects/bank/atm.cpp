#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "../../../libraries/cpp_libraries/domain/bank.h"
#include "../../../libraries/cpp_libraries/io/input.h"

using namespace std;

static const string g_clientsFileName = "files/Clients.txt";

enum en_ATM_MainMinueChoice { eQuickWithdraw = 1, eNormalWithdraw = 2, eDeposit = 3, eShowTotalBalances = 4, e_ATM_LogOut = 5 };
enum enQuickWithdraw { e50 = 1, e100 = 2, e200 = 3, e500 = 4, e1000 = 5, e2000 = 6, eBackToMainMenu = 7 };

void printTotalBalanceLogo() {
	cout << "===========================================\n";
	cout << "            Total Balance \n";
	cout << "===========================================\n";
}

void printNormalWithdrawLogo() {
	cout << "===========================================\n";
	cout << "             Normal Withdraw \n";
	cout << "===========================================\n";
}
void printQuickWithdrawLogo() {
	cout << "===========================================\n";
	cout << "              Quick Withdraw \n";
	cout << "===========================================\n";
}

void printDepositLogo() {
	cout << "===========================================\n";
	cout << "              Deposit \n";
	cout << "===========================================\n";
}
void printQuickWithdrawScreen() {
	cout << "===========================================\n";
	cout << "              QUICK WITHDRAW SCREEN \n";
	cout << "===========================================\n";
	cout << "       [1] 50.      [2] 100.\n";
	cout << "       [3] 200.     [4] 500.\n";
	cout << "       [5] 1000.    [6] 2000.\n";
	cout << "       [7] Exit.\n";
	cout << "===========================================\n";
}

int getQuickWithdrawAmount(enQuickWithdraw choise) {
	switch (choise) {
	case enQuickWithdraw::e50:
		return 50;
	case enQuickWithdraw::e100:
		return 100;
	case enQuickWithdraw::e200:
		return 200;
	case enQuickWithdraw::e500:
		return 500;
	case enQuickWithdraw::e1000:
		return 1000;
	case enQuickWithdraw::e2000:
		return 2000;
	default:
		return 0;
	}
}

void quickWithdrawScreen(vector<bank::stClientData> &vClients, int indexOfClient) {
	do {
		output::resetScreen();
		bank::loadClientDataFromFile(g_clientsFileName);

		printQuickWithdrawScreen();
		bank::PrintBalanceScreen(vClients, indexOfClient);

		enQuickWithdraw choise = static_cast<enQuickWithdraw>(input::readNumber<int>("Enter your choise: ", 1, 7));
		if (choise == enQuickWithdraw::eBackToMainMenu) {
			return;
		}

		if (choise != enQuickWithdraw::e50 && choise != enQuickWithdraw::e100 && choise != enQuickWithdraw::e200 && choise != enQuickWithdraw::e500 && choise != enQuickWithdraw::e1000 && choise != enQuickWithdraw::e2000) {
			cout << "Invalid choice. Please try again.\n";
			continue;
		}

		if (input::readBool("Are you sure you want to withdraw " + to_string(getQuickWithdrawAmount(choise)) + "? [1] Yes [0] No: ")) {
			if (bank::isClientHasEnoughBalance(vClients[indexOfClient], static_cast<double>(getQuickWithdrawAmount(choise)))) {
				bank::withdrawAmount(vClients[indexOfClient], static_cast<double>(getQuickWithdrawAmount(choise)));
				cout << "Withdrawal successful. New balance: " << vClients[indexOfClient].accountBalance << "\n";
			} else {
				cout << "Insufficient funds. Current balance: " << vClients[indexOfClient].accountBalance << "\n";
			}
			bank::saveDataToFile(g_clientsFileName, vClients);
		}

	} while (input::readBool("Do you want to make another quick withdraw? [1] Yes [0] No: "));
}

void normalWithdrawScreen(vector<bank::stClientData> &vClients, int indexOfClient) {
	do {
		output::resetScreen();
		bank::loadClientDataFromFile(g_clientsFileName);
		printNormalWithdrawLogo();

		bank::PrintBalanceScreen(vClients, indexOfClient);
		double amount = input::readNumber<double>("Enter the amount to withdraw: ", 0.01, vClients[indexOfClient].accountBalance);

		if (input::readBool("Are you sure you want to withdraw " + to_string(amount) + "? [1] Yes [0] No: ")) {
			if (bank::isClientHasEnoughBalance(vClients[indexOfClient], amount)) {
				bank::withdrawAmount(vClients[indexOfClient], amount);
				cout << "Withdrawal successful. New balance: " << vClients[indexOfClient].accountBalance << "\n";
			} else {
				cout << "Insufficient funds. Current balance: " << vClients[indexOfClient].accountBalance << "\n";
			}
			bank::saveDataToFile(g_clientsFileName, vClients);
		}
	} while (input::readBool("Do you want to make another normal withdraw? [1] Yes [0] No: "));
}

void depositScreen(vector<bank::stClientData> &vClients, int indexOfClient) {
	do {
		output::resetScreen();
		bank::loadClientDataFromFile(g_clientsFileName);
		printDepositLogo();
		bank::PrintBalanceScreen(vClients, indexOfClient);
		double amount = input::readNumber<double>("Enter the amount to deposit: ", 0.01);

		if (input::readBool("Are you sure you want to deposit " + to_string(amount) + "? [1] Yes [0] No: ")) {
			bank::depositAmount(vClients[indexOfClient], amount);
			cout << "Deposit successful. New balance: " << vClients[indexOfClient].accountBalance << "\n";
			bank::saveDataToFile(g_clientsFileName, vClients);
		}
	} while (input::readBool("Do you want to make another deposit? [1] Yes [0] No: "));
}

void showTotalBalancesScreen(const vector<bank::stClientData> &vClients, int indexOfClient) {
	output::resetScreen();

	printTotalBalanceLogo();
	cout << "Total Balances of All Clients: " << vClients[indexOfClient].accountBalance << "\n\n";

	system("pause");
}

bool handle_ATM_MenuChoice(en_ATM_MainMinueChoice choise, vector<bank::stClientData> &vClients, int indexOfClient) {
	bool shouldLogOut = false;

	switch (choise) {
	case en_ATM_MainMinueChoice::eQuickWithdraw:
		quickWithdrawScreen(vClients, indexOfClient);
		break;

	case en_ATM_MainMinueChoice::eNormalWithdraw:
		normalWithdrawScreen(vClients, indexOfClient);
		break;

	case en_ATM_MainMinueChoice::eDeposit:
		depositScreen(vClients, indexOfClient);
		break;

	case en_ATM_MainMinueChoice::eShowTotalBalances:
		showTotalBalancesScreen(vClients, indexOfClient);
		break;

	case en_ATM_MainMinueChoice::e_ATM_LogOut: {
		shouldLogOut = input::readBool("Are you sure you want to log out? [1] Yes [0] No: ");
		if (shouldLogOut) {
			cout << "Log Out Successfully!\n";
		} else {
			cout << "Log out cancelled. Returning to Main Menu...\n";
		}
		break;
	}

	default:
		cout << "Invalid choice. Please try again.\n";
		break;
	}

	bank::saveDataToFile(g_clientsFileName, vClients);
	return shouldLogOut;
}

void print_ATM_MainMenueScreen() {
	cout << "===========================================\n";
	cout << "              ATM MAIN MENUE \n";
	cout << "===========================================\n";
	cout << "    1. Quick Withdraw.\n";
	cout << "    2. Normal Withdraw.\n";
	cout << "    3. Deposit.\n";
	cout << "    4. Show Total Balances.\n";
	cout << "    5. Log Out.\n";
	cout << "===========================================\n";
}

void mainApp(vector<bank::stClientData> vClients, const int indexOfClient) {
	bool shouldLogOut = false;

	while (!shouldLogOut) {
		output::resetScreen();

		print_ATM_MainMenueScreen();

		en_ATM_MainMinueChoice choise = static_cast<en_ATM_MainMinueChoice>(input::readNumber<int>("Enter your choise: ", 1, 5));

		shouldLogOut = handle_ATM_MenuChoice(choise, vClients, indexOfClient);
	}
}

bool loginLogic(const vector<bank::stClientData> &vClients) {
	bool isPasswordCorrect = false;
	bool isFirstLoginAttempt = true;

	while (!isPasswordCorrect) {
		output::resetScreen();
		bank::printLoginScreenLogo();

		if (!isFirstLoginAttempt) {
			cout << "Incorrect Account Number or Pin Code. Please try again.\n";
		}
		isFirstLoginAttempt = false;

		string accountNumber = input::readString("Enter Account Number: ");
		string pinCode = input::readString("Enter Pin Code: ");

		for (const auto &client : vClients) {
			if ((client.accountNumber == accountNumber) && (client.pinCode == pinCode)) {
				cout << "Login Successful!\n";
				this_thread::sleep_for(chrono::seconds(1));

				int indexOfUser = bank::getIndexOfClientByAccountNumber(vClients, accountNumber);

				mainApp(vClients, indexOfUser);
				isPasswordCorrect = true;
				return false;
			}
		}
	}

	return false;
}

void logIn() {
	vector<bank::stClientData> vClients = bank::loadClientDataFromFile(g_clientsFileName);

	do {
		do {
			output::resetScreen();
		} while (loginLogic(vClients));
	} while (input::readBool("Do you want to log in again? [1] Yes [0] No: "));
}

int main() {
	logIn();

	return 0;
}
