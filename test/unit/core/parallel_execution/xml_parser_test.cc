#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#include "core/parallel_execution/xml_parser.h"

#include <fstream>
#include <string>

namespace bdm {

class XMLParserTest : public ::testing::Test {
 public:
  static constexpr const char* kXMLFileName = "dummy.xml";
  static constexpr const char* kXMLContent =
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<model>\n"
      "  <simulation_objects>\n"
      "    <object>\n"
      "      <name>MyCell</name>\n"
      "      <population value_type=\"scalar\">8</population>\n"
      "      <type value_type=\"scalar\">0</type>\n"
      "      <mass value_type=\"set\">\n"
      "      \t<value>1</value>\n"
      "      \t<value>2</value>\n"
      "      \t<value>4</value>\n"
      "      </mass>\n"
      "      <diameter value_type=\"range\">\n"
      "        <min>10</min>\n"
      "        <max>30</max>\n"
      "        <stride>5</stride>\n"
      "      </diameter>\n"
      "      <adherence value_type=\"range\">\n"
      "        <min>0.0</min>\n"
      "        <max>1.0</max>\n"
      "        <stride>0.1</stride>\n"
      "      </adherence>\n"
      "      <velocity value_type=\"distribution_poisson\">\n"
      "\t      <mean>0</mean>\n"
      "      </velocity>\n"
      "    </object>\n"
      "  </simulation_objects>\n"
      "  <biology_modules>\n"
      "    <module>\n"
      "      <name>GrowDivide</name>\n"
      "      <growth_rate value_type=\"scalar\">4000</growth_rate>\n"
      "      <threshold value_type=\"range\">\n"
      "        <min>10</min>\n"
      "        <max>30</max>\n"
      "        <stride>5</stride>\n"
      "      </threshold>\n"
      "    </module>\n"
      "  </biology_modules>\n"
      "</model>\n"
      "";

  XMLParserTest() {
    remove(kXMLFileName);
    std::ofstream xml_file(kXMLFileName);
    xml_file << kXMLContent;
    xml_file.close();
    xp = new XMLParser(std::string(kXMLFileName));
  }

  TXMLNode* GetRootNode() { return xp->root_node_; }
  XMLParser* GetParser() { return xp; }

  bool IsValidNode(TXMLNode* n) { return xp->IsValidNode(n); }

  void CheckConstruction() {
    EXPECT_TRUE(xp->dom_parser_ != nullptr);
    EXPECT_EQ(0, xp->dom_parser_->GetValidate());
    EXPECT_EQ(0, xp->dom_parser_->GetParseCode());
    EXPECT_TRUE(xp->root_node_ != nullptr);
  }

  TXMLNode* GetNodeByName(TXMLNode* node, std::string node_name) {
    return xp->GetNodeByName(node, node_name);
  }

 private:
  XMLParser* xp;
};

TEST_F(XMLParserTest, Constructor) { CheckConstruction(); }

TEST_F(XMLParserTest, GetNodeByName) {
  TXMLNode* so = GetNodeByName(GetRootNode(), "simulation_objects");
  TXMLNode* bm = GetNodeByName(GetRootNode(), "biology_modules");
  TXMLNode* nan = GetNodeByName(GetRootNode(), "non-existing-node");

  EXPECT_EQ("simulation_objects", std::string(so->GetNodeName()));
  EXPECT_EQ("biology_modules", std::string(bm->GetNodeName()));
  EXPECT_TRUE(nan == nullptr);
}

TEST_F(XMLParserTest, IsValidNode) {
  TXMLNode* so = GetNodeByName(GetRootNode(), "simulation_objects");
  auto* node = so->GetChildren();
  for (; node; node = node->GetNextNode()) {
    // Skip the "text" nodes
    if (std::string(node->GetNodeName()) != std::string("text")) {
      EXPECT_TRUE(IsValidNode(node));
    }
  }
}

TEST_F(XMLParserTest, GetContainers) {
  auto ranges = GetParser()->GetContainer<Range>("range");
  EXPECT_EQ(3u, ranges.size());
  EXPECT_EQ(5, ranges[0].GetNumElements());
  EXPECT_EQ(11, ranges[1].GetNumElements());
  EXPECT_EQ(5, ranges[2].GetNumElements());

  auto sets = GetParser()->GetContainer<Set>("set");
  EXPECT_EQ(3, sets[0].GetNumElements());
  EXPECT_EQ(1, sets[0].GetValue(0));
  EXPECT_EQ(2, sets[0].GetValue(1));
  EXPECT_EQ(4, sets[0].GetValue(2));
}

}  // namespace bdm
