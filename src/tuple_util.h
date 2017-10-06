#ifndef TUPLE_UTIL_H_
#define TUPLE_UTIL_H_

#include <type_traits>
#include <utility>

namespace bdm {

using std::size_t;

namespace detail {
// Inspiration taken from:
// https://stackoverflow.com/questions/21062864/optimal-way-to-access-stdtuple-element-in-runtime-by-index

/// Applies the given function on tuple element TIndex
/// This function is called from `detail::Apply`. It has a compile time index
/// to be used in `std::get` to obtain the right type within the tuple.
/// The obtained type is then passed to `function`.
template <typename TTuple, typename TFunction, size_t TIndex>
void ApplyImpl(TTuple* t, TFunction&& function) {
  return function(&std::get<TIndex>(*t));
}

/// Does the translation of runtime index to compile time index by using
/// a look up table (lut) array of `ApplyImpl` functions. Forwards the call to
/// ApplyImpl
template <typename TTuple, typename TFunction, size_t... TIndices>
void Apply(TTuple* t, size_t index, TFunction&& f,
           std::index_sequence<TIndices...>) {
  using ApplyImplSignature = void(TTuple*, TFunction &&);  // NOLINT
  // create lookup table that maps runtime index to right ApplyImpl function
  static constexpr ApplyImplSignature* kLut[] = {
      &ApplyImpl<TTuple, TFunction, TIndices>...};
  return kLut[index](t, f);
}

template <typename T, size_t Counter, typename... Types>
struct GetIndexImpl;

template <typename T, size_t Counter, typename FirstType>
struct GetIndexImpl<T, Counter, FirstType> {
  size_t static constexpr GetValue() { return Counter; }
};

template <typename T, size_t Counter, typename FirstType,
          typename... RemainingTypes>
struct GetIndexImpl<T, Counter, FirstType, RemainingTypes...> {
  size_t static constexpr GetValue() {
    // TODO(lukas) after changing to c++17: change to if constexpr (...)
    if (std::is_same<T, FirstType>::value) {
      return Counter;
    } else {
      return GetIndexImpl<T, Counter + 1, RemainingTypes...>::GetValue();
    }
  }
};

// template <typename TFunction, typename TTuple, typename TFirst>
// struct ReturnTypeImpl {
//   using type =
//   decltype(TFunction(std::get<TFirst>(&std::declval<TTuple>())));
// };
//
// template <typename TFunction, typename TTuple, typename TFirst, typename...
// TRemainingTypes>
// struct ReturnTypeImpl {
//   using type = type_ternary_operator<enable_if<
// };

}  // namespace detail

/// This function applies the given function on tuple element index.
/// The peculiarity is that index is a runtime parameter.
/// `std::get<N>(tuple)` however, requires a compile time constant.
/// Therefore, `Apply` performs the translation to a compile time index.
/// @param t std::tuple or similar type that supports std::get<N>
/// @param index runtime index specifying the type within t
/// @param f function that should be executed on the type
template <typename TTuple, typename TFunction>
void Apply(TTuple* t, size_t index, TFunction&& f) {
  detail::Apply(t, index, f,
                std::make_index_sequence<std::tuple_size<TTuple>::value>());
}

/// Return the index of the first occurence of type T within the variadic
/// template parameter Types.
template <typename T, typename... Types>
inline size_t GetIndex() {
  return bdm::detail::template GetIndexImpl<T, 0, Types...>::GetValue();
}

}  // namespace bdm

#endif  // TUPLE_UTIL_H_
