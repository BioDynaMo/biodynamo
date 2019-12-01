#ifndef CORE_PARALLEL_EXECUTION_XML_UTIL_H
#define CORE_PARALLEL_EXECUTION_XML_UTIL_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "core/util/io.h"
#include "core/util/log.h"

namespace bdm {

using XMLMap =
    std::unordered_map<std::string, std::unordered_map<std::string, double>>;

class XMLParamMap {
 public:
  XMLParamMap() {}

  double Get(std::string k1, std::string k2) {
    if (data_.find(k1) != data_.end()) {
      if (data_[k1].find(k2) != data_[k1].end()) {
        return data_[k1][k2];
      }
    }

    std::stringstream ss;
    ss << "The parameter [" << k1 << ", " << k2
       << "] was not found in the given XML file.";
    Log::Error("XMLParamMap", ss.str());
    exit(1);
  }

  void Print() {
    std::cout << "{\n";
    for (auto elem : data_) {
      std::cout << "\t" << elem.first << "{\n";
      for (auto elem2 : elem.second) {
        std::cout << "\t\t[" << elem2.first << ", " << elem2.second << "]\n";
      }
      std::cout << "\t}\n";
    }
    std::cout << "}\n" << std::endl;
  }

  void Set(std::string k1, std::string k2, double val) { data_[k1][k2] = val; }

 private:
  XMLMap data_;
  BDM_CLASS_DEF_NV(XMLParamMap, 1);
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_XML_UTIL_H
