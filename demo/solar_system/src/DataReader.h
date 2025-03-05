// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------
#ifndef DATAREADER_H_
#define DATAREADER_H_

#include <biodynamo.h>

namespace bdm {
namespace astrophysics {

// Define the structure to hold each object's data
struct ObjectData {
  std::string name;
  real_t mass;
  real_t diameter;
  real_t orbital_velocity;
  real_t distance_from_parent;
  real_t angle;

  void ScaleDistance(real_t scale_factor) {
    diameter *= scale_factor;
    distance_from_parent *= scale_factor;
    orbital_velocity *= scale_factor;
  }
};

inline void LoadDataFromFile(const std::string& filename,
                             std::vector<ObjectData>& data) {
  std::ifstream file(filename);
  std::string line;
  std::getline(file, line);  // Skip the header line

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    ObjectData obj;
    std::string name;
    double mass;
    double diameter;
    double orbital_velocity;
    double distance_from_parent;
    double angle;

    if (iss >> name >> mass >> diameter >> orbital_velocity >>
        distance_from_parent >> angle) {
      obj.name = name;
      obj.mass = mass;
      obj.diameter = diameter;
      obj.orbital_velocity = orbital_velocity;
      obj.distance_from_parent = distance_from_parent;
      obj.angle = angle;
      data.push_back(obj);
    }
  }
}

}  // namespace astrophysics
}  // namespace bdm

#endif  // NASADATA_H_
