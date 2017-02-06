// proof of concept to remove data members from an object if it is not needed
// saves memory and reduces unnecessary loads/stores
#include <iostream>
#include <string>
#include <typeinfo>

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
  friend ostream& operator<<(ostream& out, const Nulltype& value) {
    return out;
  }
  Nulltype& operator=(const Nulltype& other) { return *this; }
};

#define DATA_MEMBER(TYPE, NAME)                                         \
  static constexpr int getDataMemberUid##NAME() { return __COUNTER__; } \
  typename TMemberSelector<TYPE, self, getDataMemberUid##NAME()>::type NAME

//TODO preprocessor iterator
#define NEW_MEMBER_SELECTOR(NAME, CLASS, MEMBER_1)                \
  template <typename Type, typename EnclosingClass, int id>       \
  struct NAME {                                                   \
    typedef Nulltype type;                                        \
  };                                                              \
                                                                  \
  template <typename Type>                                        \
  struct NAME<Type, CLASS, CLASS::getDataMemberUid##MEMBER_1()> { \
    typedef Type type;                                            \
  };

// TODO define NEW_MEMBER_REMOVE

#define BDM_DEFAULT_TEMPLATE                                              \
  template <template <typename, typename, int> typename TMemberSelector = \
                SelectAllMembers>

// -----------------------------------------------------------------------------
// usage

// Simulation Object
BDM_DEFAULT_TEMPLATE
struct Foo {
  using self = Foo<>;

  DATA_MEMBER(double, member1) = 3.14;
  DATA_MEMBER(double, member2) = 3.14 * 2;

  template <typename T>
  Foo<TMemberSelector>& operator=(const T& src) {
    member1 = src.member1;
    member2 = src.member2;
    return *this;
  }

  friend ostream& operator<<(ostream& out, const Foo<TMemberSelector>& other) {
    out << "sizeof:   " << sizeof(other) << " - data_members: " << other.member1
        << " - " << other.member2;
    return out;
  }
};

NEW_MEMBER_SELECTOR(SelectMember1, Foo<>, member1);
NEW_MEMBER_SELECTOR(SelectMember2, Foo<>, member2);

int main() {
  Foo<> foo_full;              // Foo with all data members
  Foo<SelectMember1> foo_m1;   // Foo with only member1
  std::cout << foo_full << std::endl << foo_m1 << std::endl;

  foo_full.member1 *= 2;
  foo_full.member2 *= 3;
  foo_m1 = foo_full;  // 1 move instruction
  std::cout << foo_full << std::endl << foo_m1 << std::endl;
  // does not compile since foo_full has members that are not in foo_m1
  // intentional behavior
  // foo_full = foo_m1;

  Foo<> another_full_foo;
  another_full_foo.member1 = 1.2;
  another_full_foo.member2 = 3.4;
  foo_full = another_full_foo;  // 2 move instructions
  std::cout << another_full_foo << std::endl << foo_full << std::endl;

  Foo<SelectMember2> foo_m2;   // Foo with only member2
  foo_m2 = foo_full;           // 1 move instruction
  // foo_full = foo_m2;
  // foo_m1 = foo_m2;
  // foo_m2 = foo_m1;
}
