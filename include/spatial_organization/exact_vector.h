#ifndef SPATIAL_ORGANIZATION_EXACT_VECTOR_H_
#define SPATIAL_ORGANIZATION_EXACT_VECTOR_H_

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <sstream>

namespace bdm {
namespace spatial_organization {

/**
 * This class is used to store vectors in 3D-space. It also provides basic
 * linear algebra functions. All functions are performed using precise arithmetics by
 * using instances of double to store the entries and to perform the calculations.
 *
 * This class is exclusively designed to store 3 elements per vector.
 *
 * Note: function argument nullptr checks have been omitted, as this class is only used internally
 */
class ExactVector : public std::enable_shared_from_this<ExactVector> {
 public:
 /**
  * Creates a new ExactVector object and returns it within a <code>std::shared_ptr</code>
  */
  static std::shared_ptr<ExactVector> create(const std::array<double, 3>& values);

  /**
   * Computes the determinant of a 3x3 matrix.
   * @param c An std::array of size 3 which contains the column vectors of the matrix.
   * @return A double containing the determinant of the specified matrix.
   */
  static double det(const std::array<std::shared_ptr<ExactVector>, 3>& c);

  virtual ~ExactVector();

  /**
   * Computes the square of the length of this vector.
   * @return The square of the Euclidean length of this vector.
   */
  double squaredLength() const;

  /**
   * Computes the sum of this vector with another vector and returns a new instance of <code>ExactVector</code>
   * containing the result.
   * @param other The second argument of the addition.
   * @return A new instance of <code>ExactVector</code>.
   */
  std::shared_ptr<ExactVector> add(const std::shared_ptr<ExactVector>& other) const;

  /**
   * Adds another vector to this vector.
   * @param other The vector by which this vector should be increased.
   * @return A reference to <code>this</code>, which has been modified.
   */
  std::shared_ptr<ExactVector> increaseBy(const std::shared_ptr<ExactVector>& other);

  /**
   * Computes the result of this vector minus another vector and returns a new instance of <code>ExactVector</code>
   * containing the result.
   * @param other The second argument of the subtraction.
   * @return A new instance of <code>ExactVector</code>.
   */
  std::shared_ptr<ExactVector> subtract(const std::shared_ptr<ExactVector>& other);

  /**
   * Decreases this vector by another vector.
   * @param other The vector by which this vector should be decreased.
   * @return A reference to <code>this</code>, which has been modified.
   */
  std::shared_ptr<ExactVector> decreaseBy(const std::shared_ptr<ExactVector>& other);

  /**
   * Computes the product of this vector with a double number and returns a new instance of <code>ExactVector</code>
   * containing the result. This vector itself remains unchanged.
   * @param factor The constant by which all entries of this vector should be multiplied.
   * @return A new instance of <code>ExactVector</code>.
   */
  std::shared_ptr<ExactVector> multiply(double factor);

  /**
   * Multiplies this vector with a double number. The result is stored in this vector itself.
   * @param factor The constant by which all entries of this vector should be multiplied.
   * @return A reference to <code>this</code>, which has been modified.
   */
  std::shared_ptr<ExactVector> multiplyBy(double factor);

  /**
   * Computes the division of this vector with a double number and returns a new instance of <code>ExactVector</code>
   * containing the result. This vector itself remains unchanged.
   * @param factor The constant by which all entries of this vector should be divided.
   * @return A new instance of <code>ExactVector</code>.
   */
  std::shared_ptr<ExactVector> divide(double factor);

  /**
   * Divides this vector by a double number. The result is stored in this vector itself.
   * @param factor The constant by which all entries of this vector should be divided.
   * @return A reference to <code>this</code>, which has been modified.
   */
  std::shared_ptr<ExactVector> divideBy(double factor);

  /**
   * Computes the dot product of this vector with another one. The result is stored in a new instance of <code>double</code>.
   * @param other The other argument of this dot product.
   * @return A double.
   */
  double dotProduct(const std::shared_ptr<ExactVector>& other);

  /**
   * Multiplies this vector by -1. This vector itself is modified during that process.
   * @return A reference to this vector itself.
   */
  std::shared_ptr<ExactVector> negate();

  /**
   * Returns the cross-product of this vector and another.
   *
   * @param  other The vector with which the cross-product should be calculated
   * @return The cross-product of this vector and <code>other<\code>, stored in a new instance of <code>ExactVector</code>
   */
  std::shared_ptr<ExactVector> crossProduct(const std::shared_ptr<ExactVector>& other);

  /**
   * Returns a string representation of this object.
   */
  std::string toString();

  /**
   * Determines if two instances of this object are equal
   */
  bool equalTo(const std::shared_ptr<ExactVector>& other);

 private:
  /**
   *  Stores the elements of this vector.
   */
  std::array<double, 3> elements_;

  ExactVector() = delete;
  ExactVector(const ExactVector&) = delete;
  ExactVector& operator=(const ExactVector&) = delete;

  /**
   * Creates a new vector from an std::array of double numbers.
   * @param values The entries for this vector. The length of this std::array is expected to be 3.
   */
  explicit ExactVector(const std::array<double, 3>& values);
};

}  // namespace spatial_organizationd
}  // namespace bdm

#endif  // SPATIAL_ORGANIZATION_EXACT_VECTOR_H_
