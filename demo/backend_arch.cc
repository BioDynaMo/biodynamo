#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <sstream>
#include <utility>
#include <typeinfo>

#include <Vc/Vc>
#include <omp.h>

#include "cpp_magic.h"
#include "timing.h"
#include "timing_aggregator.h"

using bdm::Timing;
using bdm::TimingAggregator;

// benchmark function declarations
void benchmarkSoaCell(const size_t num_cells, const size_t iterations,
                      TimingAggregator* statistic);
void benchmarkAosoaCell(const size_t num_cells, const size_t iterations,
                        TimingAggregator* statistic);
void benchmarkPlainSoa(const size_t num_cells, const size_t iterations,
                       TimingAggregator* statistic);
void benchmarkPlainAosoa(const size_t num_cells, const size_t iterations,
                         TimingAggregator* statistic);

template <typename T>
class SoaRefWrapper {
 public:
  SoaRefWrapper(T& data) : data_(data) {}

  // TODO add all operators

  Vc_ALWAYS_INLINE typename T::value_type& operator[](std::size_t index) {
    return data_[index];
  }

  Vc_ALWAYS_INLINE const typename T::value_type& operator[](
      std::size_t index) const {
    return data_[index];
  }

  template <typename U>
  Vc_ALWAYS_INLINE auto operator<=(const U& u) const
      -> decltype(std::declval<typename T::value_type>() <= u) {
    return data_ <= u;
  }

  template <typename U>
  Vc_ALWAYS_INLINE auto operator<(const U& u) const
      -> decltype(std::declval<typename T::value_type>() < u) {
    return data_ < u;
  }

  template <typename U>
  Vc_ALWAYS_INLINE SoaRefWrapper<T>& operator+=(const U& u) {
    data_ += u;
    return *this;
  }

  Vc_ALWAYS_INLINE SoaRefWrapper<T>& operator=(const SoaRefWrapper<T>& other) {
    if (this != &other) {
      data_ = other.data_;
    }
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const SoaRefWrapper<T>& wrapper) {
    out << wrapper.data_;
    return out;
  }

  typename T::iterator begin() { return data_.begin(); }
  typename T::iterator end() { return data_.end(); }

  typename T::const_iterator begin() const { return data_.cbegin(); }
  typename T::const_iterator end() const { return data_.cend(); }

 private:
  T& data_;
};

/// This class represents an array with exactly one element
/// Needed for AOSOA: Objects will store a single e.g. real_v instead of N
/// instances. However code was written for SOA and expects an array interface
/// which is exposed with this class.
/// Makes it easy for the compiler to optimize out the extra call to operator[]
/// Didn't work with std::array<T, 1>
template <typename T>
class OneElementArray {
 public:
  OneElementArray() : data_() {}
  OneElementArray(const T& data) : data_(data) {}
  OneElementArray(T&& data) : data_(data) {}

  Vc_ALWAYS_INLINE T& operator[](const size_t idx) { return data_; }

  Vc_ALWAYS_INLINE const T& operator[](const size_t idx) const { return data_; }

  T* begin() { return &data_; }
  T* end() { return &data_ + 1; }

  const T* begin() const { return &data_; }
  const T* end() const { return &data_ + 1; }

 private:
  T data_;
};

template <bool condition, typename T, typename U>
struct type_ternary_operator {};

template <typename T, typename U>
struct type_ternary_operator<true, T, U> {
  typedef T type;
};

template <typename T, typename U>
struct type_ternary_operator<false, T, U> {
  typedef U type;
};

struct VcBackend {
  typedef const std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = Vc::double_v::Size;
  typedef Vc::double_v real_v;
  template <typename T>
  using SimdArray = std::array<T, kVecLen>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = OneElementArray<T>;
};

struct VcSoaBackend {
  typedef std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = VcBackend::kVecLen;
  typedef VcBackend::real_v real_v;
  template <typename T>
  using SimdArray = typename VcBackend::template SimdArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = std::vector<T, Allocator>;
};

