#include "spatial_organization/exact_vector.h"

namespace bdm {
namespace spatial_organization {

using std::array;
using std::shared_ptr;
using bdm::spatial_organization::ExactVector;

std::shared_ptr<ExactVector> ExactVector::create(const std::array<double, 3>& values) {
  return std::shared_ptr < ExactVector > (new ExactVector(values));
}

double ExactVector::det(const std::array<shared_ptr<ExactVector>, 3>& c) {
  return (c[0]->elements_[0] * c[1]->elements_[1] * c[2]->elements_[2]) +
         (c[0]->elements_[1] * c[1]->elements_[2] * c[2]->elements_[0]) +
         (c[0]->elements_[2] * c[1]->elements_[0] * c[2]->elements_[1]) -
         (c[0]->elements_[0] * c[1]->elements_[2] * c[2]->elements_[1]) -
         (c[0]->elements_[1] * c[1]->elements_[0] * c[2]->elements_[2]) -
         (c[0]->elements_[2] * c[1]->elements_[1] * c[2]->elements_[0]);
}

ExactVector::ExactVector(const array<double, 3>& values)
    : elements_(values) {
}

ExactVector::~ExactVector() {
}

double ExactVector::squaredLength() const {
  double l = 0;
  for (auto element : elements_) {
    l += element*element;
  }
  return l;
}

shared_ptr<ExactVector> ExactVector::add(const shared_ptr<ExactVector>& other) const {
  array<double, 3> vector;
  for (int i = 0; i < 3; i++)
    vector[i] = elements_[i] + other->elements_[i];
  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::increaseBy(const shared_ptr<ExactVector>& other) {
  for (int i = 0; i < 3; i++)
    elements_[i] += other->elements_[i];
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::subtract(const shared_ptr<ExactVector>& other) {
  array<double, 3> vector;
  for (int i = 0; i < 3; i++)
    vector[i] = elements_[i] - other->elements_[i];
  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::decreaseBy(const shared_ptr<ExactVector>& other) {
  for (int i = 0; i < 3; i++)
    elements_[i] -= other->elements_[i];
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::multiply(double factor) {
  array<double, 3> vector;
  for (int i = 0; i < 3; i++)
    vector[i] = elements_[i] * factor;
  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::multiplyBy(double factor) {
  for (int i = 0; i < 3; i++)
    elements_[i] *= factor;
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::divide(double factor) {
  array<double, 3> vector;
  for (int i = 0; i < 3; i++)
    vector[i] = elements_[i] / factor;
  return ExactVector::create(vector);
}

shared_ptr<ExactVector> ExactVector::divideBy(double factor) {
  for (int i = 0; i < 3; i++)
    elements_[i] /= factor;
  return shared_from_this();
}

double ExactVector::dotProduct(const shared_ptr<ExactVector>& other) {
  double r = 0;
  for (int i = 0; i < 3; i++)
    r += other->elements_[i] * elements_[i];
  return r;
}

shared_ptr<ExactVector> ExactVector::negate() {
  for (int i = 0; i < 3; i++)
    elements_[i] = -elements_[i];
  return shared_from_this();
}

shared_ptr<ExactVector> ExactVector::crossProduct(const shared_ptr<ExactVector>& other) {
  array<double, 3> vector;
  for (int i = 0; i < 3; i++) {
    vector[i] = (elements_[((i + 1) % 3)] * other->elements_[((i + 2) % 3)]) -
                (elements_[((i + 2) % 3)] * other->elements_[((i + 1) % 3)]);
  }
  return ExactVector::create(vector);
}

std::string ExactVector::toString() {
  std::stringstream ret;
  ret << "(";
  for (size_t i = 0; i < elements_.size(); i++) {
    if (i != 0)
      ret << ", ";
    ret << elements_[i];
  }
  ret << ")";
  return ret.str();
}

bool ExactVector::equalTo(const shared_ptr<ExactVector>& other) {
  for (size_t i = 0; i < elements_.size(); i++) {
    if (elements_[i] - other->elements_[i] != 0) {
      return false;
    }
  }
  return true;
}

}  // namespace spatial_organization
}  // namespace bdm
