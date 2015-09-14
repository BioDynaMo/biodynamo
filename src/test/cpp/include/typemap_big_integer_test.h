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

#ifndef TYPEMAP_BIG_INTEGER_TEST_H_
#define TYPEMAP_BIG_INTEGER_TEST_H_

#include <string>
#include <iostream>
#include "gmpxx.h"

namespace cx3d {

using BigInteger = mpz_class;

class BigIntegerConsumer {
 public:
  static BigInteger createBigInt(std::string string) {
    BigInteger big_int;
    big_int.set_str(string.c_str(), 10);
    return big_int;
  }

  static std::string getString(const BigInteger& big_int) {
    char* c_str = mpz_get_str(NULL, 10, big_int.get_mpz_t());
    std::string string(c_str);
    delete c_str;
    return string;
  }
};

}  // namespace cx3d

#endif  // TYPEMAP_BIG_INTEGER_TEST_H_
