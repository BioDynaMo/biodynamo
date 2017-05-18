#ifndef DEBUG_H_
#define DEBUG_H_

#include <algorithm>
#include <array>
#include <iostream>

void Print(const std::array<double, 3>& a) {
  std::cout << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
}

#endif  // DEBUG_H_
