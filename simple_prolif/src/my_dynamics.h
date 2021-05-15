
#ifndef MY_DYNAMICS_H_
#define MY_DYNAMICS_H_

#include "biodynamo.h"


#include <iostream>
//using namespace std;

namespace bdm {

  // Define growth behaviour
  struct MyGrowth : public Behavior {
    BDM_BEHAVIOR_HEADER(MyGrowth, Behavior, 1);

    MyGrowth() { AlwaysCopyToNew(); }
    virtual ~MyGrowth() {}

    //int growth_counter=0
    int diffstate_switch_counter=500;
    int currAge;
    double speed_=2.0;

    DiffusionGrid* dgrid_ = Simulation::GetActive()->GetResourceManager()->GetDiffusionGrid(
      "GuidanceCue");

      void Run(Agent* agent) override {
        if (auto* cell = dynamic_cast<MyCell*>(agent)) {
          cell->IncrementAge();
          currAge = cell->GetAge();


          if (currAge<diffstate_switch_counter) {
            if (cell->GetDiameter()<15) {
              cell->ChangeVolume(700);

            } else {
              auto* random = Simulation::GetActive()->GetRandom();
              //if (random->Uniform(0, 1) < 1) {
              cell->Divide();
              //}
            }
          }

          //else (currAge>diffstate_switch_counter) {
          //else {

          if (cell->GetCellType()==1) {
            if (currAge>diffstate_switch_counter) {
              auto& position = cell->GetPosition();

              double currConc = dgrid_->GetConcentration(position);

              std::cout << "conc: " << currConc << " " << std::endl;

              if (currConc>0.01) {
                std::cout << "stopped cell" << std::endl;
                cell->RemoveBehavior(this);
              }
              else {
                Double3 gradient;
                dgrid_->GetGradient(position, &gradient);  // returns normalized gradient
                #pragma omp critical
                std::cout << "an age from a cell: " << cell->GetAge() << std::endl;
                std::cout << "grad: " << gradient[0] << "/" << gradient[1] << "/" << gradient[2] << std::endl;
                cell->UpdatePosition(gradient * speed_);
              }

            }
          }


          if (cell->GetCellType()==3) {
            if (currAge>diffstate_switch_counter) {
              auto& position = cell->GetPosition();

              double currConc = dgrid_->GetConcentration(position);

              std::cout << "conc: " << currConc << " " << std::endl;

              if (currConc>0.005) {
                std::cout << "stopped cell" << std::endl;
                cell->RemoveBehavior(this);
              }
              else {
                Double3 gradient;
                dgrid_->GetGradient(position, &gradient);  // returns normalized gradient
                #pragma omp critical
                std::cout << "an age from a cell: " << cell->GetAge() << std::endl;
                std::cout << "grad: " << gradient[0] << "/" << gradient[1] << "/" << gradient[2] << std::endl;
                cell->UpdatePosition(gradient * speed_);
              }

            }
          }

        }
      }

      // void Run(Agent* agent) override {
      //   if (auto* cell = dynamic_cast<MyCell*>(agent)) {
      //
      //
      //
      //
      //     if (cell->GetDiameter() < 8) {
      //       auto* random = Simulation::GetActive()->GetRandom();
      //       // Here 400 is the speed and the change to the volume is based on the
      //       // simulation time step.
      //       // The default here is 0.01 for timestep, not 1.
      //       cell->ChangeVolume(400);
      //
      //       // create an array of 3 random numbers between -2 and 2
      //       Double3 cell_movements = random->template UniformArray<3>(-2, 2);
      //       // update the cell mass location, ie move the cell
      //       cell->UpdatePosition(cell_movements);
      //     } else {  //
      //       auto* random = Simulation::GetActive()->GetRandom();
      //
      //       if (cell->GetCanDivide() && random->Uniform(0, 1) < 0.8) {
      //         cell->Divide();
      //       } else {
      //         cell->SetCanDivide(false);  // this cell won't divide anymore
      //       }
      //     }
      //   }
      // }
    };

    // inline int Simulate(int argc, const char** argv) {
    //   auto set_param = [](Param* param) {
    //     param->bound_space = true;
    //     param->min_bound = 0;
    //     param->max_bound = 100;  // cube of 100*100*100
    //   };
    //
    //   Simulation simulation(argc, argv, set_param);
    //   auto* rm = simulation.GetResourceManager();
    //   auto* param = simulation.GetParam();
    //   auto* myrand = simulation.GetRandom();
    //
    //   size_t nb_of_cells = 2400;  // number of cells in the simulation
    //   double x_coord, y_coord, z_coord;
    //
    //   for (size_t i = 0; i < nb_of_cells; ++i) {
    //     // our modelling will be a cell cube of 100*100*100
    //     // random double between 0 and 100
    //     x_coord = myrand->Uniform(param->min_bound, param->max_bound);
    //     y_coord = myrand->Uniform(param->min_bound, param->max_bound);
    //     z_coord = myrand->Uniform(param->min_bound, param->max_bound);
    //
    //     // creating the cell at position x, y, z
    //     MyCell* cell = new MyCell({x_coord, y_coord, z_coord});
    //     // set cell parameters
    //     cell->SetDiameter(7.5);
    //     // will vary from 0 to 5. so 6 different layers depending on y_coord
    //     cell->SetCellColor(static_cast<int>((y_coord / param->max_bound * 6)));
    //
    //     rm->AddAgent(cell);  // put the created cell in our cells structure
    //   }
    //
    //   // create a cancerous cell, containing the behavior Growth
    //   MyCell* cell = new MyCell({20, 50, 50});
    //   cell->SetDiameter(6);
    //   cell->SetCellColor(8);
    //   cell->SetCanDivide(true);
    //   cell->AddBehavior(new Growth());
    //   rm->AddAgent(cell);  // put the created cell in our cells structure
    //
    //   // Run simulation
    //   simulation.GetScheduler()->Simulate(500);
    //
    //   std::cout << "Simulation completed successfully!" << std::endl;
    //   return 0;
    // }

  }  // namespace bdm

  #endif  // MY_DYNAMICS_H_
