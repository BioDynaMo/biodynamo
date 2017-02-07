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
  Nulltype(T&& d) {}
  template <typename T>
  Nulltype& operator=(const T& other) { return *this; }
  friend ostream& operator<<(ostream& out, const Nulltype& value) {
    return out;
  }
};

#define MAP1(m, first, second, third, ...) \
  m(first, second, third)                  \
      IF(HAS_ARGS(__VA_ARGS__))(DEFER2(_MAP1)()(m, first, __VA_ARGS__))
#define _MAP1() MAP1

#define DATA_MEMBER(TYPE, NAME)                                         \
  static constexpr int getDataMemberUid##NAME() { return __COUNTER__; } \
  typename TMemberSelector<TYPE, self, getDataMemberUid##NAME()>::type NAME

// TODO internal - implementation detail - #undef??
#define MEMBER_ITERATOR(NAME, CLASS, MEMBER)                    \
  template <typename Type>                                      \
  struct NAME<Type, CLASS, CLASS::getDataMemberUid##MEMBER()> { \
    typedef Type type;                                          \
  };

#define NEW_MEMBER_SELECTOR(NAME, ...)                      \
  template <typename Type, typename EnclosingClass, int id> \
  struct NAME {                                             \
    typedef Nulltype type;                                  \
  };                                                        \
  EVAL(MAP1(MEMBER_ITERATOR, NAME, __VA_ARGS__))

// TODO define NEW_MEMBER_REMOVE

#define BDM_DEFAULT_TEMPLATE                                              \
  template <template <typename, typename, int> class TMemberSelector = \
                SelectAllMembers>

// -----------------------------------------------------------------------------
// usage

// Simulation Object
BDM_DEFAULT_TEMPLATE
struct Foo {
  using self = Foo<>;

  Foo() {}

  DATA_MEMBER(double, member1) = 3.14;
  DATA_MEMBER(double, member2) = 3.14 * 2;
  DATA_MEMBER(double, member3) = 3.14 * 3;

  template <typename T>
  Foo<TMemberSelector>& operator=(const T& src) {
    member1 = src.member1;
    member2 = src.member2;
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
}
