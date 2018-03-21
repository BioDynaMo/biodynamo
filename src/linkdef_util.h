#ifndef LINKDEF_UTIL_H_
#define LINKDEF_UTIL_H_

#include <iostream> // TODO remove
#include <cxxabi.h>
#include <functional>
#include <ostream>
#include <set>
#include <sstream>
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

  void GenerateLinkDefLine(std::vector<std::string>& linkdef_lines) const {
    auto demangled = Demangle(type_index_.name());
    if(!Filter(demangled)) {
      std::stringstream stream;
      stream << "#pragma link C++ class " << Demangle(type_index_.name())
             << (streamer_ ? "+;" : "-;");
      linkdef_lines.emplace_back(stream.str());
    }
  }

  friend std::ostream& operator<<(std::ostream& stream, const LinkDefDescriptor& ldd) {
    auto demangled = Demangle(ldd.type_index_.name());
    if(!Filter(demangled)) {
      stream << "#pragma link C++ class " << Demangle(ldd.type_index_.name())
             << (ldd.streamer_ ? "+;" : "-;");
    }
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
      ReplaceAllInString(ret_value, "bdm::SimulationObject>", "bdm::SimulationObject_TCTParam_TDerived>");
      free(result);
      return ret_value;
    }
    Fatal("LinkDefDescriptor::Demangle", "abi::__cxa_demangle returned non-zero exit code");
    return "";
  }

  static bool Filter(const std::string& demangled_type_name) {
    // TODO current solution is whitelisting the types. -> all types must be
    // in the namespace `bdm`.
    // Maybe better to switch to blacklisting
    return demangled_type_name.find("bdm::") == std::string::npos;
  }

  // TODO move to string util
  static void ReplaceAllInString(std::string& s, const std::string search, const std::string replace) {
	 size_t pos = s.find(search);
	 while( pos != std::string::npos){
	    s.replace(pos, search.size(), replace);
	    pos = s.find(search, pos + search.size());
	 }
   std::cout << s << std::endl;
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
/// TODO describe that T can be pointer, reference, ...
/// uses `std::decay_t` and `std::remove_pointer_t`
namespace detail {

/// NB: entries is of non-trivial type and can't be part of the variadic argument
/// ...
template<typename T>
void CallAddToLinkDef(std::set<LinkDefDescriptor>& entries, ...) {}

template<typename T>
auto CallAddToLinkDef(std::set<LinkDefDescriptor>& entries, int) -> decltype(std::remove_pointer_t<std::decay_t<T>>::AddToLinkDef(std::declval<std::set<LinkDefDescriptor>&>()), void()) {
  std::remove_pointer_t<std::decay_t<T>>::AddToLinkDef(entries);
}

}  // namespace detail

template<typename T>
void CallAddToLinkDef(std::set<LinkDefDescriptor>& entries) {
  ::bdm::detail::CallAddToLinkDef<T>(entries, 0);
}

// -----------------------------------------------------------------------------
// forward declaration
template <typename T>
void AddAllLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer);

namespace detail {

/// Default
template <typename T>
void AddAllLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer, ...) {
  std::cout << "add default  " << LinkDefDescriptor::Create<T>(true)  << std::endl;

  entries.insert(LinkDefDescriptor::Create<T>(true));
  ::bdm::CallAddToLinkDef<T>(entries);
}

/// for std::tuple
template <typename TupleType>
auto AddAllLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer, int) -> decltype(std::enable_if_t<is_tuple<TupleType>::value>(), void()) {
  std::cout << "add tuple    " << LinkDefDescriptor::Create<TupleType>(true) << std::endl;

  entries.insert(LinkDefDescriptor::Create<TupleType>(true));
  // runtime dispatch - TODO(lukas) replace with c++17 std::apply
  TupleType tuple;
  for (uint16_t i = 0; i < std::tuple_size<TupleType>::value; i++) {
    ::bdm::Apply(&tuple, i, [&](auto* container) {
      using ContainerType = decltype(container);
      entries.insert(LinkDefDescriptor::Create<ContainerType>(true));
      ::bdm::CallAddToLinkDef<ContainerType>(entries);
    });
  }
}

/// for std::vector
template <typename T>
auto AddAllLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer, int) -> decltype(std::enable_if_t<is_vector<std::remove_pointer_t<std::decay_t<T>>>::value>(), void()) {
  using VectorType = typename std::remove_pointer_t<std::decay_t<T>>;
  using value_type = std::remove_pointer_t<std::decay_t<typename VectorType::value_type>>;
  std::cout << "add vector   " << LinkDefDescriptor::Create<T>(true) << std::endl;
  std::cout << "  " << typeid(value_type).name() << std::endl;
  // entries.insert(LinkDefDescriptor::Create<value_type>(true));
  // CallAddToLinkDef<value_type>(entries);
  ::bdm::AddAllLinkDefEntries<value_type>(entries, streamer);
}

/// for bdm::Variant
template <typename T>
auto AddAllLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer, int) -> decltype(std::enable_if_t<is_Variant<std::remove_pointer_t<T>>::value>(), void()) {
  std::cout << "add variant  " << LinkDefDescriptor::Create<T>(false) << std::endl;
  std::cout << "  " << typeid(T).name() << std::endl;

  entries.insert(LinkDefDescriptor::Create<T>(false));
  ::bdm::CallAddToLinkDef<T>(entries);
}

}  // namespace detail

/// TODO documentation
/// recursive
template <typename T>
void AddAllLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer) {
  detail::AddAllLinkDefEntries<T>(entries, streamer, 0);
}


template <typename T>
void AddSelfToLinkDefEntries(std::set<LinkDefDescriptor>& entries, bool streamer) {
  entries.insert(LinkDefDescriptor::Create<T>(true));
}

// -----------------------------------------------------------------------------
/// Use this macro to add this type and all its depending types to the linkdef
/// file. It creates a lambda and registers it with `kAddToLinkDefFunctions`.
/// Automatic registration is performed with a static variable inside a template
/// specialization of `RegistrationHelper`. The use of `RegistrationHelper`
/// avoids the problem of generating a unique identifier across files.
/// e.g. `static unique_identifier = ::bdm::AddToLinkDefFunction(...);`
#ifdef BDM_CREATE_LINKDEF
#define BDM_ADD_TYPE_TO_LINKDEF(...) \
  template <typename> struct RegistrationHelper; \
  template <> struct RegistrationHelper<__VA_ARGS__> { \
    static int value; \
  }; \
  int RegistrationHelper<__VA_ARGS__>::value = ::bdm::AddToLinkDefFunction([](std::set<LinkDefDescriptor>& entries){ \
    ::bdm::AddAllLinkDefEntries<__VA_ARGS__>(entries, true); \
  });
#else
#define BDM_ADD_TYPE_TO_LINKDEF(...)
#endif   // BDM_CREATE_LINKDEF


}  // namespace bdm

#endif  // LINKDEF_UTIL_H_
