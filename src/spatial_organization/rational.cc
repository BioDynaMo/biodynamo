#include "spatial_organization/rational.h"

#include <stdexcept>

#include "string_util.h"

using std::shared_ptr;

namespace cx3d {
namespace spatial_organization {

#ifndef RATIONAL_NATIVE
Rational::Rational()
    : numerator_(0),
      denominator_(1) {
}
#endif

Rational::Rational(uint64_t numerator, uint64_t denominator)
    : numerator_(0),
      denominator_(1) {
  setBigIntTo(numerator_, numerator);
  setBigIntTo(denominator_, denominator);
}

Rational::Rational(const BigInteger& numerator, const BigInteger& denominator)
    : numerator_(numerator),
      denominator_(denominator) {
  if (sgn(denominator) == -1) {
    mpz_neg(numerator_.get_mpz_t(), numerator.get_mpz_t());
    mpz_neg(denominator_.get_mpz_t(), denominator.get_mpz_t());
  }
}

Rational::Rational(double value)
    : numerator_(0),
      denominator_(1) {
  uint64_t mantisse;
  memcpy(&mantisse, &value, sizeof(uint64_t));
  int64_t ex = ((mantisse & 0x7ff0000000000000L) >> 52) - 1023;
  uint64_t sign = mantisse & 0x8000000000000000L;

  // lets calculate the mantisse:
  mantisse &= 0x000fffffffffffffL;
  uint64_t denom = 0x0010000000000000L;
  // if value is a denormalized value...
  if (ex == -1023) {
    ex++;
    mantisse <<= 1;
  } else {  // otherwise, add the denominator to the mantisse:
    mantisse |= denom;
  }

  if (sign != 0) {
    numerator_ -= static_cast<unsigned long int>(mantisse);
  } else {
    numerator_ += static_cast<unsigned long int>(mantisse);;
  }
  denominator_ += static_cast<unsigned long int>(denom);;

  if (ex > 0) {
    numerator_ = numerator_ * pow2(static_cast<int>(ex));
  } else {
    denominator_ = denominator_ * pow2(-static_cast<int>(ex));
  }
  cancel();
}

Rational::~Rational() {
}

bool Rational::isZero() const {
  return numerator_ == 0;
}

shared_ptr<Rational> Rational::negate() {
  mpz_neg(numerator_.get_mpz_t(), numerator_.get_mpz_t());
  return shared_from_this();
}

shared_ptr<Rational> Rational::add(const shared_ptr<Rational>& other) const {
  BigInteger gcd;
  mpz_gcd(gcd.get_mpz_t(), denominator_.get_mpz_t(), other->denominator_.get_mpz_t());

  BigInteger other_non_div = other->denominator_ / gcd;
  BigInteger new_numerator = (numerator_ * other_non_div) + (other->numerator_ * (denominator_ / gcd));
  BigInteger new_denominator = denominator_ * other_non_div;

  return Rational::create(new_numerator, new_denominator);
}

shared_ptr<Rational> Rational::increaseBy(const shared_ptr<Rational>& other) {
  BigInteger gcd;
  mpz_gcd(gcd.get_mpz_t(), denominator_.get_mpz_t(), other->denominator_.get_mpz_t());

  BigInteger other_non_div = other->denominator_ / gcd;
  numerator_ = (numerator_ * other_non_div) + (other->numerator_ * (denominator_ / gcd));
  denominator_ = denominator_ * other_non_div;

  return shared_from_this();
}

shared_ptr<Rational> Rational::subtract(const shared_ptr<Rational>& other) const {
  BigInteger gcd;
  mpz_gcd(gcd.get_mpz_t(), denominator_.get_mpz_t(), other->denominator_.get_mpz_t());

  BigInteger other_non_div = other->denominator_ / gcd;
  BigInteger new_numerator = (numerator_ * other_non_div) - (other->numerator_ * (denominator_ / gcd));
  BigInteger new_denominator = denominator_ * other_non_div;

  return Rational::create(new_numerator, new_denominator);
}

shared_ptr<Rational> Rational::decreaseBy(const shared_ptr<Rational>& other) {
  BigInteger gcd;
  mpz_gcd(gcd.get_mpz_t(), denominator_.get_mpz_t(), other->denominator_.get_mpz_t());

  BigInteger other_non_div = other->denominator_ / gcd;
  numerator_ = (numerator_ * other_non_div) - (other->numerator_ * (denominator_ / gcd));
  denominator_ = denominator_ * other_non_div;

  return shared_from_this();
}

shared_ptr<Rational> Rational::multiply(const shared_ptr<Rational>& other) const {
  BigInteger this_num_other_denom_gcd;
  mpz_gcd(this_num_other_denom_gcd.get_mpz_t(), numerator_.get_mpz_t(), other->denominator_.get_mpz_t());
  BigInteger other_num_this_denom_gcd;
  mpz_gcd(other_num_this_denom_gcd.get_mpz_t(), other->numerator_.get_mpz_t(), denominator_.get_mpz_t());

  BigInteger new_numerator = (numerator_ / this_num_other_denom_gcd) * (other->numerator_ / other_num_this_denom_gcd);
  BigInteger new_denominator = (denominator_ / other_num_this_denom_gcd)
      * (other->denominator_ / this_num_other_denom_gcd);

  return Rational::create(new_numerator, new_denominator);
}

shared_ptr<Rational> Rational::multiplyBy(const shared_ptr<Rational>& other) {
  BigInteger this_num_other_denom_gcd;
  mpz_gcd(this_num_other_denom_gcd.get_mpz_t(), numerator_.get_mpz_t(), other->denominator_.get_mpz_t());
  BigInteger other_num_this_denom_gcd;
  mpz_gcd(other_num_this_denom_gcd.get_mpz_t(), other->numerator_.get_mpz_t(), denominator_.get_mpz_t());

  numerator_ = (numerator_ / this_num_other_denom_gcd) * (other->numerator_ / other_num_this_denom_gcd);
  denominator_ = (denominator_ / other_num_this_denom_gcd) * (other->denominator_ / this_num_other_denom_gcd);

  return shared_from_this();
}

shared_ptr<Rational> Rational::divide(const shared_ptr<Rational>& other) const {
  if (other->numerator_ == 0) {
    throw std::invalid_argument("Divisor must not be zero!");
  }
  BigInteger this_num_other_denom_gcd;
  mpz_gcd(this_num_other_denom_gcd.get_mpz_t(), numerator_.get_mpz_t(), other->denominator_.get_mpz_t());
  BigInteger other_num_this_denom_gcd;
  mpz_gcd(other_num_this_denom_gcd.get_mpz_t(), other->numerator_.get_mpz_t(), denominator_.get_mpz_t());

  BigInteger new_numerator = (numerator_ / this_num_other_denom_gcd) * (other->denominator_ / other_num_this_denom_gcd);
  BigInteger new_denominator = (denominator_ / other_num_this_denom_gcd)
      * (other->numerator_ / this_num_other_denom_gcd);

  return Rational::create(new_numerator, new_denominator);
}

shared_ptr<Rational> Rational::divideBy(const shared_ptr<Rational>& other) {
  if (other->numerator_ == 0) {
    throw std::invalid_argument("Divisor must not be zero!");
  }
  BigInteger this_num_other_denom_gcd;
  mpz_gcd(this_num_other_denom_gcd.get_mpz_t(), numerator_.get_mpz_t(), other->denominator_.get_mpz_t());
  BigInteger other_num_this_denom_gcd;
  mpz_gcd(other_num_this_denom_gcd.get_mpz_t(), other->numerator_.get_mpz_t(), denominator_.get_mpz_t());

  numerator_ = (numerator_ / this_num_other_denom_gcd) * (other->denominator_ / other_num_this_denom_gcd);
  denominator_ = (denominator_ / other_num_this_denom_gcd) * (other->numerator_ / this_num_other_denom_gcd);

  return shared_from_this();
}

double Rational::doubleValue() const {
  return numerator_.get_d() / denominator_.get_d();
}

std::string Rational::toString() {
  return StringUtil::toStr(doubleValue());
}

void Rational::cancel() {
  BigInteger gcd;
  mpz_gcd(gcd.get_mpz_t(), numerator_.get_mpz_t(), denominator_.get_mpz_t());

  numerator_ = numerator_ / gcd;
  denominator_ = denominator_ / gcd;
}

const BigInteger Rational::pow2(int exp) const {
  BigInteger result(1);  // = new BigInteger(1);
  BigInteger temp(2);
  while (exp > 0) {
    if ((exp & 1) == 1) {
      result = result * temp;
    }
    temp = temp * temp;
    exp >>= 1;
  }
  return result;
}

int Rational::compareTo(const shared_ptr<Rational>& other) const {
  return sgn(this->subtract(other)->numerator_);
}

void Rational::setBigIntTo(BigInteger& big_int, uint64_t value) {
  auto representation = big_int.get_mpz_t();

  mpz_set_si(representation, static_cast<int>(value >> 32));
  mpz_mul_2exp(representation, representation, 32);
  mpz_add_ui(representation, representation, value);
}

bool Rational::equalTo(const std::shared_ptr<Rational>& other) {
  return compareTo(other) == 0;
}

}  // namespace spatial_organization
}  // namespace cx3d
