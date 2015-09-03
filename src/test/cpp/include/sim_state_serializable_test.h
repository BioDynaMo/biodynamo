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

#ifndef SIM_STATE_SERIALIZABLE_TEST_H_
#define SIM_STATE_SERIALIZABLE_TEST_H_

#include "sim_state_serializable.h"
#include "string_builder.h"

namespace cx3d {

using cx3d::SimStateSerializable;
using cx3d::StringBuilder;

class SimulationObject : public SimStateSerializable {
 public:
   virtual StringBuilder& simStateToJson(StringBuilder& sb) override {
     sb.append("Hello World from CPP defined SimulationObject!");
   }
};

class SimStateSerializerConsumer{
 public:
    SimStateSerializerConsumer() : string_builder_() {}
    virtual ~SimStateSerializerConsumer() {}

    StringBuilder& persistSimState(SimStateSerializable& serializable){
      return serializable.simStateToJson(string_builder_);
    }
 private:
   StringBuilder string_builder_;
};

}  // namespace cx3d

#endif  // SIM_STATE_SERIALIZABLE_TEST_H_
