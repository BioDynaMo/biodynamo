#ifndef TIMING_AGGREGATOR_H_
#define TIMING_AGGREGATOR_H_

#include <map>
#include <string>
#include <vector>

namespace bdm {

class TimingAggregator {
 public:
  TimingAggregator() {}
  ~TimingAggregator() {}

  void AddEntry(const std::string& key, int64_t value) {
    if (!timings_.count(key)) {
      std::vector<int64_t> data;
      data.push_back(value);
      timings_[key] = data;
    } else {
      timings_[key].push_back(value);
    }
  }

  void AddDescription(const std::string text) { descriptions_.push_back(text); }

 private:
  std::map<std::string, std::vector<int64_t>> timings_;
  std::vector<std::string> descriptions_;

  friend std::ostream& operator<<(std::ostream& os, const TimingAggregator& p);
};

inline std::ostream& operator<<(std::ostream& os, const TimingAggregator& ta) {
  if (ta.timings_.size() != 0) {
    std::vector<std::string> keys(ta.timings_.size());
    std::vector<std::vector<int64_t>> values(ta.timings_.size());
    size_t counter = 0;
    for (auto& timing : ta.timings_) {
      keys[counter] = timing.first;
      values[counter] = timing.second;
      counter++;
    }
    // print header
    if (ta.descriptions_.size() != 0) {
      os << "descriptions, ";
    }
    for (auto& key : keys) {
      os << key << ", ";
    }
    os << std::endl;
    // print table
    for (size_t i = 0; i < values[0].size(); i++) {
      if (i < ta.descriptions_.size()) {
        os << ta.descriptions_[i] << ", ";
      }
      for (auto& value : values) {
        os << value[i] << ", ";
      }
      os << std::endl;
    }
  }
  os << std::endl;
  return os;
}

}  // namespace bdm

#endif  // TIMING_AGGREGATOR_H_
