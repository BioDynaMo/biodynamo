#ifndef MY_PARAM_H_
#define MY_PARAM_H_

#include "core/param/module_param.h"
#include "core/util/root.h"
#include "cpptoml/cpptoml.h"

namespace bdm {


struct MyParam : public ModuleParam {
  static const ModuleParamUid kUid;

  /// Amount of timesteps to run the simulation
  int timesteps_ = 300;

  /// The lower limit of the simulation space
  double min_space_ = 300;

  /// The upper limit of the simulation space
  double max_space_ = 300;

  /// Number of T-Cells in the simulation
  int num_t_cells_ = 100;

  /// Diameter of T-Cells
  double t_cell_diameter_ = 5;

  /// NUmber of Monocytes in the simulation
  int num_monocytes_ = 100;

  /// Diameter of Monocytes
  double monocyte_diameter_ = 5;

  /// The mimimum Antibody concentration required for inhibiting Monocytes to bind
  double concentration_threshold_ = .0095;

  /// The probability at which Monocytes are inhibited to bind (if minimum 
  /// concentration is reached)
  double binding_probability_ = .2;

  /// The interaction radius for T-Cells to be able to bind to Monocytes
  double binding_radius_ = 5;

  /// The diffusion rate (coefficient) for the Antibodies
  double diffusion_rate_ = 0.4;

  ModuleParam* GetCopy() const override;

  ModuleParamUid GetUid() const override;

 protected:
  /// Assign values from config file to variables
  void AssignFromConfig(const std::shared_ptr<cpptoml::table>&) override;

 private:
  BDM_CLASS_DEF_OVERRIDE(MyParam, 1);
};

}  // namespace bdm

#endif  // MY_PARAM_H_
