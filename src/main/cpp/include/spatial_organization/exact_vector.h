/*
 Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
 Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
 Sabina Pfister, Adrian M. Whatley & Lukas Breitwieser.

 This file is part of CX3D.

 CX3D is free software: you can redistribute it and/or modify
 it under the terms of the GNU General virtual License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 CX3D is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General virtual License for more details.

 You should have received a copy of the GNU General virtual License
 along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPATIAL_ORGANIZATION_EXACT_VECTOR_H_
#define SPATIAL_ORGANIZATION_EXACT_VECTOR_H_

#include <array>
#include <vector>
#include <memory>

namespace cx3d {
namespace spatial_organization {

class Rational;

/**
 * This class is used to store vectors in 3D-space. It also provides basic
 * linear algebra functions. All functions are performed using precise arithmetics by
 * using instances of {@link Rational} to store the entries and to perform the calculations.
 *
 * This class is exclusively designed to store 3 elements per vector.
 *
 * Note: function argument nullptr checks have been omitted, as this class is only used internally
 */
class ExactVector : public std::enable_shared_from_this<ExactVector> {
 public:
  /**
   * Creates a new ExactVector object and returns it within a <code>std::shared_ptr</code>
   * @see ExactVector(const std::array<std::shared_ptr<Rational>, 3>& values)
   *
   * If functions return a std::shared_ptr of <code>*this</code> using
   * <code>return shared_from_this();</code>, the following precondition must be met:
   * There must be at least one std::shared_ptr p that owns *this!
   * Calling <code>shared_from_this</code> on a non-shared object results in undefined behaviour.
   * http://mortoray.com/2013/08/02/safely-using-enable_shared_from_this/
   *
   * Therefore, all constructors are private and accessed through static factory methods that return
   * a std::shared_ptr.
   *
   * TODO(lukas) SWIG doesn't seem to support variadic templates and perfect forwarding system.
   * Once mapping to Java is not needed anymore, replace following create functions with:
   * <code>
   * template<typename ... T>
   * static std::shared_ptr<ExactVector> create(T&& ... all) {
   *   return std::shared_ptr<ExactVector>(new ExactVector(std::forward<T>(all)...));
   * }
   * </code>
   */
  static std::shared_ptr<ExactVector> create(
      const std::array<std::shared_ptr<Rational>, 3>& values) {
    return std::shared_ptr<ExactVector>(new ExactVector(values));
  }

  /**
   * @see create(const std::array<std::shared_ptr<Rational>, 3>& values)
   * @see ExactVector(const std::array<double, 3>& values)
   */
  static std::shared_ptr<ExactVector> create(const std::array<double, 3>& values) {
    return std::shared_ptr<ExactVector>(new ExactVector(values));
  }

  /**
   * Computes the determinant of a 3x3 matrix.
   * @param c An std::array of size 3 which contains the column vectors of the matrix.
   * @return A new instance of <code>Rational</code> containing the determinant of the specified matrix.
   */
  static std::shared_ptr<Rational> det(const std::array<std::shared_ptr<ExactVector>, 3>& c);

  virtual ~ExactVector();

  /**
   * Computes the square of the length of this vector.
   * @return The square of the Euclidean length of this vector.
   */
  virtual std::shared_ptr<Rational> squaredLength() const;

  /**
   * Computes the sum of this vector with another vector and returns a new instance of <code>ExactVector</code>
   * containing the result.
   * @param other The second argument of the addition.
   * @return A new instance of <code>ExactVector</code>.
   */
  virtual std::shared_ptr<ExactVector> add(const std::shared_ptr<ExactVector>& other) const;

  /**
   * Adds another vector to this vector.
   * @param other The vector by which this vector should be increased.
   * @return A reference to <code>this</code>, which has been modified.
   */
  virtual std::shared_ptr<ExactVector> increaseBy(const std::shared_ptr<ExactVector>& other);

  /**
   * Computes the result of this vector minus another vector and returns a new instance of <code>ExactVector</code>
   * containing the result.
   * @param other The second argument of the subtraction.
   * @return A new instance of <code>ExactVector</code>.
   */
  virtual std::shared_ptr<ExactVector> subtract(const std::shared_ptr<ExactVector>& other);

  /**
   * Decreases this vector by another vector.
   * @param other The vector by which this vector should be decreased.
   * @return A reference to <code>this</code>, which has been modified.
   */
  virtual std::shared_ptr<ExactVector> decreaseBy(const std::shared_ptr<ExactVector>& other);

  /**
   * Computes the product of this vector with a rational number and returns a new instance of <code>ExactVector</code>
   * containing the result. This vector itself remains unchanged.
   * @param factor The constant by which all entries of this vector should be multiplied.
   * @return A new instance of <code>ExactVector</code>.
   */
  virtual std::shared_ptr<ExactVector> multiply(const std::shared_ptr<Rational>& factor);

  /**
   * Multiplies this vector with a rational number. The result is stored in this vector itself.
   * @param factor The constant by which all entries of this vector should be multiplied.
   * @return A reference to <code>this</code>, which has been modified.
   */
  virtual std::shared_ptr<ExactVector> multiplyBy(const std::shared_ptr<Rational>& factor);

  /**
   * Computes the division of this vector with a rational number and returns a new instance of <code>ExactVector</code>
   * containing the result. This vector itself remains unchanged.
   * @param factor The constant by which all entries of this vector should be divided.
   * @return A new instance of <code>ExactVector</code>.
   */
  virtual std::shared_ptr<ExactVector> divide(const std::shared_ptr<Rational>& factor);

  /**
   * Divides this vector by a rational number. The result is stored in this vector itself.
   * @param factor The constant by which all entries of this vector should be divided.
   * @return A reference to <code>this</code>, which has been modified.
   */
  virtual std::shared_ptr<ExactVector> divideBy(const std::shared_ptr<Rational>& factor);

  /**
   * Computes the dot product of this vector with another one. The result is stored in a new instance of <code>Rational</code>.
   * @param other The other argument of this dot product.
   * @return A new instance of <code>Rational</code> containing the computed dot product.
   */
  virtual std::shared_ptr<Rational> dotProduct(const std::shared_ptr<ExactVector>& other);

  /**
   * Multiplies this vector by -1. This vector itself is modified during that process.
   * @return A reference to this vector itself.
   */
  virtual std::shared_ptr<ExactVector> negate();

  /**
   * Returns the cross-product of this vector and another.
   *
   * @param  other The vector with which the cross-product should be calculated
   * @return The cross-product of this vector and <code>other<\code>, stored in a new instance of <code>ExactVector</code>
   */
  virtual std::shared_ptr<ExactVector> crossProduct(const std::shared_ptr<ExactVector>& other);

 private:
  /**
   *  Stores the elements of this vector.
   */
  std::array<std::shared_ptr<Rational>, 3> elements_;
  std::array<double, 3> test;

  ExactVector() = delete;
  ExactVector(const ExactVector&) = delete;
  ExactVector& operator=(const ExactVector&) = delete;

  /**
   * Creates a new vector from an std::array of rational numbers.
   * @param values The entries for this vector. The length of this std::array is expected to be 3.
   */
  explicit ExactVector(const std::array<std::shared_ptr<Rational>, 3>& values);

  /**
   * Creates a new vector from an std::array of double values. The constructor {@link Rational#Rational(double)} is used
   * to transform the given double values into rational values.
   * @param values The entries for this vector. The length of this std::array is expected to be 3.
   */
  explicit ExactVector(const std::array<double, 3>& values);
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_EXACT_VECTOR_H_
