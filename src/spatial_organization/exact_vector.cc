#include "spatial_organization/exact_vector.h"

#include "spatial_organization/rational.h"

namespace cx3d {
namespace spatial_organization {

using std::array;
using std::shared_ptr;
using cx3d::spatial_organization::ExactVector;

shared_ptr<Rational> ExactVector::det(const std::array<shared_ptr<ExactVector>, 3>& c) {
  return c[0]->elements_[0]->multiply(c[1]->elements_[1])->multiply(c[2]->elements_[2])->add(
      c[0]->elements_[1]->multiply(c[1]->elements_[2])->multiply(c[2]->elements_[0]))->add(
      c[0]->elements_[2]->multiply(c[1]->elements_[0])->multiply(c[2]->elements_[1]))->subtract(
      c[0]->elements_[0]->multiply(c[1]->elements_[2])->multiply(c[2]->elements_[1]))->subtract(
      c[0]->elements_[1]->multiply(c[1]->elements_[0])->multiply(c[2]->elements_[2]))->subtract(
      c[0]->elements_[2]->multiply(c[1]->elements_[1])->multiply(c[2]->elements_[0]));
}

ExactVector::ExactVector(const array<shared_ptr<Rational>, 3>& values)
    : elements_(values) {
}

ExactVector::ExactVector(const array<double, 3>& values)
    : elements_(
        array<shared_ptr<Rational>, 3>({ Rational::create(values[0]), Rational::create(values[1]),
            Rational::create(values[2]) })) {
}

ExactVector::~ExactVector() {
}

shared_ptr<Rational> ExactVector::squaredLength() const {
  auto rational = Rational::create(0L, 1L);
  for (auto element : elements_) {
    rational->add(element->multiply(element));
  }
  return rational;
}

shared_ptr<ExactVector> ExactVector::add(const shared_ptr<ExactVector>& other) const {
  array<shared_ptr<Rational>, 3> vector;
  vector[0] = elements_[0]->add(other->elements_[0]);
  vector[1] = elements_[1]->add(other->elements_[1]);
  vector[2] = elements_[2]->add(other->elements_[2]);

  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::increaseBy(const shared_ptr<ExactVector>& other) {
  for (int i = 0; i < 3; i++) {
    elements_[i]->increaseBy(other->elements_[i]);
  }
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::subtract(const shared_ptr<ExactVector>& other) {
  array<shared_ptr<Rational>, 3> vector;
  vector[0] = elements_[0]->subtract(other->elements_[0]);
  vector[1] = elements_[1]->subtract(other->elements_[1]);
  vector[2] = elements_[2]->subtract(other->elements_[2]);

  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::decreaseBy(const shared_ptr<ExactVector>& other) {
  for (int i = 0; i < 3; i++) {
    elements_[i]->decreaseBy(other->elements_[i]);
  }
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::multiply(const shared_ptr<Rational>& factor) {
  array<shared_ptr<Rational>, 3> vector;
  vector[0] = elements_[0]->multiply(factor);
  vector[1] = elements_[1]->multiply(factor);
  vector[2] = elements_[2]->multiply(factor);

  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::multiplyBy(const shared_ptr<Rational>& factor) {
  for (auto element : elements_) {
    element->multiplyBy(factor);
  }
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::divide(const shared_ptr<Rational>& factor) {
  array<shared_ptr<Rational>, 3> vector;
  vector[0] = elements_[0]->divide(factor);
  vector[1] = elements_[1]->divide(factor);
  vector[2] = elements_[2]->divide(factor);

  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::divideBy(const shared_ptr<Rational>& factor) {
  for (auto element : elements_) {
    element->divideBy(factor);
  }
  return shared_from_this();
}

shared_ptr<Rational> ExactVector::dotProduct(const shared_ptr<ExactVector>& other) {
  auto rational = Rational::create(0L, 1L);
  for (int i = 0; i < 3; i++) {
    rational = rational->add(other->elements_[i]->multiply(elements_[i]));
  }
  return rational;
}

shared_ptr<ExactVector> ExactVector::negate() {
  for (auto element : elements_) {
    element->negate();
  }
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::crossProduct(const shared_ptr<ExactVector>& other) {
  array<shared_ptr<Rational>, 3> vector;
  for (int i = 0; i < 3; i++) {
    vector[i] = elements_[((i + 1) % 3)]->multiply(other->elements_[((i + 2) % 3)])->subtract(
        elements_[((i + 2) % 3)]->multiply(other->elements_[((i + 1) % 3)]));
  }
  return ExactVector::create(vector);
}

std::string ExactVector::toString() {
  std::stringstream ret;
  ret << "(";
  for (size_t i = 0; i < elements_.size(); i++) {
    if (i != 0)
      ret << ", ";
    ret << elements_[i]->toString();
  }
  ret << ")";
  return ret.str();
}

bool ExactVector::equalTo(const shared_ptr<ExactVector>& other) {
  for (size_t i = 0; i < elements_.size(); i++) {
    if (elements_[i]->compareTo(other->elements_[i]) != 0) {
      return false;
    }
  }
  return true;
}

}  // namespace spatial_organization
}  // namespace cx3d
