// proof of concept to remove data members from an object if they are not needed
// saves memory and reduces unnecessary loads/stores
#include <iostream>
#include <string>
#include <typeinfo>

#include "cpp_magic.h"

using std::ostream;

template <typename Type, typename EnclosingClass, int id>
struct SelectAllMembers {
  typedef Type type;
};

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
  friend ostream& operator<<(ostream& out, const Nulltype& value) {
    return out;
  }
};

/// loops over variadic macro arguments and calls the specified operation
/// removes the first three parameters in each iteration, but adds the first one
/// again for the next call
/// e.g. LOOP(OP, a, b, c, d, e) will lead to:
/// OP(a, b, c)
/// OP(a, d, e)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP(operation, first, second, third, ...)           \
  operation(first, second, third)                            \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP)()(operation, first, __VA_ARGS__))
#define _LOOP() LOOP
// clang-format on

/// adds the partial template specialization to select one clazz-member pair
/// only for internal usage - will be called inside LOOP
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_SELECT_MEMBER(name, clazz, member)             \
  template <typename Type>                                      \
  struct name<Type, clazz, clazz::getDataMemberUid##member()> { \
    typedef Type type;                                          \
  };

/// creates a new selector type
/// it only enables the specified data members if applied to a simulation object
/// others will be removed
#define NEW_MEMBER_SELECTOR(name, ...)                      \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Nulltype type;                                  \
  };                                                        \
  EVAL(LOOP(INTERNAL_SELECT_MEMBER, name, __VA_ARGS__))

/// adds the partial template specialization to remove one clazz-member pair
/// only for internal usage - will be called inside LOOP
/// @param name:    selector name
/// @param clazz:   clazz of the data member
/// @param member:  data member name
#define INTERNAL_MEMBER_REMOVER(name, clazz, member)            \
  template <typename Type>                                      \
  struct name<Type, clazz, clazz::getDataMemberUid##member()> { \
    typedef Nulltype type;                                      \
  };

/// creates a new selector type
/// it removes the specified data members if applied to a simulation object
/// others will be kept -> inverse of NEW_MEMBER_SELECTOR
#define NEW_MEMBER_REMOVER(name, ...)                       \
  template <typename Type, typename EnclosingClass, int id> \
  struct name {                                             \
    typedef Type type;                                      \
  };                                                        \
  EVAL(LOOP(INTERNAL_MEMBER_REMOVER, name, __VA_ARGS__))

#define BDM_DEFAULT_TEMPLATE                                           \
  template <template <typename, typename, int> class TMemberSelector = \
                SelectAllMembers>

/// Macro to define data member for a simulation object
/// Hides complexity needed to conditionally remove the data member
#define BDM_DATA_MEMBER(type_name, var_name)                                \
  static constexpr int getDataMemberUid##var_name() { return __COUNTER__; } \
  typename TMemberSelector<type_name, self,                                 \
                           getDataMemberUid##var_name()>::type var_name

// -----------------------------------------------------------------------------
// usage

// Simulation Object
BDM_DEFAULT_TEMPLATE
struct Foo {
  using self = Foo<>;

  Foo() {}

  BDM_DATA_MEMBER(double, member1) = 3.14;
  BDM_DATA_MEMBER(double, member2) = 3.14 * 2;
  BDM_DATA_MEMBER(double, member3) = 3.14 * 3;

  template <typename T>
  Foo<TMemberSelector>& operator=(const T& src) {
    member1 = src.member1;
    member2 = src.member2;
    member3 = src.member3;
    return *this;
  }

  void IncrementMember2() { member2 += 2; }

  friend ostream& operator<<(ostream& out, const Foo<TMemberSelector>& other) {
    out << "sizeof:   " << sizeof(other) << " - data_members: " << other.member1
        << " - " << other.member2 << " - " << other.member3;
    return out;
  }
};

NEW_MEMBER_SELECTOR(SelectMember1, Foo<>, member1);
NEW_MEMBER_SELECTOR(SelectMember2, Foo<>, member2);
NEW_MEMBER_SELECTOR(SelectMember1And2, Foo<>, member1, Foo<>, member2);
NEW_MEMBER_REMOVER(RemoveMember2, Foo<>, member2);

int main() {
  Foo<> foo_full;             // Foo with all data members
  Foo<SelectMember1> foo_m1;  // Foo with only member1
  std::cout << foo_full << std::endl << foo_m1 << std::endl;

  foo_full.member1 += 2;
  foo_full.member2 += 3;
  foo_full.member2 += 4;
  foo_m1 = foo_full;  // 1 move instruction
  std::cout << foo_full << std::endl << foo_m1 << std::endl;
  // does not compile since foo_full has members that are not in foo_m1
  // intentional behavior
  // foo_full = foo_m1;
  // foo_m1.IncrementMember2();

  Foo<> another_full_foo;
  another_full_foo.member1 = 1.2;
  another_full_foo.member2 = 3.4;
  another_full_foo.member2 = 5.6;
  foo_full = another_full_foo;  // 3 move instructions
  std::cout << another_full_foo << std::endl << foo_full << std::endl;

  Foo<SelectMember2> foo_m2;  // Foo with only member2
  foo_m2 = foo_full;          // 1 move instruction
  // foo_full = foo_m2;
  // foo_m1 = foo_m2;
  // foo_m2 = foo_m1;

  Foo<SelectMember1And2> foo_m12;  // Foo with member 1 and member2
  foo_m12 = another_full_foo;      // 2 move instructions
  std::cout << foo_m12 << std::endl;

  foo_m1 = foo_m12;

  Foo<RemoveMember2> foo_without_m2;
  std::cout << "Foo<RemoveMember2>: " << foo_without_m2 << std::endl;
  // won't compile since member2 has been removed
  // double m2 = foo_without_m2.member2;
  // CAUTION: this will compile
  auto auto_m2 = foo_without_m2.member2;
  // but following statement will throw compile error -> does not go unnoticed
  // double m2_inc = auto_m2 + 1;

  foo_m1 = foo_without_m2;
  // won't compile - no member2 in foo_without_m2 but in foo_m12
  // foo_m12 = foo_without_m2;
}
