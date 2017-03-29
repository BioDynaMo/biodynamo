#ifndef MULTIFORM_OBJECT_H_
#define MULTIFORM_OBJECT_H_

#include <ostream>
#include <stdexcept>

#include "macros.h"
#include "simulation_object.h"

namespace bdm {

using std::ostream;

/// Type for removed data members - which can be optimized out by the compiler
template <typename T>
struct Nulltype {
  int empty[0] = {};

  Nulltype() {}

  template <typename TT>
  Nulltype(TT&& d) {}  // NOLINT(runtime/explicit)

  template <typename TT>
  Nulltype& operator=(const TT& other) {
    return *this;
  }

  /// required to mimic correct type if subscript operator is used on
  /// Nulltype data_member. Must be the same as decltype(T[0])
  /// SFINAE - if T doesnot have a subscript operator than this function is
  /// removed from the candidate set
  /// FIXME(lukas) invalidates robustness -> no compile errors if removed member
  /// is accessed
  template <typename TT = T>
  decltype(std::declval<TT>()[0]) operator[](std::size_t idx) const {
    return T()[0];
  }

  Nulltype(std::initializer_list<T>) {}

  /// required to accept initialization like `...) : position_{{0, 0, 0}}`
  template <typename TT>
  Nulltype(std::initializer_list<std::initializer_list<TT>>) {}

  friend ostream& operator<<(ostream& out, const Nulltype<T>& value) {
    return out;
  }
};

/// adds the partial template specialization to select one clazz-member pair
/// only for internal usage - will be called inside LOOP_3_1
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_SELECT_MEMBER(name, clazz, member)         \
  template <typename Type>                                  \
  struct name<Type, clazz, clazz::kDataMemberUid##member> { \
    typedef Type type;                                      \
  };

/// creates a new selector type
/// it only enables the specified data members if applied to a simulation object
/// others will be removed
#define NEW_MEMBER_SELECTOR(name, ...)                      \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Nulltype<Type> type;                            \
  };                                                        \
  EVAL(LOOP_3_1(INTERNAL_SELECT_MEMBER, name, __VA_ARGS__))

/// adds the partial template specialization to remove one clazz-member pair
/// only for internal usage - will be called inside LOOP_3_1
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_MEMBER_REMOVER(name, clazz, member)        \
  template <typename Type>                                  \
  struct name<Type, clazz, clazz::kDataMemberUid##member> { \
    typedef Nulltype<Type> type;                            \
  };

/// creates a new selector type
/// it removes the specified data members if applied to a simulation object
/// others will be kept -> inverse of NEW_MEMBER_SELECTOR
#define NEW_MEMBER_REMOVER(name, ...)                       \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Type type;                                      \
  };                                                        \
  EVAL(LOOP_3_1(INTERNAL_MEMBER_REMOVER, name, __VA_ARGS__))

/// \brief This struct is used to access static functions / members of a class
/// that
/// has template parameter(s) without a default value.
///
/// For example:
///
///     template <typename T> class Foo { static const int kBar = 3; };
///     Foo<PlaceholderType>::kBar
///
/// The definition of the static function / member must be invariant of the
/// template parameter(s)
/// e.g. `Foo<PlaceholderType>::kBar == Foo<AnyOtherType>::kBar`
/// Usage solely to create a valid id for the scope operator. It is not allowed
/// to instantiate an object with template parameter PlaceholderType. The
/// following statement is invalid and will throw an exception at runtime:
/// `Foo<PlaceholderType> foo;`
struct PlaceholderType : public SimulationObject<> {
  PlaceholderType() {
    throw std::logic_error(
        "Creating an instance of type PlaceholderType is not allowed. "
        "PlaceholderType should solely be used for creating a valid id "
        "for the scope operator");
  }
  template <class... A>
  explicit PlaceholderType(const A&... a) : PlaceholderType() {}
};

}  // namespace bdm

#endif  // MULTIFORM_OBJECT_H_