struct VcSoaRefBackend {
  typedef std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = VcBackend::kVecLen;
  typedef VcBackend::real_v real_v;
  template <typename T>
  using SimdArray = typename VcSoaBackend::template SimdArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container =
      SoaRefWrapper<typename VcSoaBackend::template Container<T, Allocator>>;
};

struct ScalarBackend {
  typedef const std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = 1;
  // TODO change to OneElementArray?
  typedef Vc::SimdArray<double, kVecLen> real_v;
  template <typename T>
  using SimdArray = OneElementArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = OneElementArray<T>;
};

typename VcBackend::real_v iif(
    const decltype(std::declval<typename VcBackend::real_v>() <
                   std::declval<typename VcBackend::real_v>())& condition,
    const typename VcBackend::real_v& true_value,
    const typename VcBackend::real_v& false_value) {
  return Vc::iif(condition, true_value, false_value);
}

/// loops over variadic macro arguments and calls the specified operation
/// processes one argument in each iteration
/// e.g. LOOP(OP, a, b) will lead to:
/// OP(a)
/// OP(b)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP(operation, first, ...)                          \
  operation(first)                                           \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP)()(operation, __VA_ARGS__))
#define _LOOP() LOOP
// clang-format on

#define BDM_CLASS_HEADER_PUSH_BACK_BODY(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_PUSH_BACK_BODY_ITERATOR(data_member) \
  data_member.push_back(other.data_member[0]);

#define BDM_CLASS_HEADER_CPY_CTOR_INIT(...) \
  EVAL(LOOP(BDM_CLASS_HEADER_CPY_CTOR_INIT_ITERATOR, __VA_ARGS__))

#define BDM_CLASS_HEADER_CPY_CTOR_INIT_ITERATOR(data_member) \
  data_member(other.data_member),

#define BDM_CLASS_HEADER(class_name, self_specifier, ...)                      \
 public:                                                                       \
  /* reduce verbosity of some types and variables by defining a local alias */ \
  using Base::idx_;                                                            \
                                                                               \
  using Backend = typename Base::Backend;                                      \
  using real_v = typename Backend::real_v;                                     \
                                                                               \
  template <typename T>                                                        \
  using SimdArray = typename Backend::template SimdArray<T>;                   \
                                                                               \
  template <typename T, typename Allocator = std::allocator<T>>                \
  using Container = typename Backend::template Container<T, Allocator>;        \
                                                                               \
  template <typename Backend>                                                  \
  using Self = self_specifier;                                                 \
                                                                               \
  /* all template versions of this class are friends of each other */          \
  /* so they can access each others data members */                            \
  template <typename T>                                                        \
  friend class class_name;                                                     \
                                                                               \
  template <class... A>                                                        \
  static Self<ScalarBackend> NewScalar(const A&... a) {                        \
    return Self<ScalarBackend>(a...);                                          \
  }                                                                            \
                                                                               \
 protected:                                                                    \
  /* TODO call Base class cpy ctor */                                          \
  /* Ctor to create SoaRefBackend */                                           \
  /* only compiled if T == VcSoaRefBackend */                                  \
  /* template parameter required for enable_if - otherwise compile error */    \
  template <typename T = Backend>                                              \
  class_name(Self<VcSoaBackend>& other,                                        \
             typename std::enable_if<                                          \
                 std::is_same<T, VcSoaRefBackend>::value>::type* = 0)          \
      : REMOVE_TRAILING_COMMAS(BDM_CLASS_HEADER_CPY_CTOR_INIT(__VA_ARGS__)) {} \
                                                                               \
 public:                                                                       \
  /* TODO only for SoaBackends */                                              \
  /* needed because operator[] is not thread safe - index is shared among  */  \
  /* all threads */                                                            \
  Vc_ALWAYS_INLINE Self<VcSoaRefBackend> GetSoaRef() {                         \
    return Self<VcSoaRefBackend>(*this);                                       \
  }                                                                            \
                                                                               \
  /* TODO call base class */                                                   \
  /* only compiled if Backend == Soa(Ref)Backend */                            \
  /* template parameter required for enable_if - otherwise compile error */    \
  template <typename T = Backend>                                              \
  void push_back(                                                              \
      const Self<VcBackend>& other,                                            \
      typename std::enable_if<std::is_same<T, VcSoaRefBackend>::value ||       \
                              std::is_same<T, VcSoaBackend>::value>::type* =   \
          0) {                                                                 \
    BDM_CLASS_HEADER_PUSH_BACK_BODY(__VA_ARGS__);                              \
  }                                                                            \
                                                                               \
  /* only compiled if Backend == Soa(Ref)Backend */                            \
  /* template parameter required for enable_if - otherwise compile error */    \
  template <typename T = Backend>                                              \
  void push_back(                                                              \
      const Self<VcBackend>& other,                                            \
      typename std::enable_if<std::is_same<T, ScalarBackend>::value>::type* =  \
          0) {                                                                 \
    throw std::runtime_error("TODO implement: see src/cell.h:Append");         \
  }                                                                            \
                                                                               \
  /* This operator is not thread safe! all threads modify the same index. */   \
  /* For parallel execution create a reference object for each thread -- */    \
  /* see GetSoaRef */                                                          \
  /* only compiled if Backend == Soa(Ref)Backend */                            \
  /* no version if Backend == VcBackend that returns a Self<ScalarBackend> */  \
  /* since this would involves copying of elements and would therefore */      \
  /* degrade performance -> it is therefore discouraged */                     \
  template <typename T = Backend>                                              \
  typename std::enable_if<std::is_same<T, VcSoaRefBackend>::value ||           \
                              std::is_same<T, VcSoaBackend>::value,            \
                          Self<Backend>&>::type                                \
  operator[](int index) {                                                      \
    idx_ = index;                                                              \
    return *this;                                                              \
  }                                                                            \
                                                                               \
 private:

