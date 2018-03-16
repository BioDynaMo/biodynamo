#ifndef LINKDEF_UTIL_H_
#define LINKDEF_UTIL_H_

#include <functional>
#include <set>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

namespace bdm {

struct LinkDefDescriptor {
  std::type_index type_index_;
  bool custom_streamer_ = false;

  bool operator<(const LinkDefDescriptor& other) const {
    return type_index_.hash_code() < other.type_index_.hash_code();
  }
};

static std::vector<std::function<void(std::set<LinkDefDescriptor>&)>> kAddToLinkDefFunctions;

template <typename TLambda>
int AddToLinkDefFunction(TLambda l) {
  kAddToLinkDefFunctions.push_back(l);
  return 0;
}

#define BDM_ADD_TYPE_TO_LINKDEF(type, with_streamer) \
  static int kRegisterFunction ## __FILE__ ## __LINE__ = ::bdm::AddToLinkDefFunction([](std::set<LinkDefDescriptor>& entries){ \
    entries.insert({typeid(type), with_streamer});\
    bdm::CallAddToLinkDef<type>(entries); \
  });

// -----------------------------------------------------------------------------
// has_AddToLinkDef typetrait
namespace detail {

template<typename T>
std::false_type has_AddToLinkDef(...);

template<typename T>
auto has_AddToLinkDef(int) -> decltype(T::AddToLinkDef(std::declval<std::set<LinkDefDescriptor>&>()), std::true_type());

}  // namespace detail

template <typename T>
struct has_AddToLinkDef : decltype(detail::has_AddToLinkDef<T>(0)) {};

// -----------------------------------------------------------------------------
// CallAddToLinkDefFunctor
template<typename T>
void CallAddToLinkDef(...) {}

template<typename T>
auto CallAddToLinkDef(std::set<LinkDefDescriptor>& entries) -> decltype(T::AddToLinkDef(std::declval<std::set<LinkDefDescriptor>&>()), void()) {
  T::AddToLinkDef(entries);
}

}  // namespace bdm

#endif  // LINKDEF_UTIL_H_
