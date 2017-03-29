#ifndef DEBUG_H_
#define DEBUG_H_

#include <algorithm>
#include <array>
#include <iostream>
#include "Vc/Vc"

void print(const std::array<double, 3>& a) {
  std::cout << a[0] << ", " << a[1] << ", " << a[2] << std::endl;
}

void print(const Vc::float_v& v) {
  for (size_t i = 0; i < Vc::float_v::Size; i++) {
    std::cout << v[i] << ", ";
  }
  std::cout << std::endl;
}

void print(const std::array<Vc::float_v, 3>& matrix) {
  print(matrix[0]);
  print(matrix[1]);
  print(matrix[2]);
  std::cout << std::endl;
}

#endif  // DEBUG_H_
