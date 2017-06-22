#ifndef TUPLE_UTIL_H_
#define TUPLE_UTIL_H_

#include <utility>

namespace bdm {

namespace detail {
// Inspiration taken from:
// https://stackoverflow.com/questions/21062864/optimal-way-to-access-stdtuple-element-in-runtime-by-index

/// Applies the given function on tuple element TIndex
/// This function is called from `detail::Apply`. It has a compile time index
/// to be used in `std::get` to obtain the right type within the tuple.
/// The obtained type is then passed to `function`.
template <typename TTuple, typename TFunction, size_t TIndex>
void ApplyImpl(TTuple* t, TFunction&& function) {
  function(std::get<TIndex>(*t));
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
  kLut[index](t, f);
}

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

}  // namespace bdm

#endif  // TUPLE_UTIL_H_
