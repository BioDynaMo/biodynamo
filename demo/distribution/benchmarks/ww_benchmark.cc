#include "ww_benchmark.h"

namespace bdm {

zmqpp::context TestWWData::ctx_{};
size_t TestWWData::n_messages_(10000);
bool TestWWData::verbose_(false);

int main(int argc, char *argv[]) {
  TestWWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : TestWWData::n_messages_);
  TestWWData::verbose_ = (argc == 2 && strcmp(argv[1], "-v") == 0) ||
                         (argc == 3 && strcmp(argv[2], "-v") == 0);

  std::thread rworker(RWorker);
  std::thread lworker(LWorker);

  rworker.join();
  lworker.join();

  return 0;
}
}  // namespace bdm
