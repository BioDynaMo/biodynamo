#ifndef BACKEND_H_
#define BACKEND_H_

#include <Vc/Vc>

namespace bdm {

struct VcBackend {
  typedef Vc::int_v int_v;
  typedef Vc::double_v real_v;
  typedef Vc::double_v::value_type real_t;
  static const size_t kVecLen = real_v::Size;
  typedef Vc::double_v::Mask bool_v;
};

struct ScalarBackend {
  typedef Vc::SimdArray<int, 1> int_v;
  typedef Vc::SimdArray<double, 1> real_v;
  typedef double real_t;
  typedef std::array<bool, 1> bool_v;
  static const size_t kVecLen = 1;
};

using DefaultBackend = VcBackend;

template <typename T>
struct is_scalar {
  static const bool value = false;
};

template <template <typename> class Container>
struct is_scalar<Container<ScalarBackend> > {
  static const bool value = true;
};

}  // namespace bdm

#endif  // BACKEND_H_
