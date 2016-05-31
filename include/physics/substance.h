#ifndef PHYSICS_SUBSTANCE_H_
#define PHYSICS_SUBSTANCE_H_

#include <string>
#include <memory>

#include "color.h"
#include "sim_state_serializable.h"

namespace bdm {
namespace physics {

class Substance : public SimStateSerializable {
 public:
  using UPtr = std::unique_ptr<Substance>;

  friend struct SubstanceHash;
  friend struct SubstanceEquals;

  Substance();

  Substance(const Substance& other);

  Substance(const std::string& id, Color color);

  Substance(const std::string& id, double diffusion_constant, double degradation_constant);

  /**
   * This function is called after the simulation has finished to serialize the
   * simulation outcome to Json.
   * @param sb Append Json to this StringBuilder
   * @return The received StringBuilder to enable function concatenation
   */
  virtual StringBuilder& simStateToJson(StringBuilder& sb) const;

  /**
   * Increases or decreases the quantity. Makes sure the quantity is never negative.
   * @param delta_q
   */
  void changeQuantityFrom(double delta_q);

  /**
   * Well, as the name says, it multiplies the quantity and the concentration
   * by a certain value. This method is mainly used for degradation .
   */
  void multiplyQuantityAndConcentrationBy(double factor);

  /**
   * Computes the quantity represented when the current concentration is maintained
   * but the volume changes. Important when PhysicalNodes are moved.
   * @param volume
   */
  void updateQuantityBasedOnConcentration(double volume);

  /**
   * Computes the new concentration if the current quantity is distributed in a given volume.
   *  Important when PhysicalNodes are moved.
   * @param volume
   */
  void updateConcentrationBasedOnQuantity(double volume);

  /**
   * Determines whether an other object is equal to this Substance.
   * <br>The result is <code>true</code> if and only if the argument
   * is not null and is a Substance object with the same id, color,
   * degradationConstant and diffusionConstant. The
   * quantity and concentration are note taken into account.
   */
  bool equalTo(Substance* o);

  // --------- GETTERS & SETTERS--------------------------------------------------------
  std::string getId() const;

  void setId(const std::string& id);

  double getDiffusionConstant() const;

  void setDiffusionConstant(double diffusion_constant);

  double getDegradationConstant() const;

  void setDegradationConstant(double degradation_constant);

  Color getColor() const;

  void setColor(Color color);

  double getConcentration() const;

  void setConcentration(double concentration);

  double getQuantity() const;

  void setQuantity(double quantity);

  virtual UPtr getCopy() const;

  std::string toString() const;

 protected:
  /**
   * name of the substance
   */
  std::string id_;

  /**
   * determines how fast it is diffused by the methods in the PhysicalNode
   */
  double diffusion_constant_;

  /**
   * determines how rapidly it is degraded by the PhysicalNode
   */
  double degradation_constant_;

  /**
   *  The color used to represent it if painted
   */
  Color color_;

  /**
   * The total amount present at a given PhysicalNode, or inside a PhysicalObject
   */
  double quantity_;

  /**
   * Concentration = (quantity) / (volume of the PhysicalNode or PhysicalObject)
   */
  double concentration_;

 private:
  Substance& operator=(const Substance&) = delete;
};

struct SubstanceHash {
  std::size_t operator()(Substance* element) const {
    return reinterpret_cast<std::size_t>(element);
  }
};

struct SubstanceEqual {
  bool operator()(Substance* lhs, Substance* rhs) const {
    return lhs == rhs;
  }
};

}  // namespace physics
}  // namespace bdm

#endif // PHYSICS_SUBSTANCE_H_
