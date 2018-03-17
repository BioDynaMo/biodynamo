#ifndef LINKDEF_UTIL_H_
#define LINKDEF_UTIL_H_

#include <cxxabi.h>
#include <functional>
#include <ostream>
#include <set>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

#include <TError.h>

#include "tuple_util.h"
#include "type_util.h"

namespace bdm {

class LinkDefDescriptor {
public:
  /// This function decays and removes the pointer of the given type before
  /// storing it in `type_index`.
  /// @return LinkDefDescriptor object
  template <typename T>
  static LinkDefDescriptor Create(bool streamer) {
    LinkDefDescriptor ldd;
    ldd.type_index_ =  typeid(std::remove_pointer_t<std::decay_t<T>>);
    ldd.streamer_ = streamer;
    return ldd;
  }

  bool operator<(const LinkDefDescriptor& other) const {
    return type_index_.hash_code() < other.type_index_.hash_code();
  }

  friend std::ostream& operator<<(std::ostream& stream, const LinkDefDescriptor& ldd) {
    stream << "#pragma link C++ class " << Demangle(ldd.type_index_.name())
           << (ldd.streamer_ ? "+;" : ";");
    return stream;
  }

private:
  std::type_index type_index_;
  bool streamer_ = false;

  /// Default constructor setting type_index to its type
  /// Will be overwritten by function `Create`
  LinkDefDescriptor() : type_index_(typeid(LinkDefDescriptor)) {}

  static const std::string Demangle(const char* name) {
    int status = -1;
    char* result = abi::__cxa_demangle(name, NULL, NULL, &status);
    if (!status) {
      std::string ret_value(result);
      free(result);
      return ret_value;
    }
    Fatal("LinkDefDescriptor::Demangle", "abi::__cxa_demangle returned non-zero exit code");
    return "";
  }
};


// -----------------------------------------------------------------------------
static std::vector<std::function<void(std::set<LinkDefDescriptor>&)>> kAddToLinkDefFunctions;

template <typename TLambda>
int AddToLinkDefFunction(TLambda l) {
  kAddToLinkDefFunctions.push_back(l);
  return 0;
}

// -----------------------------------------------------------------------------
// has_AddToLinkDef typetrait
// TODO rename to has_AddToLinkDefFn
namespace detail {

template<typename T>
std::false_type has_AddToLinkDef(...);

template<typename T>
auto has_AddToLinkDef(int) -> decltype(T::AddToLinkDef(std::declval<std::set<LinkDefDescriptor>&>()), std::true_type());

}  // namespace detail

template <typename T>
struct has_AddToLinkDef : decltype(detail::has_AddToLinkDef<T>(0)) {};

// -----------------------------------------------------------------------------
template<typename T>
void CallAddToLinkDef(...) {}

template<typename T>
auto CallAddToLinkDef(std::set<LinkDefDescriptor>& entries) -> decltype(T::AddToLinkDef(std::declval<std::set<LinkDefDescriptor>&>()), void()) {
  T::AddToLinkDef(entries);
}

// -----------------------------------------------------------------------------
namespace detail {

/// Default
template <typename T>
void AddLinkDefEntry(std::set<LinkDefDescriptor>& entries, bool streamer, ...) {
  entries.insert(LinkDefDescriptor::Create<T>(true));
  CallAddToLinkDef<T>(entries);
}

/// for std::tuple
template <typename TupleType>
auto AddLinkDefEntry(std::set<LinkDefDescriptor>& entries, bool streamer, int) -> decltype(std::enable_if_t<is_tuple<TupleType>::value>(), void()) {
  entries.insert(LinkDefDescriptor::Create<TupleType>(true));
  // runtime dispatch - TODO(lukas) replace with c++17 std::apply
  TupleType tuple;
  for (uint16_t i = 0; i < std::tuple_size<TupleType>::value; i++) {
    ::bdm::Apply(&tuple, i, [&](auto* container) {
      using ContainerType = decltype(container);
      entries.insert(LinkDefDescriptor::Create<ContainerType>(true));
      CallAddToLinkDef<std::decay_t<ContainerType>>(entries);
    });
  }
}

/// for std::vector
template <typename T>
auto AddLinkDefEntry(std::set<LinkDefDescriptor>& entries, bool streamer, int) -> decltype(std::enable_if_t<is_vector<T>::value>(), void()) {
  using value_type = typename T::value_type;
  entries.insert(LinkDefDescriptor::Create<value_type>(true));
  CallAddToLinkDef<value_type>(entries);
}

/// for bdm::Variant
template <typename T>
auto AddLinkDefEntry(std::set<LinkDefDescriptor>& entries, bool streamer, int) -> decltype(std::enable_if_t<is_Variant<T>::value>(), void()) {
  entries.insert(LinkDefDescriptor::Create<T>(false));
  CallAddToLinkDef<T>(entries);
}

}  // namespace detail

/// TODO documentation
template <typename T>
void AddLinkDefEntry(std::set<LinkDefDescriptor>& entries, bool streamer) {
  detail::AddLinkDefEntry<T>(entries, streamer, 0);
}

// -----------------------------------------------------------------------------
#define BDM_ADD_TYPE_TO_LINKDEF(type, streamer) \
  static int kRegisterFunction ## __FILE__ ## __LINE__ = ::bdm::AddToLinkDefFunction([](std::set<LinkDefDescriptor>& entries){ \
    ::bdm::AddLinkDefEntry<type>(entries, streamer); \
  });




}  // namespace bdm

#endif  // LINKDEF_UTIL_H_
