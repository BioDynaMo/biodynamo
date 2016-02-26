#ifndef PHYSICS_SUBSTANCE_H_
#define PHYSICS_SUBSTANCE_H_

#include <string>
#include <memory>

#include "sim_state_serializable.h"

namespace cx3d {
namespace physics {

/**
 * C++ representation of java.awt.Color
 * alpha component in bits 24-31
 * red component in bits   16-23
 * green component in bits  8-15
 * blue component in bits   0- 7
 */
class Color {
 public:
  Color(unsigned value) {
    value_ = value;
  }

  bool operator==(const Color& other) const {
    return value_ == other.value_;
  }

  unsigned getValue() const {
    return value_;
  }

 private:
  unsigned value_;
};

class Substance : public SimStateSerializable {
 public:
  Substance();

  static std::shared_ptr<Substance> create() {
    return std::shared_ptr<Substance>(new Substance());
  }

  static std::shared_ptr<Substance> create(const std::shared_ptr<Substance>& other) {
    return std::shared_ptr<Substance>(new Substance(*other));
  }

  static std::shared_ptr<Substance> create(const std::string& id, double diffusion_constant,
                                           double degradation_constant) {
    return std::shared_ptr<Substance>(new Substance(id, diffusion_constant, degradation_constant));
  }

  static std::shared_ptr<Substance> create(const std::string& id, Color color) {
    return std::shared_ptr<Substance>(new Substance(id, color));
  }

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
  virtual void changeQuantityFrom(double delta_q);

  /**
   * Well, as the name says, it multiplies the quantity and the concentration
   * by a certain value. This method is mainly used for degradation .
   */
  virtual void multiplyQuantityAndConcentrationBy(double factor);

  /**
   * Computes the quantity represented when the current concentration is maintained
   * but the volume changes. Important when PhysicalNodes are moved.
   * @param volume
   */
  virtual void updateQuantityBasedOnConcentration(double volume);

  /**
   * Computes the new concentration if the current quantity is distributed in a given volume.
   *  Important when PhysicalNodes are moved.
   * @param volume
   */
  virtual void updateConcentrationBasedOnQuantity(double volume);

  /**
   * Determines whether an other object is equal to this Substance.
   * <br>The result is <code>true</code> if and only if the argument
   * is not null and is a Substance object with the same id, color,
   * degradationConstant and diffusionConstant. The
   * quantity and concentration are note taken into account.
   */
  virtual bool equalTo(const std::shared_ptr<Substance>& o);

  /**
   * Returns the color scaled by the concentration. Useful for painting PhysicalObjects / PhysicalNode
   * based on their Substance concentrations.
   * @return scaled Color
   */
  virtual Color getConcentrationDependentColor() const;

  // --------- GETTERS & SETTERS--------------------------------------------------------
  virtual std::string getId() const;

  virtual void setId(const std::string& id);

  virtual double getDiffusionConstant() const;

  virtual void setDiffusionConstant(double diffusion_constant);

  virtual double getDegradationConstant() const;

  virtual void setDegradationConstant(double degradation_constant);

  virtual Color getColor() const;

  virtual void setColor(Color color);

  virtual double getConcentration() const;

  virtual void setConcentration(double concentration);

  virtual double getQuantity() const;

  virtual void setQuantity(double quantity);

  virtual std::shared_ptr<Substance> getCopy() const;

  virtual std::string toString() const;

 protected:
  Substance(const Substance& other);
  Substance(const std::string& id, double diffusion_constant, double degradation_constant);
  Substance(const std::string& id, Color color);

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

}  // namespace physics
}  // namespace cx3d

#endif // PHYSICS_SUBSTANCE_H_
