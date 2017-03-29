#include <gtest/gtest.h>
#include "backend.h"
#include "cell.h"
#include "daosoa.h"
#include "inline_vector.h"

namespace bdm {

/// this class is used as payload for daosoa tests
template <typename TBackend=VcVectorBackend>
class Object {
 public:
  using Backend = TBackend;
  using real_v = typename TBackend::real_v;

  template <typename T>
  friend class Object;

  Object() {}
  explicit Object(real_v id) : id_{id} {}

  const real_v& GetId() { return id_; }

  // --------------------------------------------------------------------------
  // required interface for usage in daosoa

  void push_back(const Object<ScalarBackend>& object) {
    id_[size_++] = object.id_[0];
  }

  bool is_full() const { return size() == Backend::kVecLen; }

  size_t size() const { return size_; }

  void SetSize(std::size_t size) { size_ = size; }

  void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
              std::size_t dest_v_idx, std::size_t dest_idx,
              Object<VcVectorBackend>* dest) const {
    dest->id_[dest_idx] = id_[src_idx];
  }

 private:
  size_t size_ = Backend::kVecLen;
  real_v id_;
};

TEST(daosoaTest, PushBackOfScalarAndVectorCell) {
  // push_back
  Cell<VcVectorBackend> vc_cells;
  Cell<ScalarBackend> scalar_cell;

  daosoa<Cell<VcVectorBackend>> vc_daosoa;
  vc_daosoa.push_back(vc_cells);
  vc_daosoa.push_back(scalar_cell);

  daosoa<Cell<ScalarBackend>> aos;
  aos.push_back(scalar_cell);
  //  aos.push_back(vc_cells);  // should not compile -> todo(lukas) make
  //  compile time tests
}

TEST(daosoaTest, PushBackAndGetScalars) {
  daosoa<Object<>> objects;

  // create objects
  const size_t elements = VcVectorBackend::kVecLen * 2 + 2;
  for (size_t i = 0; i < elements; i++) {
    objects.push_back(Object<ScalarBackend>(i));
  }

  EXPECT_EQ(elements, objects.Elements());
  EXPECT_EQ(size_t(3), objects.Vectors());

  // check if it returns the correct objects
  for (size_t i = 0; i < elements; i++) {
    const auto vector_idx = i / VcVectorBackend::kVecLen;
    const auto scalar_idx = i % VcVectorBackend::kVecLen;
    EXPECT_EQ(i, objects[vector_idx].GetId()[scalar_idx]);
  }
}

TEST(daosoaTest, ReserveElementsSetScalar) {
  const size_t elements = VcVectorBackend::kVecLen * 2 + 2;
  daosoa<Object<>> objects(elements);

  // create objects
  for (size_t i = 0; i < elements; i++) {
    objects.push_back(Object<ScalarBackend>(i));
  }

  EXPECT_EQ(elements, objects.Elements());
  EXPECT_EQ(size_t(3), objects.Vectors());

  // check if it returns the correct objects
  for (size_t i = 0; i < elements; i++) {
    const auto vector_idx = i / VcVectorBackend::kVecLen;
    const auto scalar_idx = i % VcVectorBackend::kVecLen;
    EXPECT_EQ(i, objects[vector_idx].GetId()[scalar_idx]);
  }
}

TEST(daosoaTest, Gather) {
  daosoa<Object<>> objects;

  // create objects
  for (size_t i = 0; i < 10; i++) {
    objects.push_back(Object<ScalarBackend>(i));
  }

  aosoa<Object<VcVectorBackend>, VcVectorBackend> gathered;
  InlineVector<int, 8> indexes;
  indexes.push_back(5);
  indexes.push_back(3);
  indexes.push_back(9);
  indexes.push_back(2);
  objects.Gather(indexes, &gathered);
  // check if it returns the correct objects
  size_t target_n_vectors =
      4 / VcVectorBackend::kVecLen + (4 % VcVectorBackend::kVecLen ? 1 : 0);
  EXPECT_EQ(target_n_vectors, gathered.Vectors());
  size_t counter = 0;
  for (size_t i = 0; i < gathered.Vectors(); i++) {
    for (size_t j = 0; j < VcVectorBackend::kVecLen; j++) {
      EXPECT_EQ(indexes[counter++], gathered[i].GetId()[j]);
    }
  }
}

TEST(daosoaTest, SizeAndElements) {
  daosoa<Object<>> objects;

  EXPECT_EQ(size_t(0), objects.Vectors());
  EXPECT_EQ(size_t(0), objects.Elements());
  objects.push_back(Object<VcVectorBackend>());
  EXPECT_EQ(size_t(1), objects.Vectors());
  EXPECT_EQ(Vc::double_v::Size,
            objects.Elements());  // fixme replace with VcVectorBackend
  objects.push_back(Object<ScalarBackend>());
  EXPECT_EQ(size_t(2), objects.Vectors());
  EXPECT_EQ(VcVectorBackend::kVecLen + 1, objects.Elements());
}

}  // namespace bdm
