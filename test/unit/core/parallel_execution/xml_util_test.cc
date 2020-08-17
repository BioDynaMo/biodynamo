#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#include "core/parallel_execution/xml_util.h"
#include "core/param/command_line_options.h"
#include "core/simulation.h"

#include <fstream>
#include <string>

namespace bdm {

static constexpr const char* kXMLFileName = "dummy.xml";
static constexpr const char* kXMLContent =
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<model>\n"
    "  <simulation_objects>\n"
    "    <object>\n"
    "      <name>MyCell</name>\n"
    "      <diameter value_type=\"scalar\">5</diameter>\n"
    "    </object>\n"
    "  </simulation_objects>\n"
    "</model>\n"
    "";

TEST(XMLUtil, CaseInsensitivity) {
  remove(kXMLFileName);
  std::ofstream xml_file(kXMLFileName);
  xml_file << kXMLContent;
  xml_file.close();
  const char* argv[2] = {"./binary_name", "--xml=dummy.xml"};
  auto clo = CommandLineOptions(2, argv);
  Simulation simulation(&clo);

  EXPECT_EQ(kXMLFileName, clo.Get<std::string>("xml"));

  auto xmlp = simulation.GetXMLParam();
  double diameter = xmlp.Get("MyCell", "diameter");
  EXPECT_EQ(std::round(diameter), 5);
  double Diameter = xmlp.Get("MyCell", "Diameter");
  EXPECT_EQ(std::round(Diameter), 5);
}

}  // namespace bdm
