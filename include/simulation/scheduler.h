#ifndef SIMULATION_SCHEDULER_H_
#define SIMULATION_SCHEDULER_H_

#include <memory>
#include "simulation/ecm.h"

namespace bdm {
namespace simulation {

/**
 * This class contains static methods to loop through all the "runnable" BioDynaMo objects
 * stored in a list in ECM to call their run() method. As far as physics is concerned,
 * the call to the run() method is only made if the object has some of it's state variables
 * being modified, or is in a situation where it might occur.
 */
class Scheduler {
 public:
  static Scheduler* getInstance();

  virtual ~Scheduler();
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  /** Runs all the BioDynaMo runnable objects for one time step.*/
  void simulateOneStep();

  /** Runs the simulation, i.e. runs each active BioDynaMo runnable objects endlessly.*/
  void simulate();

  /** Runs the simulation for a given number of time steps, i.e. runs each active BioDynaMo
   * runnable objects.
   * @param steps nb of steps that the simulation is run.
   */
  void simulateThatManyTimeSteps(int steps);

  void setPrintCurrentECMTime(bool print_time);

 private:
  Scheduler();

  /** Reference to the ECM.*/
  static ECM* ecm_;

  /** static counter, needed in case where we want to make regular snapshots.*/
  int cycle_counter_ = 0;

  /** if false, the physics is not computed......*/
  bool run_physics_ = true;
  bool run_diffusion_ = true;

  bool print_current_ecm_time_ = false;
  bool print_current_step_ = false;

  ClassDef(Scheduler, 1);
};

}  // namespace simulation
}  // namespace bdm

#endif // SIMULATION_SCHEDULER_H_
