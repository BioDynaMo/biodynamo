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

#ifndef SIM_STATE_SERIALIZABLE_H_
#define SIM_STATE_SERIALIZABLE_H_

#include "string_builder.h"

namespace cx3d {

using cx3d::StringBuilder;

/**
 * Classes that implement that interface serialize their simulation state to
 * json with as little implementation details as possible (e.g. state of locks
 * or which collection implementation has been used)
 */
class SimStateSerializable {
 public:
  virtual ~SimStateSerializable() {
  }

  /**
   * This function is called after the simulation has finished to serialize the
   * simulation outcome to Json.
   * @param sb Append Json to this StringBuilder
   * @return The received StringBuilder to enable function concatenation
   */
  virtual StringBuilder& simStateToJson(StringBuilder& sb) const = 0;
};

}  // namespace cx3d

#endif  // SIM_STATE_SERIALIZABLE_H_
