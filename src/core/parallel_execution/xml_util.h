#ifndef CORE_PARALLEL_EXECUTION_XML_UTIL_H
#define CORE_PARALLEL_EXECUTION_XML_UTIL_H

#include <cstdlib>
#include <sstream>
#include <string>
#include <unordered_map>

#include "core/util/log.h"

namespace bdm {

class XMLParamMap {
 public:
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

  void Set(std::string k1, std::string k2, double val) { data_[k1][k2] = val; }

 private:
  std::unordered_map<std::string, std::unordered_map<std::string, double>>
      data_;
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_XML_UTIL_H