struct Neurite {
  Neurite() : id(0) {}
  Neurite(size_t id) : id(id) {}
  size_t id = 0;
};

template <typename TBackend>
struct BaseCell {
 protected:
  template <typename TTBackend>
  using Self = BaseCell<TTBackend>;

  using Backend = TBackend;

  // used to access the SIMD array in a soa container
  // for non Soa Backends index_t will be const so it can be optimized out
  // by the compiler
  typename Backend::index_t idx_ = 0;
};

template <typename Base>
class Cell : public Base {
  BDM_CLASS_HEADER(Cell, Cell<typename Base::template Self<Backend>>, diameter_,
                   volume_, neurites_);

 public:
  Cell() {}

  const real_v& GetDiameter() const { return diameter_[idx_]; }

  void SetDiameter(const real_v& diameter) { diameter_[idx_] = diameter; }

  const real_v& GetVolume() const { return volume_[idx_]; }

  void SetVolume(const real_v& volume) { volume_[idx_] = volume; }

  void UpdateVolume() {
    for (size_t i = 0; i < Backend::kVecLen; i++) {
      volume_[idx_][i] = diameter_[idx_][i] * diameter_[idx_][i] *
                         diameter_[idx_][i] * 4 / 3 * 3.14;
    }
  }

  void ChangeVolume(const real_v& speed) {
    volume_[idx_] += speed * 0.01;
    volume_[idx_] =
        iif(volume_[idx_] < 5.2359877E-7, real_v(5.2359877E-7), volume_[idx_]);

    // UpdateDiameter();
    // for (size_t j = 0; j < Vc::double_v::Size; j++) {
    //   vc_diameter[idx][j] = std::cbrt(vc_volume[idx][j] * 6 / 3.14);
    // }
  }

  const SimdArray<std::vector<Neurite>>& GetNeurites() const {
    return neurites_[idx_];
  }

  void SetNeurites(const SimdArray<std::vector<Neurite>>& neurites) {
    neurites_[idx_] = neurites;
  }

  void UpdateNeurites() {
    for (auto& neurites : neurites_[idx_]) {
      for (auto& neurite : neurites) {
        neurite.id++;
      }
    }
  }

