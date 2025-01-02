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

    std::string Name;
    real_t mass;
    real_t diameter;
    real_t orbitalVelocity;
    real_t distanceFromParent;
    real_t angle;
    
    void scaleDistance(real_t scale_factor){
        diameter *= scale_factor;
        distanceFromParent *= scale_factor;
        orbitalVelocity*= scale_factor;
        }
};

inline void loadDataFromFile(const std::string& filename, std::vector<ObjectData>& data) {
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);  // Skip the header line

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        ObjectData obj;
        std::string name;
        double mass;
        double diameter;
        double orbitalVelocity;
        double distanceFromParent;
        double angle;

        if (iss >> name >> mass >> diameter >> orbitalVelocity >> distanceFromParent >> angle) {
            obj.Name = name;
            obj.mass = mass;
            obj.diameter = diameter;
            obj.orbitalVelocity = orbitalVelocity;
            obj.distanceFromParent = distanceFromParent;
            obj.angle = angle;
            data.push_back(obj);
        }
    }
}



}  // namespace astrophysics
}  // namespace bdm

#endif  // NASADATA_H_
