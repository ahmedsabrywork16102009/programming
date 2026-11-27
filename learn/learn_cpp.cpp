#include <iostream>

#include "../libraries/cpp_libraries/as.h"

using namespace std;
using namespace AS;

int main() {
	int number = readNumber<int>("Enter a number: ");

	cout << "You entered: " << number << endl;
	if (isLeapYear(number)) {
		cout << number << " is a leap year." << endl;
	} else {
		cout << number << " is not a leap year." << endl;
	}

	return 0;
}
/*ahmedsabrywork16102009 ahmedsabrywork16102009,  1 week ago (November 20, 2026 at 12:34 AM)

Refactor bank management system to include user management features

Removed Users.txt file and replaced it with a new version containing user data.

Updated bank.cpp to implement user management functionalities including adding, deleting, updating, and finding users.

Introduced new structures for user data and modified existing functions to handle user-related operations.

Enhanced the main menu to include options for managing users.

Added functionality to save and load user data from Users.txt.

Created new functions for printing user data and managing user permissions.

Implemented transaction handling for clients with improved user experience.

5 files changed, 690 insertions(+), 394 deletions(-)

 67e4915   |   Open on GitHub*/