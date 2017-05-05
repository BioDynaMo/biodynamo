#include <iostream>

struct AA {};
struct BB {};

template <typename T>
struct Foo {

  template <typename TT=T>
  typename std::enable_if<std::is_same<TT, AA>::value>::type
  Bar() {
    std::cout << "version for AA" << std::endl;
  }

  template <typename TT=T>
  typename std::enable_if<std::is_same<TT, BB>::value>::type
  Bar() {
    std::cout << "version for BB" << std::endl;
  }
};

int main() {
  Foo<AA> foo;
  Foo<BB> foo_1;

  foo.Bar();
  foo_1.Bar();
}
