#ifndef CORE_PARALLEL_EXECUTION_XML_UTIL_H
#define CORE_PARALLEL_EXECUTION_XML_UTIL_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "TString.h"

#include "core/util/io.h"
#include "core/util/log.h"

namespace bdm {

using XMLMap =
    std::unordered_map<std::string, std::unordered_map<std::string, double>>;

inline std::string ToLower(const std::string& str) {
  TString tstr(str);
  tstr.ToLower();
  return std::string(tstr.Data());
}

class XMLParamMap {
 public:
  XMLParamMap();

  double Get(const std::string& k1, const std::string& k2);

  void Print();

  void Set(const std::string& k1, const std::string& k2, double val);

 private:
  XMLMap data_;
  BDM_CLASS_DEF_NV(XMLParamMap, 1);
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_XML_UTIL_H
