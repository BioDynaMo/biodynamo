#ifndef STL_UTIL_H_
#define STL_UTIL_H_

#include <list>
#include <array>
#include <unordered_map>
#include <algorithm>

namespace cx3d {

class STLUtil {
 public:
  /**
   * returns whether an element is contained in a std::list
   * Caution: linear runtime
   */
  template<typename C>
  static bool listContains(const std::list<C> &l, C element) {
    return std::find(l.begin(), l.end(), element) != l.end();
  }

  /**
   * copies the elements of the std::array into std::list
   */
  template<typename C, std::size_t N>
  static void arrayToList(const std::array<C, N> &arr, std::list<C> &list) {
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
  template<typename K, typename V, typename H, typename E>
  static bool mapContains(const std::unordered_map<K, V, H, E> &map, const K &key) {
    return map.find(key) != map.end();
  }

  /**
   * returns the associated value to this key if it is stored in the map, nullptr otherwise
   * @param map
   * @param key
   */
  template<typename K, typename V>
  static V* mapGet(const std::unordered_map<K, std::unique_ptr<V>>& map, const K& key) {
    auto it = map.find(key);
    if (it != map.end()) {
      return it->second.get();
    } else {
      return nullptr;
    }
  }

  /**
   * returns the associated value to this key if it is stored in the map, nullptr otherwise
   * @param map
   * @param key
   */
  template<typename K, typename V>
  static V* mapGet(const std::unordered_map<K, V*>& map, const K& key) {
    auto it = map.find(key);
    if (it != map.end()) {
      return it->second;
    } else {
      return nullptr;
    }
  }

  /**
   * returns the associated value to this key if it is stored in the map, default array otherwise
   * @param map
   * @param key
   */
  template<typename K, std::size_t N, typename H, typename E>
  static std::array<double, N> mapGet(const std::unordered_map<K, std::array<double, N>, H, E> &map, const K &key) {
    auto it = map.find(key);
    if (it != map.end()) {
      return it->second;
    } else {
      std::array<double, N> ret;
      return ret;
    }
  }

  /**
   * returns -1 if val < 0
   *          0 if val == 0
   *          1 if val > 0
   */
  template<typename T>
  static int sgn(T val) {
    return (T(0) < val) - (val < T(0));
  }

  /**
   * removes an element from a std::vector container
   */
  template<typename T>
  static void vectorRemove(std::vector<T> &vector, const T &el) {
    if (vector.empty()) {
      return;
    }
    auto it = std::find(vector.begin(), vector.end(), el);
    if (it != vector.end()) {
      vector.erase(it);
    }
  }

  /**
   * removes an element from a std::vector<std::unique_ptr<<T>> container
   */
  template<typename T>
  static void vectorRemove(std::vector<std::unique_ptr<T>> &vector, T* el) {
    if (vector.empty()) {
      return;
    }
    auto it = vector.begin();
    while (it != vector.end()) {
      if ((*it).get() == el) {
        break;
      }
      it++;
    }
    if (it != vector.end()) {
      vector.erase(it);
    }
  }

  /**
   * Function to statically downcast a unique pointer from a base type to its derived type
   * http://stackoverflow.com/questions/21174593/downcasting-unique-ptrbase-to-unique-ptrderived
   */
  template<typename Derived, typename Base>
  static std::unique_ptr<Derived> staticUPtrCast(std::unique_ptr<Base>&& p) {
    auto d = static_cast<Derived*>(p.release());
    return std::unique_ptr < Derived > (d);
  }

  /**
   * Function to dymamically downcast a unique pointer from a base type to its derived type
   * http://stackoverflow.com/questions/21174593/downcasting-unique-ptrbase-to-unique-ptrderived
   */
  template<typename Derived, typename Base>
  static std::unique_ptr<Derived> dynamicUPtrCast(std::unique_ptr<Base>&& p) {
    if (Derived* result = dynamic_cast<Derived*>(p.get())) {
      p.release();
      return std::unique_ptr < Derived > (result);
    }
    return std::unique_ptr < Derived > (nullptr);
  }

 private:
  STLUtil() = delete;
};

}  // namepspace cx3d

#endif // STL_UTIL_H_
