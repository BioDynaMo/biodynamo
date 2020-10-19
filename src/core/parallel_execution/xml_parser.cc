// // -----------------------------------------------------------------------------
// //
// // Copyright (C) The BioDynaMo Project.
// // All Rights Reserved.
// //
// // Licensed under the Apache License, Version 2.0 (the "License");
// // you may not use this file except in compliance with the License.
// //
// // See the LICENSE file distributed with this work for details.
// // See the NOTICE file distributed with this work for additional information
// // regarding copyright ownership.
// //
// // -----------------------------------------------------------------------------

// #include "core/parallel_execution/xml_parser.h"

// namespace bdm {

// XMLParser::XMLParser(std::string xml_file) {
//   dom_parser_ = new TDOMParser();
//   dom_parser_->SetValidate(false);
//   // Check if XML file exists
//   if (gSystem->AccessPathName(xml_file.c_str())) {
//     Log::Fatal("XMLParser", "XML file `", xml_file,
//                "' doesn't exists in working directory '",
//                gSystem->GetWorkingDirectory(), "'!");
//   }
//   dom_parser_->ParseFile(xml_file.c_str());
//   root_node_ = dom_parser_->GetXMLDocument()->GetRootNode();
//   valid_nodes_ = {"object",          "module", "simulation_objects",
//                   "biology_modules", "world",  "substances",
//                   "substance"};
// }

// // Creates a map of the XML Parameters that a worker node needs to perform
// /// the simulation with
// XMLParamMap XMLParser::CreateMap(XMLParams *xml_params) {
//   XMLParamMap ret;
//   int r = 0, s = 0, lr = 0;
//   // Lambda that populates the XMLParamsMap, and replaces range and set values
//   // with the scalar values that this worker received from the master
//   auto map_value = [&](TXMLNode *element, TXMLAttr *attr, TXMLNode *parent) {
//     auto *pn = GetNodeByName(parent, "name");

//     string parent_name = string(pn->GetText());
//     if (GetValueType(attr) == string("range")) {
//       // We take the value in the XMLParams that we assigned to us by master
//       double scalar_val = xml_params->GetData()[0][r];
//       ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
//       r++;
//     } else if (GetValueType(attr) == string("log_range")) {
//       // We take the value in the XMLParams that we assigned to us by master
//       double scalar_val = xml_params->GetData()[2][lr];
//       ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
//       lr++;
//     } else if (GetValueType(attr) == string("set")) {
//       // We take the value in the XMLParams that we assigned to us by master
//       double scalar_val = xml_params->GetData()[1][s];
//       ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
//       s++;
//     } else if (GetValueType(attr) == string("scalar")) {
//       double scalar_val = stod(element->GetText());
//       ret.Set(parent_name, string(element->GetNodeName()), scalar_val);
//     } else if (GetValueType(attr) == string("distribution")) {
//       // TODO: implement a switch case that samples random numbers based on
//       // their distribution
//     }
//   };
//   IterateTree(map_value);

//   // Process the "world" node separately because its structure in XML is not
//   // the same as the other nodes
//   if (auto *world_node = GetNodeByName(root_node_, "world")) {
//     auto *node = world_node->GetChildren();
//     while (node != nullptr && node->GetNextNode() != nullptr) {
//       node = node->GetNextNode();
//       if (node->GetText() == nullptr) {
//         continue;
//       }
//       ret.Set("World", string(node->GetNodeName()), stod(node->GetText()));
//     }
//   }
//   return ret;
// }

// std::string XMLParser::GetValueType(TXMLAttr *attr) {
//   if (string(attr->GetName()) == "value_type") {
//     return string(attr->GetValue());
//   }
//   return std::string();
// }

// bool XMLParser::HasValueOfType(TXMLAttr *attr, std::string query) {
//   return GetValueType(attr) == query;
// }

// bool XMLParser::IsValidNode(TXMLNode *node) {
//   bool is_element = node->GetNodeType() == TXMLNode::kXMLElementNode;
//   const auto &v = valid_nodes_;
//   return is_element &&
//          (std::find(v.begin(), v.end(), node->GetNodeName()) != v.end());
// }

// void XMLParser::ExtractValues(Range *r, TXMLNode *node) {
//   std::string parent_name;
//   for (; node; node = node->GetNextNode()) {
//     parent_name = std::string(node->GetParent()->GetNodeName());
//     auto v = node->GetNodeName();
//     if (string(v) == "min") {
//       r->min_ = stod(node->GetText());
//     } else if (string(v) == "max") {
//       r->max_ = stod(node->GetText());
//     } else if (string(v) == "stride") {
//       r->stride_ = stod(node->GetText());
//     }
//   }

//   // Check if max >= min
//   if (r->max_ < r->min_) {
//     std::stringstream ss;
//     ss << "We found a range type value where 'min (" << r->min_
//        << ")' is set to be greater than 'max (" << r->max_
//        << ")'. Please check your XML parameter file for parameter ["
//        << parent_name << "]" << std::endl;
//     Log::Fatal("ExtractValues<Range>", ss.str());
//   }
// }

// void XMLParser::ExtractValues(LogRange *r, TXMLNode *node) {
//   std::string parent_name;
//   for (; node; node = node->GetNextNode()) {
//     parent_name = std::string(node->GetParent()->GetNodeName());
//     auto v = node->GetNodeName();
//     if (string(v) == "base") {
//       r->base_ = stod(node->GetText());
//     } else if (string(v) == "min") {
//       r->min_ = stod(node->GetText());
//     } else if (string(v) == "max") {
//       r->max_ = stod(node->GetText());
//     } else if (string(v) == "stride") {
//       r->stride_ = stod(node->GetText());
//     }
//   }

//   // Check if max >= min
//   if (r->max_ < r->min_) {
//     std::stringstream ss;
//     ss << "We found a range type value where 'min (" << r->min_
//        << ")' is set to be greater than 'max (" << r->max_
//        << ")'. Please check your XML parameter file for parameter ["
//        << parent_name << "]" << std::endl;
//     Log::Fatal("ExtractValues<LogRange>", ss.str());
//   }
// }

// void XMLParser::ExtractValues(Set *set, TXMLNode *node) {
//   for (; node; node = node->GetNextNode()) {
//     auto v = node->GetNodeName();
//     if (string(v) == "value") {
//       set->push_back(stod(node->GetText()));
//     }
//   }
// }

// TXMLNode *XMLParser::GetNodeByName(TXMLNode *node, std::string node_name) {
//   TXMLNode *children = node->GetChildren();
//   TXMLNode *child_node = children->GetNextNode();
//   while (std::string(child_node->GetNodeName()) != node_name) {
//     child_node = child_node->GetNextNode();
//     if (child_node == nullptr) {
//       break;
//     }
//   }
//   return child_node;
// }

// }  // namespace bdm
