#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <cfenv>
using namespace std;

string doubleToHexStr(double d) {
  union {
    long long i;
    double    d;
  } value;
  value.d  = d;

  stringstream stream;
  stream << std::setw(16) << std::hex << std::setfill('0') << value.i;
  return stream.str();
}

int main() {
  cout.precision(numeric_limits<double>::max_digits10);
  errno=0; feclearexcept(FE_ALL_EXCEPT);

  double x = -0.0028724514195400627;
  double result = exp(x);
  cout << result << endl;
  cout << doubleToHexStr(result) << endl;

  if(errno != 0) {
    cout << "The following error occured: " << strerror(errno)  << endl;
  }
	return 0;
}
