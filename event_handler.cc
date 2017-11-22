#include <iostream>

constexpr int kDivision = 0;
constexpr int kBranching = 1;

struct A {};

struct Foo {
  template <int Event, typename... Args>
  int handle(Args... args);
};

template<>
int Foo::handle<kDivision>(A a, double d) {
  return 0;
}

template<>
int Foo::handle<kBranching>(int i) {
  return 1;
}

struct Default {
  template <int Event, typename... Args>
  int handle(Args... args) {
    return 99;
  }
};

struct Bar : public Foo {
  template <int Event, typename... Args>
  int handle(Args... args);
};

template<>
int Bar::handle<kBranching>(int i) {
  return 1123;
}

// need to forward to base class - otherwise: compile error
template<>
int Bar::handle<kDivision>(A a, double d) {
  return Foo::handle<kDivision>(a, d);
}

void run() {
  Foo f;
  std::cout << f.handle<0>(A(), 3.14) << std::endl;
  std::cout << f.handle<1>(1) << std::endl;

  Default d;
  std::cout << d.handle<12>(1, 2, 3) << std::endl;

  Bar b;
  std::cout << b.handle<0>(A(), 3.14) << std::endl;
  std::cout << b.handle<1>(1) << std::endl;
}
