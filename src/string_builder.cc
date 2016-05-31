#include "string_builder.h"

namespace cx3d {

StringBuilder::StringBuilder()
    : string_stream_() {
  // fixes issue with decimal separator in floating points - sets it to "."
  std::locale::global(std::locale::classic());
}

StringBuilder::StringBuilder(const StringBuilder &src)
    : string_stream_() {
  string_stream_ << src.str();
}

StringBuilder::~StringBuilder() {
}

StringBuilder& StringBuilder::append(const string &str) {
  string_stream_ << str;
  return *this;
}

void StringBuilder::overwriteLastCharOnNextAppend() {
  string_stream_.seekp(-1, string_stream_.cur);
}

string StringBuilder::str() const {
  return string_stream_.str();
}

}  // namespace cx3d