  friend std::ostream& operator<<(std::ostream& out, const Cell<Base>& cell) {
    out << "  cell : " << std::endl;
    out << "    diameter: \t" << cell.diameter_[cell.idx_] << std::endl;
    out << "    volume: \t" << cell.volume_[cell.idx_] << std::endl;

    out << "    neurites: \t";
    for (auto& neurites : cell.neurites_[cell.idx_]) {
      std::cout << "{";
      for (auto& neurite : neurites) {
        std::cout << neurite.id << ", ";
      }
      std::cout << "}, ";
    }

    return out;
  }

 private:
  // TODO change backend so Container takes Vc::Allocator if a
  // real_v is passed as template parameter
  Container<real_v, Vc::Allocator<real_v>> diameter_;
  Container<real_v, Vc::Allocator<real_v>> volume_;
  Container<SimdArray<std::vector<Neurite>>> neurites_;
};

template <typename T>
void ClientCodeExample(T* cells) {
  auto&& cell = (*cells)[0];
  cell.SetDiameter(Vc::double_v(34));
  cell.SetVolume(Vc::double_v(56));
  cell.ChangeVolume(Vc::double_v(7));
  cell.UpdateVolume();
  std::vector<Neurite> neurites_1;
  neurites_1.push_back(Neurite(987));
  neurites_1.push_back(Neurite(654));
  typename VcBackend::SimdArray<std::vector<Neurite>> neurites;
  neurites[1] = neurites_1;
  cell.SetNeurites(neurites);
  cell.UpdateNeurites();
  auto& cell_neurites = cell.GetNeurites();
}

template <typename Backend>
using MyCell = Cell<BaseCell<Backend>>;

int main(int argc, char** argv) {
  // vector cell
  std::cout << std::endl << "-----------------------------------" << std::endl;
  std::cout << "vector cell" << std::endl;
  MyCell<VcBackend> cell;
  std::cout << "initial vector cell " << std::endl << cell << std::endl;
  cell.SetDiameter(Vc::double_v(10));
  cell.SetVolume(Vc::double_v(12));
  cell.ChangeVolume(Vc::double_v(3));
  std::cout << "vector cell " << std::endl << cell << std::endl;
  cell.UpdateVolume();
  std::cout << "cell.GetVolume(): " << cell.GetVolume() << std::endl;
  std::vector<Neurite> neurites_0;
  neurites_0.push_back(Neurite(123));
  neurites_0.push_back(Neurite(456));
  typename VcBackend::SimdArray<std::vector<Neurite>> neurites;
  neurites[0] = neurites_0;
  cell.SetNeurites(neurites);
  cell.UpdateNeurites();
  auto& cell_neurites = cell.GetNeurites();
  std::cout << "VcBackend after operations " << std::endl << cell << std::endl;

  // different memory layout and client code
  std::cout << std::endl << "-----------------------------------" << std::endl;
  std::cout << "different memory layout and client code" << std::endl;
  std::cout << "Original VcBackend cell" << std::endl << cell << std::endl;
  MyCell<VcSoaBackend> soa_cells;
  soa_cells.push_back(cell);
  std::cout << "Vector cell stored in SOA memory layout" << std::endl;
  std::cout << soa_cells[0] << std::endl;
  ClientCodeExample(&soa_cells);
  std::cout << "after client code invocation" << std::endl;
  std::cout << soa_cells[0] << std::endl;

  std::vector<MyCell<VcBackend>, Vc::Allocator<MyCell<VcBackend>>> aosoa_cells;
  aosoa_cells.push_back(cell);
  std::cout << "Vector cell stored in AOSOA memory layout" << std::endl;
  std::cout << aosoa_cells[0] << std::endl;
  ClientCodeExample(&aosoa_cells);
  std::cout << "after client code invocation" << std::endl;
  std::cout << soa_cells[0] << std::endl;

  // scalar cell
  std::cout << std::endl << "-----------------------------------" << std::endl;
  std::cout << "scalar cell" << std::endl;
  auto scalar = MyCell<ScalarBackend>::NewScalar();
  std::cout << "initial scalar cell" << std::endl << scalar << std::endl;
  scalar.SetDiameter(10);
  scalar.SetVolume(91);
  scalar.ChangeVolume(3);
  std::cout << "scalar cell" << std::endl << scalar << std::endl;
  scalar.UpdateVolume();
  std::cout << "scalar_cell.GetVolume(): " << scalar.GetVolume() << std::endl;
  // TODO add assignment operator that allows: double v = s.GetVolume();
  double scalar_volume = scalar.GetVolume()[0];
  std::vector<Neurite> neurites_scalar_cell;
  neurites_scalar_cell.push_back(Neurite(123));
  neurites_scalar_cell.push_back(Neurite(456));
  scalar.SetNeurites(neurites_scalar_cell);
  scalar.UpdateNeurites();
  auto& scalar_neurites = cell.GetNeurites();
  std::cout << "final scalar cell" << std::endl << scalar << std::endl;

  // create Container SimdArray add and retrieve elements
  if (argc != 1 && argc != 4) {
    std::cout << "Usage: ./backend_arch #cells #iterations #threads"
              << std::endl;
  }
  if (argc == 4) {
    std::cout << std::endl;
    std::cout << "-------------------------------------------------"
              << std::endl;
    size_t cells;
    size_t iterations;
    size_t threads;
    std::istringstream(std::string(argv[1])) >> cells;
    std::istringstream(std::string(argv[2])) >> iterations;
    std::istringstream(std::string(argv[3])) >> threads;
    omp_set_num_threads(threads);
    TimingAggregator statistic;
    benchmarkPlainSoa(cells, iterations, &statistic);
    benchmarkSoaCell(cells, iterations, &statistic);
    benchmarkPlainAosoa(cells, iterations, &statistic);
    benchmarkAosoaCell(cells, iterations, &statistic);
    std::cout << statistic << std::endl;
  }
}

