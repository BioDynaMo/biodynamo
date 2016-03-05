#ifndef STL_UTIL_H_
#define STL_UTIL_H_

#include <list>
#include <array>
#include <unordered_map>
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

  /**
   * returns whether an element with Key key is stored in this map
   * @param map
   * @param key
   */
  template<typename K, typename V>
  static bool mapContains(const std::unordered_map<K, V>& map, const K& key) {
    return map.find(key) != map.end();
  }

  /**
   * returns -1 if val < 0
   *          0 if val == 0
   *          1 if val > 0
   */
  template <typename T>
  static int sgn(T val) {
      return (T(0) < val) - (val < T(0));
  }

 private:
  STLUtil() = delete;
};

#endif // STL_UTIL_H_
