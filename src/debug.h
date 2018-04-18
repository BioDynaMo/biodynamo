#ifndef DEBUG_H_
#define DEBUG_H_

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>

inline void Print(const std::array<double, 3>& a, int precision = 10) {
  std::cout << std::setprecision(precision) << a[0] << ", " << a[1] << ", "
            << a[2] << std::endl;
}

#endif  // DEBUG_H_
