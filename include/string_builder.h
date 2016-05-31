#ifndef STRING_BUILDER_H_
#define STRING_BUILDER_H_

#include <string>
#include <sstream>

namespace bdm {

using std::string;
using std::stringstream;

/**
 * This class is used as an equivalent to java.lang.StringBuilder.
 * It is mainly used to generate the json representation of the simulation state
 */
class StringBuilder {
 public:
  StringBuilder();

  StringBuilder(const StringBuilder &src);

  virtual ~StringBuilder();

  /**
   * Appends the given string to the current string builder
   * @param str string to append
   * @return returns itself
   */
  virtual StringBuilder& append(const string &str);

  /**
   * Sets the pointer to the second last character.
   * A consecutive call of append will overwrite the last char
   */
  virtual void overwriteLastCharOnNextAppend();

  /**
   *  Returns a string object with a copy of the current contents of the stream.
   */
  virtual string str() const;

 private:
  stringstream string_stream_;

  StringBuilder &operator=(const StringBuilder &) = delete;
};

}  // namespace bdm

#endif  // STRING_BUILDER_H_
