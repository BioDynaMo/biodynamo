#ifndef SYNAPSE_EXCRESCENCE_H_
#define SYNAPSE_EXCRESCENCE_H_

#include <array>
#include <memory>

#include "sim_state_serializable.h"

namespace bdm {

namespace local_biology {
class NeuriteElement;
}  // namespace local_biology

namespace physics {
class PhysicalObject;
}  // namespace physics

namespace simulation {
class ECM;
}  // namespace simulation

namespace synapse {

using local_biology::NeuriteElement;
using physics::PhysicalObject;
using simulation::ECM;

/**
 * General class for dendritic spines and axonal boutons
 * This class doesn't derive from PhysicalObject. It has thus no SpaceNode.
 */
class Excrescence : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<Excrescence>;

  enum Type {
    kSpine = 0,
    kBouton = 1,
    kSomaticSpine = 2,
    kShaft = 3
  };

  Excrescence(TRootIOCtor*) { }  // only used for ROOT I/O

  Excrescence();

  Excrescence(Excrescence::Type type);

  Excrescence(PhysicalObject* po, const std::array<double, 2>& origin, double length, Excrescence::Type type);

  virtual ~Excrescence();

  virtual StringBuilder& simStateToJson(StringBuilder& sb) const override;

  /** returns the absolute coord of the point where this element is attached on the PO.*/
  std::array<double, 3> getProximalEnd() const;

  /** returns the absolute coord of the point where this element ends.*/
  std::array<double, 3> getDistalEnd() const;

  virtual bool synapseWith(Excrescence* other, bool create_physical_bond) = 0;

  virtual bool synapseWithSoma(Excrescence* other_excrescence, bool create_phyiscal_bond) = 0;

  virtual bool synapseWithShaft(NeuriteElement* other_ne, double max_dis, int nr_segments,
                                bool create_phyiscal_bond) = 0;

  // getters and setters
  Excrescence* getEx() const;

  void setEx(Excrescence* ex);

  double getLength() const;

  void setLength(double length);

  PhysicalObject* getPo() const;

  void setPo(PhysicalObject* po);

  std::array<double, 2> getPositionOnPO() const;

  void setPositionOnPO(const std::array<double, 2>& position);

  Type getType() const;

  void setType(Type type);

 protected:
  static ECM* ecm_;

  /** the physical object it is attached to.*/
  PhysicalObject* po_ = nullptr;

  /** the other structure with which it forms a synapse.*/
  Excrescence* ex_ = nullptr;

  /** The position in polar coordinates on the Physical Object where it's origin is.*/
  std::array<double, 2> position_on_po_;

  double length_ = 1.0;

  /** spine or bouton*/
  Excrescence::Type type_ = Type::kSpine;

  /** The neurite Element to which the shaft synapse is made */
  NeuriteElement* ne_shaft_ = nullptr;

 private:
  Excrescence(const Excrescence&) = delete;
  Excrescence& operator=(const Excrescence&) = delete;

  ClassDefOverride(Excrescence, 1);
};

}  // namespace synapse
}  // namespace bdm

#endif  // SYNAPSE_EXCRESCENCE_H_
