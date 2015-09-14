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

#ifndef STRING_BUILDER_H_
#define STRING_BUILDER_H_

#include <string>
#include <sstream>

namespace cx3d {

using std::string;
using std::stringstream;

/**
 * This class is used as an equivalent to java.lang.StringBuilder.
 * It is mainly used to generate the json representation of the simulation state
 */
class StringBuilder {
 public:
  StringBuilder()
      : string_stream_() {
  }
  StringBuilder(const StringBuilder &src)
      : string_stream_() {
    string_stream_ << src.str();
  }
  virtual ~StringBuilder() {
  }

  /**
   * Appends the given string to the current string builder
   * @param str string to append
   * @return returns itself
   */
  virtual StringBuilder& append(const string &str) {
    string_stream_ << str;
    return *this;
  }

  /**
   * Sets the pointer to the second last character.
   * A consecutive call of append will overwrite the last char
   */
  virtual void overwriteLastCharOnNextAppend() {
    string_stream_.seekp(-1, string_stream_.cur);
  }

  /**
   *  Returns a string object with a copy of the current contents of the stream.
   */
  virtual string str() const {
    return string_stream_.str();
  }

 private:
  stringstream string_stream_;

  StringBuilder &operator=(const StringBuilder &) = delete;
};

}  // namespace cx3d

#endif  // STRING_BUILDER_H_
