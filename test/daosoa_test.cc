#include <gtest/gtest.h>
#include "backend.h"
#include "cell.h"
#include "daosoa.h"
#include "inline_vector.h"

namespace bdm {

/// this class is used as payload for daosoa tests
template <typename Backend>
class Object {
 public:
  using real_v = typename Backend::real_v;

  template <typename T>
  friend class Object;

  Object() {}
  explicit Object(real_v id) : id_{id} {}

  const real_v& GetId() { return id_; }

  // --------------------------------------------------------------------------
  // required interface for usage in daosoa

  Object<ScalarBackend> Get(size_t idx) const {
    Object<ScalarBackend> o;
    o.id_ = {id_[idx]};
    return o;
  }

  void Set(size_t idx, const Object<ScalarBackend>& object) {
    id_[idx] = object.id_[0];
  }

  void push_back(const Object<ScalarBackend>& object) { Set(size_++, object); }

  bool is_full() const { return Size() == Backend::kVecLen; }

  constexpr size_t VecLength() { return Backend::kVecLen; }

  size_t Size() const { return size_; }

  void SetUninitialized() { size_ = 0; }

  void SetSize(std::size_t size) { size_ = size; }

  void CopyTo(std::size_t from_idx, std::size_t to_idx,
              Object<VcBackend>* dest) const {
    dest->id_[to_idx] = id_[from_idx];
  }

 private:
  size_t size_ = Backend::kVecLen;
  real_v id_;
};

TEST(daosoaTest, PushBackOfScalarAndVectorCell) {
  // push_back
  Cell<VcBackend> vc_cells;
  Cell<ScalarBackend> scalar_cell;

  daosoa<Cell, VcBackend> vc_daosoa;
  vc_daosoa.push_back(vc_cells);
  vc_daosoa.push_back(scalar_cell);

  daosoa<Cell, ScalarBackend> aos;
  aos.push_back(scalar_cell);
  //  aos.push_back(vc_cells);  // should not compile -> todo(lukas) make
  //  compile time tests
}

TEST(daosoaTest, PushBackAndGetScalars) {
  daosoa<Object> objects;

  // create objects
  const size_t elements = VcBackend::kVecLen * 2 + 2;
  for (size_t i = 0; i < elements; i++) {
    objects.push_back(Object<ScalarBackend>(i));
  }

  EXPECT_EQ(elements, objects.elements());
  EXPECT_EQ(size_t(3), objects.vectors());

  // check if it returns the correct objects
  for (size_t i = 0; i < elements; i++) {
    const auto vector_idx = i / VcBackend::kVecLen;
    const auto scalar_idx = i % VcBackend::kVecLen;
    EXPECT_EQ(i, objects[vector_idx].GetId()[scalar_idx]);
  }
}

TEST(daosoaTest, ReserveElementsSetScalar) {
  const size_t elements = VcBackend::kVecLen * 2 + 2;
  daosoa<Object> objects(elements);

  // create objects
  for (size_t i = 0; i < elements; i++) {
    objects.push_back(Object<ScalarBackend>(i));
  }

  EXPECT_EQ(elements, objects.elements());
  EXPECT_EQ(size_t(3), objects.vectors());

  // check if it returns the correct objects
  for (size_t i = 0; i < elements; i++) {
    const auto vector_idx = i / VcBackend::kVecLen;
    const auto scalar_idx = i % VcBackend::kVecLen;
    EXPECT_EQ(i, objects[vector_idx].GetId()[scalar_idx]);
  }
}

TEST(daosoaTest, Gather) {
  daosoa<Object> objects;

  // create objects
  for (size_t i = 0; i < 10; i++) {
    objects.push_back(Object<ScalarBackend>(i));
  }

  aosoa<Object<VcBackend>, VcBackend> gathered;
  InlineVector<int, 8> indexes;
  indexes.push_back(5);
  indexes.push_back(3);
  indexes.push_back(9);
  indexes.push_back(2);
  objects.Gather(indexes, &gathered);
  // check if it returns the correct objects
  size_t target_n_vectors =
      4 / VcBackend::kVecLen + (4 % VcBackend::kVecLen ? 1 : 0);
  EXPECT_EQ(target_n_vectors, gathered.vectors());
  size_t counter = 0;
  for (size_t i = 0; i < gathered.vectors(); i++) {
    for (size_t j = 0; j < VcBackend::kVecLen; j++) {
      EXPECT_EQ(indexes[counter++], gathered[i].GetId()[j]);
    }
  }
}

TEST(daosoaTest, SizeAndElements) {
  daosoa<Object> objects;

  EXPECT_EQ(size_t(0), objects.vectors());
  EXPECT_EQ(size_t(0), objects.elements());
  objects.push_back(Object<VcBackend>());
  EXPECT_EQ(size_t(1), objects.vectors());
  EXPECT_EQ(Vc::double_v::Size,
            objects.elements());  // fixme replace with VcBackend
  objects.push_back(Object<ScalarBackend>());
  EXPECT_EQ(size_t(2), objects.vectors());
  EXPECT_EQ(VcBackend::kVecLen + 1, objects.elements());
}

}  // namespace bdm
