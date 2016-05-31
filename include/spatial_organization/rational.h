#ifndef SPATIAL_ORGANIZATION_RATIONAL_H_
#define SPATIAL_ORGANIZATION_RATIONAL_H_

#include <stdint.h>
#include <memory>
#include <string>
#include "gmpxx.h"

namespace cx3d {
namespace spatial_organization {

using BigInteger = mpz_class;

/**
 * Used to represent numbers as fractions. Each number therefore consists of a numerator and
 * a denominator, both of which are stored as instances of type {@link BigInteger}.
 *
 * All numbers are stored as canceled fractions. This class provides the functionality to
 * add, subtract, multiply, divide or compare two rational numbers.
 *
 */
class Rational : public std::enable_shared_from_this<Rational> {
 public:
  /**
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
   *
   * template<typename ... T>
   * static std::shared_ptr<Rational> create(T&& ... all) {
   *   return std::shared_ptr<Rational>(new Rational(std::forward<T>(all)...));
   * }
   */
  static std::shared_ptr<Rational> create(uint64_t numerator, uint64_t denominator);

  /**
   * @see static std::shared_ptr<Rational> create(long numerator, long denominator)
   */
  static std::shared_ptr<Rational> create(const BigInteger& numerator, const BigInteger& denominator);

  /**
   * @see static std::shared_ptr<Rational> create(long numerator, long denominator)
   */
  static std::shared_ptr<Rational> create(double value);

  virtual ~Rational();

  /**
   * @return <code>true</code>, if this number is equal to 0.
   */
  virtual bool isZero() const;

  /**
   * Negates this rational number. The number itself is changed during this process.
   * @return A reference to <code><b>this</b></code>.
   */
  virtual std::shared_ptr<Rational> negate();

  /**
   * Adds another rational number to this rational and returns a new instance of Rational. This number itself
   * is not modified during this calculation.
   * @param other The second argument of the addition.
   * @return A new instance of <code>Rational</code> representing the result.
   */
  virtual std::shared_ptr<Rational> add(const std::shared_ptr<Rational>& other) const;

  /**
   * Increases this rational by another rational. The number itself is modified during this calculation.
   * @param otherValue The value by which this rational should be increased.
   * @return A reference to this object itself.
   */
  virtual std::shared_ptr<Rational> increaseBy(const std::shared_ptr<Rational>& other);

  /**
   * Subtracts another rational number from this rational and returns a new instance of Rational. This number itself
   * is not modified during this calculation.
   * @param otherValue The second argument of the subtraction.
   * @return A new instance of <code>Rational</code> representing the result.
   */
  virtual std::shared_ptr<Rational> subtract(const std::shared_ptr<Rational>& other) const;

  /**
   * Decreases this rational by another rational. The number itself is modified during this calculation.
   * @param otherValue The value by which this rational should be decreased.
   * @return A reference to this object itself.
   */
  virtual std::shared_ptr<Rational> decreaseBy(const std::shared_ptr<Rational>& other);

  /**
   * Multiplies another rational number with this rational and returns a new instance of <code>Rational</code>. This number itself
   * is not modified during this calculation.
   * @param otherValue The second argument of the multiplication.
   * @return A new instance of <code>Rational</code> representing the result.
   */
  virtual std::shared_ptr<Rational> multiply(const std::shared_ptr<Rational>& other) const;

  /**
   * Multiplies this rational by another rational number. The number itself is modified during this calculation.
   * @param otherValue The value by which this rational should be multiplied.
   * @return A reference to this object itself.
   */
  virtual std::shared_ptr<Rational> multiplyBy(const std::shared_ptr<Rational>& other);

  /**
   * Divides this rational number by another rational number and returns a new instance of <code>Rational</code>. This number itself
   * is not modified during this calculation.
   * @param otherValue The second argument of the division.
   * @return A new instance of <code>Rational</code> representing the result.
   */
  virtual std::shared_ptr<Rational> divide(const std::shared_ptr<Rational>& other) const;

  /**
   * Divides this rational by another rational number. The number itself is modified during this calculation.
   * @param otherValue The value by which this rational should be divided.
   * @return A reference to this object itself.
   */
  virtual std::shared_ptr<Rational> divideBy(const std::shared_ptr<Rational>& other);

  /**
   * Creates a double approximation of this rational.
   * @return An approximation of this rational. The function internally transforms the numerator and the denominator
   * into <code>double</code> values and then returns the division of the first by the second.
   */
  virtual double doubleValue() const;

  /**
   * Compares this object with the specified object for order.
   * Returns a negative integer, zero, or a positive integer as this object is less than, equal to, or greater
   * than the specified object.
   */
  virtual int compareTo(const std::shared_ptr<Rational>& other) const;

  /**
   * Returns a string representation of this object.
   */
  virtual std::string toString();

  /**
   * Determines if two instances of this object are equal
   */
  virtual bool equalTo(const std::shared_ptr<Rational>& other);

 protected:
  /**
   * Divides numerator and denominator of this rational number by their greatest common divisor.
   */
  virtual void cancel();

 private:
  /**
   * The numerator of this rational number.
   */
  BigInteger numerator_;

  /**
   * The denominator of this rational number.
   */
  BigInteger denominator_;

  Rational() = delete;
  Rational(const Rational&) = delete;
  Rational& operator=(const Rational& other) = delete;

  /**
   * Initializes a new rational number from two long integer values.
   * Both values are transformed internally into numbers of type {@link BigInteger}.
   * @param numerator The numerator of the new rational.
   * @param denominator The denominator of the new rational.
   */
  Rational(uint64_t numerator, uint64_t denominator);

  /**
   * Initializes a new rational number from two values of type {@link BigInteger}.
   * @param numerator The numerator of the new rational.
   * @param denominator The denominator of the new rational.
   */
  Rational(const BigInteger& numerator, const BigInteger& denominator);

  /**
   * Creates a rational number from a <code>double</code> value.
   * The floating point number is split into mantisse and exponent and these are then
   * used to calculate numerator and denominator of the rational number.
   * The procedure used assures that the number is correctly transformed into
   * the rational number which it represents.
   * @param value The double value that should be transformed.
   */
  explicit Rational(double value);

  /**
   * Computes 2^<code>exp</code>.
   * @param exp The exponent.
   * @return A <code>BigInteger</code> storing the value of 2^<code>exp</code>.
   */
  const BigInteger pow2(int exp) const;

  /**
   * Sets the BigInteger object to value
   * @param big_int The BigInteger object whose internal representation should be changed
   * @param value The target value which big_int should be set to
   */
  void setBigIntTo(BigInteger& big_int, uint64_t value);
};

}  // namespace spatial_organization
}  // namespace cx3d

#endif  // SPATIAL_ORGANIZATION_RATIONAL_H_