void benchmarkSoaCell(const size_t num_cells, const size_t iterations,
                      TimingAggregator* statistic) {
  const size_t N = num_cells / Vc::double_v::Size;
  MyCell<VcSoaBackend> cells;

  // initialization
  for (size_t i = 0; i < N; i++) {
    MyCell<VcBackend> cell;
    cell.SetDiameter(Vc::double_v(30));
    cell.SetVolume(Vc::double_v(0));
    cells.push_back(cell);
  }

  auto&& cells_ref = cells.GetSoaRef();

  {
    Timing timing("soaCell", statistic);
#pragma omp parallel for default(none) shared(cells_ref) \
    firstprivate(iterations)
    for (size_t i = 0; i < N; i++) {
      auto& cell = cells_ref[i];
      auto ifresult = cell.GetDiameter() <= 40;
      Vc::double_v dv(300);
      dv.setZeroInverted(ifresult);
      cell.ChangeVolume(dv);
    }
  }

  // verify results
  volatile double volume_sum = 0;
  for (size_t i = 0; i < N; i++) {
    volume_sum += cells_ref[i].GetVolume().sum();
  }
  assert(std::abs(volume_sum - N * Vc::double_v::Size * 3) < 1e-3);
}

void benchmarkPlainSoa(const size_t num_cells, const size_t iterations,
                       TimingAggregator* statistic) {
  const size_t N = num_cells / Vc::double_v::Size;

  class SoaCell {
   public:
    using real_v = Vc::double_v;

    SoaCell(const size_t elements) {
      diameter_.resize(elements);
      volume_.resize(elements);
      neurites_.resize(elements);
      for (size_t i = 0; i < elements; i++) {
        diameter_[i] = real_v(30);
        volume_[i] = real_v(0);
      }
    }

    SoaCell& operator[](const size_t idx) {
      idx_ = idx;
      return *this;
    }

    const real_v& GetDiameter() const { return diameter_[idx_]; }

    const real_v& GetVolume() const { return volume_[idx_]; }

    void ChangeVolume(const real_v& speed) {
      volume_[idx_] += speed * 0.01;
      volume_[idx_] = Vc::iif(volume_[idx_] < 5.2359877E-7,
                              Vc::double_v(5.2359877E-7), volume_[idx_]);

      // UpdateDiameter();
      // for (size_t j = 0; j < Vc::double_v::Size; j++) {
      //   vc_diameter[idx][j] = std::cbrt(vc_volume[idx][j] * 6 / 3.14);
      // }
    }

   private:
    size_t idx_ = 0;
    std::vector<real_v> diameter_;
    std::vector<real_v> volume_;
    std::vector<std::array<Neurite, real_v::Size>> neurites_;
  };

  // initialization in ctor
  SoaCell cells(N);

  {
    Timing timing("vcSoa", statistic);
#pragma omp parallel for default(none) shared(cells) firstprivate(iterations)
    for (size_t i = 0; i < N; i++) {
      auto& cell = cells[i];
      auto ifresult = cell.GetDiameter() <= 40;
      Vc::double_v dv(300);
      dv.setZeroInverted(ifresult);
      cell.ChangeVolume(dv);
    }
  }

  // verify results
  volatile double volume_sum = 0;
  for (size_t i = 0; i < N; i++) {
    volume_sum += cells[i].GetVolume().sum();
  }
  assert(std::abs(volume_sum - N * Vc::double_v::Size * 3) < 1e-3);
}

