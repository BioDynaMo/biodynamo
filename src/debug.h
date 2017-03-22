#ifndef DEBUG_H_
#define DEBUG_H_

#include <Eigen/CXX11/Tensor>
#include <Vc/Vc>
#include <algorithm>
#include <array>
#include <iostream>

void print(const Eigen::Tensor<float, 1>& tensor) {
  for (size_t i = 0; i < tensor.dimension(0); i++) {
    std::cout << tensor(i) << ", ";
  }
  std::cout << std::endl << std::endl;
}

void print2(const Eigen::Tensor<float, 2>& tensor) {
  for (size_t i = 0; i < tensor.dimension(0); i++) {
    for (size_t j = 0; j < tensor.dimension(1); j++) {
      std::cout << tensor(i, j) << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

template <int64_t N>
void print(const Eigen::TensorFixedSize<float, Eigen::Sizes<N>>& tensor) {
  for (size_t i = 0; i < N; i++) {
    std::cout << tensor(i) << ", ";
  }
  std::cout << std::endl << std::endl;
}

template <int64_t R, int64_t C>
void print(const Eigen::TensorFixedSize<float, Eigen::Sizes<R, C>>& tensor) {
  for (size_t i = 0; i < R; i++) {
    for (size_t j = 0; j < C; j++) {
      std::cout << tensor(i, j) << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

template <int64_t N>
void print(const std::array<Eigen::TensorFixedSize<float, Eigen::Sizes<N>>, 3>&
               matrix) {
  for (size_t i = 0; i < 3; i++) {
    print(matrix[i]);
  }
  std::cout << std::endl << std::endl;
}

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
