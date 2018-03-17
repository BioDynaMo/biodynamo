#include <string>

// TODO rename to linkdef_generator_template.cc.in

// TODO add all the headers here
#include "figure_5.h"
#include "linkdef_util.h"

// const std::string demangle(const char* name) {
//   int status = -4;
//   char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
//   const char* const demangled_name = (status==0)?res:name;
//   std::string ret_val(demangled_name);
//   free(res);
//   return ret_val;
// }

using bdm::LinkDefDescriptor;

// static int test = bdm::AddToLinkDefFunction([](std::set<LinkDefDescriptor>& entries){
//   entries.insert({typeid(bdm::ResourceManager<>), true});
//   bdm::CallAddToLinkDef<bdm::ResourceManager<>>(entries);
//   std::cout << "RM inserted" << std::endl;
// });
BDM_ADD_TYPE_TO_LINKDEF(bdm::ResourceManager<>, true);

  const char* linkdef_header = R"LINKDEF_HEADER(// some C++ header definition
  #ifdef __ROOTCLING__
  // turns off dictionary generation for all
  #pragma link off all class;
  #pragma link off all function;
  #pragma link off all global;
  #pragma link off all typedef;
)LINKDEF_HEADER";

int main() {
  // std::cout << typeid(bdm::ResourceManager<>).name() << std::endl;

  std::set<LinkDefDescriptor> entries;
  // entries.insert({typeid(bdm::ResourceManager<>), true});

  // bdm::kAddToLinkDef.push_back([](std::set<LinkDefDescriptor>& entries){
  //   entries.insert({typeid(bdm::ResourceManager<>), true});
  // });
  for(auto&& function : bdm::kAddToLinkDefFunctions) {
    function(entries);
  }

  for(auto&& ld_entry : entries) {
    std::cout << ld_entry << std::endl;
  }

  // bdm::ResourceManager<>::Add
}


//root -b -q ../cmake/generate_linkdef/generate_linkdef_macro.c
// #include "cxxabi.h"
// #include <iostream>
//
// #define BDM_SRC_DIR ""
// #define thread_local ""
//
// const string demangle(const char* name) {
//   int status = -4;
//   char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
//   const char* const demangled_name = (status==0)?res:name;
//   string ret_val(demangled_name);
//   free(res);
//   return ret_val;
// }
//
// void generate_linkdef_macro() {
//   gInterpreter->Declare("#include \"../demo/figure_5.cc\"");
//   std::cout << "foo" << endl;
// }
