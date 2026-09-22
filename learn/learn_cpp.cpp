#include "../libraries/cpp_libraries/enumeration.h"
#include "../libraries/cpp_libraries/input.h"
#include "../libraries/cpp_libraries/output.h"
#include "../libraries/cpp_libraries/process.h"

#include <cstddef>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace std;
using namespace AS;

int main() {

  // stPeriod period = readPeriod();
  // stPeriod period2 = readPeriod();

  string dateString = readDateString();

  stDate date = convertToDate(dateString);

  cout << dateToString(date) << "\n";

  return 0;
}