#ifndef SIMULATION_SCHEDULER_H_
#define SIMULATION_SCHEDULER_H_

#include <memory>
#include "simulation/ecm.h"

namespace cx3d {
namespace simulation {

/**
 * This class contains static methods to loop through all the "runnable" CX3D objects
 * stored in a list in ECM to call their run() method. As far as physics is concerned,
 * the call to the run() method is only made if the object has some of it's state variables
 * being modified, or is in a situation where it might occur.
 */
class Scheduler {
 public:
  static Scheduler* getInstance();

  ~Scheduler();
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  /** Runs all the CX3D runnable objects for one time step.*/
  void simulateOneStep();

  /** Runs the simulation, i.e. runs each active CX3D runnable objects endlessly.*/
  void simulate();

  /** Runs the simulation for a given number of time steps, i.e. runs each active CX3D
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
};

}  // namespace simulation
}  // namespace cx3d

#endif // SIMULATION_SCHEDULER_H_
