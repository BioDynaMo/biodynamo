#ifndef MULTIFORM_OBJECT_H_
#define MULTIFORM_OBJECT_H_

#include <ostream>
#include <stdexcept>

#include "preprocessor.h"
#include "simulation_object.h"

namespace bdm {

using std::ostream;

/// Type for removed data members - which can be optimized out by the compiler
struct Nulltype {
  int empty[0] = {};
  Nulltype() {}
  template <typename T>
  Nulltype(T&& d) {}  // NOLINT(runtime/explicit)
  template <typename T>
  Nulltype& operator=(const T& other) {
    return *this;
  }

  template <typename T>
  Nulltype(std::initializer_list<T>) {}

  friend ostream& operator<<(ostream& out, const Nulltype& value) {
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
    typedef Nulltype type;                                  \
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
    typedef Nulltype type;                                  \
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

/// This struct is used to access static functions / members of a class that
/// has template parameter(s) without a default value.
/// e.g.
/// ```
/// template <typename T> class Foo { static const int kBar = 3; };
/// Foo<PlaceholderType>::kBar
/// ```
/// The definition of the static function / member must be invariant of the
/// template parameter(s)
/// e.g. `Foo<PlaceholderType>::kBar == Foo<AnyOtherType>::kBar`
/// Usage solely to create a valid id for the scope operator. It is not allowed
/// to instantiate an object with template parameter PlaceholderType. The
/// following statement is invalid and will throw an exception at runtime:
/// `Foo<PlaceholderType> foo;`
struct PlaceholderType : public BdmSimObject<> {
  PlaceholderType() {
    throw std::logic_error(
        "Creating an instance of type PlaceholderType is not allowed. "
        "PlaceholderType should solely be used for creating a valid id "
        "for the scope operator");
  }
  template <class... A>
  explicit PlaceholderType(const A&... a)
      : PlaceholderType() {}
};

}  // namespace bdm

#endif  // MULTIFORM_OBJECT_H_
