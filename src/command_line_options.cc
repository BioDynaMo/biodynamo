#include "command_line_options.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <TError.h>
#include "OptionParser.h"


namespace bdm {

CommandLineOptions DefaultSimulationOptionParser(int& argc, const char**& argv) {
  auto binary_name = argv[0];

  // NB: parser does not work without these statement
  // skip program name argv[0] if present
  argc -= (argc > 0);
  argv += (argc > 0);

  enum OptionIndex { kUnknown, kRestoreFilename, kBackupFilename, kHelp };
  enum OptionTypes { kNoType, kString };

  std::stringstream simulation_usage_stream;
  simulation_usage_stream << "Start " << binary_name << " simulation.\n";
  simulation_usage_stream << "Usage: " << binary_name << " [options]\n\n";
  simulation_usage_stream << "Options:\n";

  const char* simulation_usage = simulation_usage_stream.str().c_str();

  const char* restore_file_usage =
      "-r, --restore\tfilename\n"
      "    Restores the simulation from the checkpoint found in filename and\n"
      "    continues simulation from that point.\n";

  const char* backup_file_usage =
      "-b, --backup\tfilename\n"
      "    Periodically create full simulation backup to the specified file\n"
      "    NOTA BENE: File will be overriden if it exists\n";

  // clang-format off
  const ROOT::option::Descriptor simulation_usage_descriptor[] = {
      {
        kUnknown,
        kNoType,
        "", "",
        ROOT::option::Arg::None,
        simulation_usage
      },

      {
        kRestoreFilename,
        kString,
        "r", "restore",
        ROOT::option::FullArg::Required,
        restore_file_usage
      },

      {
        kBackupFilename,
        kString,
        "b", "backup",
        ROOT::option::FullArg::Required,
        backup_file_usage
      },

      {
        kHelp,
        kNoType,
        "h", "help",
        ROOT::option::Arg::None,
        "--help\n    Print usage and exit.\n"
      },
      {0, 0, 0, 0, 0, 0}
    };
  // clang-format on

  ROOT::option::Stats stats(simulation_usage_descriptor, argc, argv);
  ROOT::option::Option options[stats.options_max], buffer[stats.buffer_max];
  ROOT::option::Parser parse(simulation_usage_descriptor, argc, argv, options,
                             buffer);

  if (parse.error()) {
    Error(0, "Argument parsing error!\n");
    ROOT::option::printUsage(std::cout, simulation_usage_descriptor);
    exit(1);
  }

  // Print help if needed
  if (options[kHelp]) {
    ROOT::option::printUsage(std::cout, simulation_usage_descriptor);
    exit(0);
  }

  CommandLineOptions cl_options;

  if (options[kRestoreFilename]) {
    cl_options.restore_file_ = options[kRestoreFilename].arg;
    // TODO(lukas) check file ending
  }

  if (options[kBackupFilename]) {
    cl_options.backup_file_ = options[kBackupFilename].arg;
    // TODO(lukas) check file ending
  }

  return cl_options;
}

}  // namespace bdm
