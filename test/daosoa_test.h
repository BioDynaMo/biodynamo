#ifndef DAOSOA_TEST_H_
#define DAOSOA_TEST_H_

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

  explicit Object(TRootIOCtor*) {}
  Object() {}
  explicit Object(real_v id) : id_{id} {}

  virtual ~Object() {}

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

  void Append(const Object<ScalarBackend>& object) { Set(size_++, object); }

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
  ClassDef(Object, 1);
};

}  // namespace bdm

#endif  // DAOSOA_TEST_H_
