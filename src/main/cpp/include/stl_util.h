#ifndef STL_UTIL_H_
#define STL_UTIL_H_

#include <list>
#include <array>
#include <algorithm>

class STLUtil {
 public:
  /**
   * returns whether an element is contained in a std::list
   * Caution: linear runtime
   */
  template<typename C>
  static bool listContains(const std::list<C>& l, C element) {
    return std::find(l.begin(), l.end(), element) != l.end();
  }

  /**
   * copies the elements of the std::array into std::list
   */
  template<typename C, std::size_t N>
  static void arrayToList(const std::array<C, N>& arr, std::list<C>& list) {
    list.clear();
    for (auto el : arr) {
      list.push_back(el);
    }
  }
 private:
  STLUtil() = delete;
};

#endif // STL_UTIL_H_
