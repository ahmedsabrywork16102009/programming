#include "../libraries/cpp_libraries/input.h"
#include "../libraries/cpp_libraries/output.h"
#include "../libraries/cpp_libraries/process.h"

#include <iostream>

using namespace std;
using namespace AS;

int main() {
    int number1 = readNumber<int>("Enter number1: ");
    int number2 = readNumber<int>("Enter number2: ");
    int number3 = readNumber<int>("Enter number3: ");

    cout << "\n";
    cout << "Sum: " << sumRangeWithStep(number1, number2, number3) << "\n";
    cout << "Product: " << productRangeWithStep(number1, number2, number3) << "\n";
    cout << "Divide: " << divideRangeWithStep(number1, number2, number3) << "\n";
    cout << "Power: " << powerRangeWithStep(number1, number2, number3) << "\n";

    return 0;
}