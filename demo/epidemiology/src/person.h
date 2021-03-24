// -----------------------------------------------------------------------------
//
// Copyright (C) Lukas Breitwieser.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef PERSON_H_
#define PERSON_H_

#include "core/agent/cell.h"

namespace bdm {

/// Possible Person states.
enum State { kSusceptible, kInfected, kRecovered };

class Person : public Cell {
  BDM_AGENT_HEADER(Person, Cell, 1);

 public:
  Person() {}
  explicit Person(const Double3& position) : Base(position) {}
  virtual ~Person() {}

  /// This data member stores the current state of the person.
  int state_ = State::kSusceptible;
};

}  // namespace bdm

#endif  // PERSON_H_
