#ifndef CORE_VIRTUALIZATION_VIRTUALIZATION_ADAPTOR_H_
#define CORE_VIRTUALIZATION_VIRTUALIZATION_ADAPTOR_H_

namespace bdm {

class VisualizationAdaptor {
 public:
  virtual ~VisualizationAdaptor() {}
  virtual void Visualize() = 0;
};

}  // namespace bdm

#endif  // CORE_VIRTUALIZATION_VIRTUALIZATION_ADAPTOR_H_
