#include <string>
#include "linkdef_util.h"

// TODO rename to linkdef_generator_template.cc.in

// TODO add all the headers here
#include "figure_5.h"


namespace bdm {

static int main_add_to_linkdef = AddToLinkDefFunction([](std::set<LinkDefDescriptor>& entries){
  AddAllLinkDefEntries<ResourceManager<>>(entries, true);
});

}  // namespace bdm

using bdm::LinkDefDescriptor;
// BDM_ADD_TYPE_TO_LINKDEF(bdm::ResourceManager<>, true);


  const char* linkdef_header = R"LINKDEF_HEADER(// some C++ header definition
  #ifdef __ROOTCLING__
  // turns off dictionary generation for all
  #pragma link off all class;
  #pragma link off all function;
  #pragma link off all global;
  #pragma link off all typedef;

)LINKDEF_HEADER";

  const char* linkdef_footer = "#endif";

int main() {
  // std::cout << typeid(bdm::ResourceManager<>).name() << std::endl;

  std::set<LinkDefDescriptor> entries;

  for(auto&& function : bdm::kAddToLinkDefFunctions) {
    function(entries);
  }

  std::vector<std::string> linkdef_lines;
  for(auto&& ld_entry : entries) {
    ld_entry.GenerateLinkDefLine(linkdef_lines);
  }

  std::sort(linkdef_lines.begin(), linkdef_lines.end());

  std::cout << linkdef_header << std::endl;

  for (auto&& line : linkdef_lines) {
    std::cout << "  " << line << std::endl;
  }

  std::cout << std::endl <<  linkdef_footer << std::endl;
}
