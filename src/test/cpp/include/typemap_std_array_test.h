/*
 Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
 Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
 Sabina Pfister, Adrian M. Whatley & Lukas Breitwieser.

 This file is part of CX3D.

 CX3D is free software: you can redistribute it and/or modify
 it under the terms of the GNU General virtual License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 CX3D is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General virtual License for more details.

 You should have received a copy of the GNU General virtual License
 along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TYPEMAP_STD_ARRAY_TEST_H_
#define TYPEMAP_STD_ARRAY_TEST_H_

#include <array>
#include <cmath>

namespace cx3d {

using std::abs;

class ArrayUtil {
 private:
  std::array<double, 3> content_;
  std::array<std::array<double, 3>, 2> two_dim_content_;

 public:
  ArrayUtil()
      : content_ { 98, 97, 96 },
        two_dim_content_ { { { 95, 94, 93 }, { 92, 91, 90 } } } {
  }

  std::array<double, 3> const getContent() {
    return content_;
  }

  double l1Norm(const std::array<double, 3>& array) {
    double ret = 0;
    for (double el : array) {
      ret += abs(el);
    }
    return ret;
  }

  void scalarAddition(std::array<double, 3>& array, double scalar) {
    for (size_t i = 0; i < array.size(); i++) {
      array[i] += scalar;
    }
  }

  std::array<std::array<double, 3>, 2> const getTwoDimContent() {
    return two_dim_content_;
  }

  double l1Norm(const std::array<std::array<double, 3>, 2>& array) {
    double ret = 0;
    for (auto arr : array) {
      for (double el : arr) {
        ret += abs(el);
      }
    }
    return ret;
  }

  void scalarAddition(std::array<std::array<double, 3>, 2>& array, double scalar) {
    for (size_t i = 0; i < array.size(); i++) {
      for (size_t j = 0; j < array[i].size(); j++) {
        array[i][j] += scalar;
      }
    }
  }
};

}  // namespace cx3d

#endif  // TYPEMAP_STD_ARRAY_TEST_H_
