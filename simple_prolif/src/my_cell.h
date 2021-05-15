// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef MY_CELL_H_
#define MY_CELL_H_

#include <biodynamo.h>
#include "core/substance_initializers.h"

namespace bdm {

class MyCell : public Cell {
// MyCell extends Cell by inheritance
//BDM_SIM_OBJECT_HEADER(MyCell, Cell, 1, phenotype_, age_);
BDM_AGENT_HEADER(MyCell, Cell, 1);

public:
  MyCell() {}

  explicit MyCell(const Double3& position, int cell_type, int age, std::string guidance_cue)
      : Base(position), cell_type_(cell_type), age_(age), guidance_cue_(guidance_cue) {}
  //explicit MyCell(const Double3& position, int cell_type, int age)
  //    : Base(position), cell_type_(cell_type), age_(age) {}
  virtual ~MyCell() {}


  //explicit MyCell(const Double3& position) : Base(position) { age_++; }
  //
  /// If MyCell divides, daughter 2 copies the data members from the mother
  //MyCell (const Event& event, SimObject* other, uint64_t new_oid = 0)
  //    : Base(event, other, new_oid) {
  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);
    if (auto* mother = dynamic_cast<MyCell*>(event.existing_agent)) {
      if (event.GetUid() == CellDivisionEvent::kUid) {
      //if (event.GetId() == CellDivisionEvent::kEventId) {
        //phenotype_ = 1 + mother->phenotype_;
        // age cannot be inherited

        auto* random = Simulation::GetActive()->GetRandom();


        if (mother->cell_type_==0) {
          if (random->Uniform(0, 1) < GetP1()) {
            cell_type_=1;
          }
          else {
            //cell_type_ = mother->cell_type_;
            cell_type_ =2;
          }
        }

        if (mother->cell_type_==2) {
          if (random->Uniform(0, 1) < GetP2()) {
            cell_type_=3;
          }
          else {
            //cell_type_ = mother->cell_type_;
            cell_type_ =2;
          }
        }

        age_ = mother->age_;
      }
    }
  }
  //
  // template <typename TDaughter>
  // void EventHandler (const CellDivisionEvent& event, TDaughter* daughter) {
  //   Base::EventHandler(event, daughter);
  // }
  // //
  // void SetPhenotype(int pt) { phenotype_ = pt; }
  // int GetPhenotype() const { return phenotype_; }
  // //
  int GetCellType() const { return cell_type_; }
  void SetCellType(int cell_type) {cell_type_ = cell_type; }

  void IncrementAge() { age_++; }
  int GetAge() const { return age_; }



  std::string GetGuidanceCue() const { return guidance_cue_; }

  double GetP1() const { return p1_; }
  double GetP2() const { return p2_; }

private:
  int cell_type_;
  //int phenotype_ = 0;
  int age_;
  double p1_=0.5;
  double p2_=0.5;
  std::string guidance_cue_;
};

}  // namespace bdm

#endif  // MY_CELL_H_
