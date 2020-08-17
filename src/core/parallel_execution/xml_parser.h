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

#include "core/parallel_execution/xml_util.h"
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
  XMLParser(std::string xml_file);

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
  XMLParamMap CreateMap(XMLParams *xml_params);

 private:
  /// Return the value type of an XML attribute marked with "value_type". If
  /// not found, return an empty string
  std::string GetValueType(TXMLAttr *attr);

  /// Check if the XML attribute is marked with a "value_type" of type `query`
  bool HasValueOfType(TXMLAttr *attr, std::string query);

  /// Check if `node` is one of the valid nodes listed in `valid_nodes_`
  bool IsValidNode(TXMLNode *node);

  // Any XML element that has "value_type = range" is expected to be proceeded
  // by the `min`, `max` and `stride` values
  void ExtractValues(Range *r, TXMLNode *node);

  // Any XML element that has "value_type = range" is expected to be proceeded
  // by the `min`, `max` and `stride` values
  void ExtractValues(LogRange *r, TXMLNode *node);

  // Any XML element that has "value_type = range" is expected to be proceeded
  // by the `min`, `max` and `stride` values
  void ExtractValues(Set *set, TXMLNode *node);

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
  TXMLNode *GetNodeByName(TXMLNode *node, std::string node_name);

  TDOMParser *dom_parser_ = nullptr;
  TXMLNode *root_node_ = nullptr;
  std::vector<std::string> valid_nodes_;

  friend XMLParserTest;
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_XML_PARSER_H_
