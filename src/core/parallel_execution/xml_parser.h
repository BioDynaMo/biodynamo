// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_PARALLEL_EXECUTION_XML_PARSER_H_
#define CORE_PARALLEL_EXECUTION_XML_PARSER_H_

#include <TDOMParser.h>
#include <TROOT.h>
#include <TXMLAttr.h>
#include <TXMLNode.h>

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/parallel_execution/util.h"
#include "core/param/param.h"
#include "core/util/log.h"

using std::vector;
using std::string;
using std::stod;

namespace bdm {

class XMLParserTest;

class XMLParser {
 public:
  XMLParser(std::string xml_file) {
    dom_parser_ = new TDOMParser();
    dom_parser_->SetValidate(false);
    dom_parser_->ParseFile(xml_file.c_str());
    root_node_ = dom_parser_->GetXMLDocument()->GetRootNode();
    valid_nodes_ = {"object",          "module", "simulation_objects",
                    "biology_modules", "world",  "substances",
                    "substance"};
  }

  template <typename TType>
  vector<TType> GetContainer(std::string type_name) {
    // Lambda to extract range value types from the XML file and store them in a
    // vector
    vector<TType> containers;
    // We will receive an TXMLNode* that contains an XML element with attribute
    // `type_name`. Its children will contain the min, max and stride values
    auto get_iterable = [&](TXMLNode *element, TXMLAttr *attr,
                            TXMLNode *parent) {
      if (HasValueOfType(attr, type_name)) {
        auto *xml_param = element->GetChildren();
        TType t;
        ExtractValues(&t, xml_param);
        containers.push_back(t);
      }
    };
    IterateTree(get_iterable);

    return containers;
  }

  /// Creates a map of the XML Parameters that a worker node needs to perform
  /// the simulation with
  XMLParamMap CreateMap(XMLParams *xml_params) {
    XMLParamMap ret;
    int r = 0, s = 0;
    // Lambda that populates the XMLParamsMap, and replaces range and set values
    // with the scalar values that this worker received from the master
    auto map_value = [&](TXMLNode *element, TXMLAttr *attr, TXMLNode *parent) {
      auto *pn = GetNodeByName(parent, "name");

      string parent_name = string(pn->GetText());
      if (GetValueType(attr) == string("range")) {
        // We take the value in the XMLParams that we assigned to us by master
        double scalar_val = xml_params->GetData()[0][r];
        ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
        r++;
      } else if (GetValueType(attr) == string("set")) {
        // We take the value in the XMLParams that we assigned to us by master
        double scalar_val = xml_params->GetData()[1][s];
        ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
        s++;
      } else if (GetValueType(attr) == string("scalar")) {
        double scalar_val = stod(element->GetText());
        ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
      } else if (GetValueType(attr) == string("distribution")) {
        // TODO: implement a switch case that samples random numbers based on
        // their distribution
      }
    };
    IterateTree(map_value);

    // Process the "world" node separtely because its structure in XML is not
    // the same as the other nodes
    auto *world_node = GetNodeByName(root_node_, "world");
    auto *node = world_node->GetChildren();
    while (node != nullptr && node->GetNextNode() != nullptr) {
      node = node->GetNextNode();
      if (node->GetText() == nullptr) {
        continue;
      }
      ret.Set("World", string(node->GetNodeName()), stod(node->GetText()));
    }
    return ret;
  }

 private:
  /// Return the value type of an XML attribute marked with "value_type". If
  /// not found, return an empty string
  std::string GetValueType(TXMLAttr *attr) {
    if (string(attr->GetName()) == "value_type") {
      return string(attr->GetValue());
    }
    return std::string();
  }

  /// Check if the XML attribute is marked with a "value_type" of type `query`
  bool HasValueOfType(TXMLAttr *attr, std::string query) {
    return GetValueType(attr) == query;
  }

  /// Check if `node` is one of the valid nodes listed in `valid_nodes_`
  bool IsValidNode(TXMLNode *node) {
    bool is_element = node->GetNodeType() == TXMLNode::kXMLElementNode;
    const auto &v = valid_nodes_;
    return is_element &&
           (std::find(v.begin(), v.end(), node->GetNodeName()) != v.end());
  }

  // Any XML element that has "value_type = range" is expected to be proceeded
  // by the `min`, `max` and `stride` values
  void ExtractValues(Range *r, TXMLNode *node) {
    std::string parent_name;
    for (; node; node = node->GetNextNode()) {
      parent_name = std::string(node->GetParent()->GetNodeName());
      auto v = node->GetNodeName();
      if (string(v) == "min") {
        r->min_ = stod(node->GetText());
      } else if (string(v) == "max") {
        r->max_ = stod(node->GetText());
      } else if (string(v) == "stride") {
        r->stride_ = stod(node->GetText());
      }
    }

    // Check if max >= min
    if (r->max_ < r->min_) {
      std::stringstream ss;
      ss << "We found a range type value where 'min (" << r->min_
         << ")' is set to be greater than 'max (" << r->max_
         << ")'. Please check your XML parameter file for parameter ["
         << parent_name << "]" << std::endl;
      Log::Fatal("ExtractValues<Range>", ss.str());
    }
  }

  // Any XML element that has "value_type = range" is expected to be proceeded
  // by the `min`, `max` and `stride` values
  void ExtractValues(Set *set, TXMLNode *node) {
    for (; node; node = node->GetNextNode()) {
      auto v = node->GetNodeName();
      if (string(v) == "value") {
        set->push_back(stod(node->GetText()));
      }
    }
  }

  template <typename Lambda>
  void IterateTree(Lambda action) {
    auto *parent_node = root_node_->GetChildren();
    while (parent_node != nullptr && parent_node->GetNextNode() != nullptr) {
      parent_node = parent_node->GetNextNode();
      if (IsValidNode(parent_node)) {
        auto *node = parent_node->GetChildren();
        for (; node; node = node->GetNextNode()) {
          if (IsValidNode(node)) {
            auto *param = node->GetChildren();
            for (; param; param = param->GetNextNode()) {
              if (param->HasAttributes()) {
                TList *attrList = param->GetAttributes();
                TIter iterate(attrList);
                TXMLAttr *attr;
                while ((attr = static_cast<TXMLAttr *>(iterate()))) {
                  action(param, attr, node);
                }
              }
            }
          }
        }
      }
    }
  }

  /// Iterate over the children of 'node' until node with name `node_name` is
  /// found. Return `nullptr` if node is not found
  TXMLNode *GetNodeByName(TXMLNode *node, std::string node_name) {
    TXMLNode *children = node->GetChildren();
    TXMLNode *child_node = children->GetNextNode();
    while (std::string(child_node->GetNodeName()) != node_name) {
      child_node = child_node->GetNextNode();
      if (child_node == nullptr) {
        break;
      }
    }
    return child_node;
  }

  TDOMParser *dom_parser_ = nullptr;
  TXMLNode *root_node_ = nullptr;
  std::vector<std::string> valid_nodes_;

  friend XMLParserTest;
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_XML_PARSER_H_