void benchmarkAosoaCell(const size_t num_cells, const size_t iterations,
                        TimingAggregator* statistic) {
  const size_t N = num_cells / Vc::double_v::Size;
  std::vector<MyCell<VcBackend>, Vc::Allocator<MyCell<VcBackend>>> cells;

  // initialization
  for (size_t i = 0; i < N; i++) {
    MyCell<VcBackend> cell;
    cell.SetDiameter(Vc::double_v(30));
    cell.SetVolume(Vc::double_v(0));
    cells.push_back(cell);
  }

  {
    Timing timing("aosoaCell", statistic);
#pragma omp parallel for default(none) shared(cells) firstprivate(iterations)
    for (size_t i = 0; i < N; i++) {
      auto& cell = cells[i];
      auto ifresult = cell.GetDiameter() <= 40;
      Vc::double_v dv(300);
      dv.setZeroInverted(ifresult);
      cell.ChangeVolume(dv);
    }
  }

  // verify results
  volatile double volume_sum = 0;
  for (size_t i = 0; i < N; i++) {
    volume_sum += cells[i].GetVolume().sum();
  }
  assert(std::abs(volume_sum - N * Vc::double_v::Size * 3) < 1e-3);
}

void benchmarkPlainAosoa(const size_t num_cells, const size_t iterations,
                         TimingAggregator* statistic) {
  const size_t N = num_cells / Vc::double_v::Size;

  class AosoaCell {
   public:
    using real_v = Vc::double_v;

    AosoaCell() : diameter_(real_v(30)), volume_(real_v(0)) {}

    const real_v& GetDiameter() const { return diameter_; }

    const real_v& GetVolume() const { return volume_; }

    void ChangeVolume(const real_v& speed) {
      volume_ += speed * 0.01;
      volume_ =
          Vc::iif(volume_ < 5.2359877E-7, Vc::double_v(5.2359877E-7), volume_);

      // UpdateDiameter();
      // for (size_t j = 0; j < Vc::double_v::Size; j++) {
      //   vc_diameter[idx][j] = std::cbrt(vc_volume[idx][j] * 6 / 3.14);
      // }
    }

   private:
    real_v diameter_;
    real_v volume_;
    std::array<std::vector<Neurite>, real_v::Size> neurites_;
  };

  // initiazation in ctor
  std::vector<AosoaCell, Vc::Allocator<AosoaCell>> cells(N);

  {
    Timing timing("vcAosoa", statistic);
#pragma omp parallel for default(none) shared(cells) firstprivate(iterations)
    for (size_t i = 0; i < N; i++) {
      auto& cell = cells[i];
      auto ifresult = cell.GetDiameter() <= 40;
      Vc::double_v dv(300);
      dv.setZeroInverted(ifresult);
      cell.ChangeVolume(dv);
    }
  }

  // verify results
  volatile double volume_sum = 0;
  for (size_t i = 0; i < N; i++) {
    volume_sum += cells[i].GetVolume().sum();
  }
  assert(std::abs(volume_sum - N * Vc::double_v::Size * 3) < 1e-3);
}
