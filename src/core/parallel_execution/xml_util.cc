#include "core/parallel_execution/xml_util.h"

namespace bdm {

XMLParamMap::XMLParamMap() {}

double XMLParamMap::Get(const std::string& k1, const std::string& k2) {
  auto k1l = ToLower(k1);
  auto k2l = ToLower(k2);
  if (data_.find(k1l) != data_.end()) {
    if (data_[k1l].find(k2l) != data_[k1l].end()) {
      return data_[k1l][k2l];
    }
  }

  std::stringstream ss;
  ss << "The parameter [" << k1 << ", " << k2
     << "] was not found in the given XML file.";
  Log::Error("XMLParamMap", ss.str());
  exit(1);
}

void XMLParamMap::Print() {
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

void XMLParamMap::Set(const std::string& k1, const std::string& k2,
                      double val) {
  data_[ToLower(k1)][ToLower(k2)] = val;
}

}  // namespace bdm
