#ifndef COMMAND_LINE_OPTIONS_H_
#define COMMAND_LINE_OPTIONS_H_

#include <string>

namespace bdm {

/// struct holding the parsed command line option values
struct CommandLineOptions {
  std::string backup_file_ = "";
  std::string restore_file_ = "";
};

/// This function parses command line arguments using a default set of options
/// for simulations
CommandLineOptions DefaultSimulationOptionParser(int& argc, const char**& argv);

}  // namespace bdm

#endif
