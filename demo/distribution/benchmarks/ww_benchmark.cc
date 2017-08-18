#include "ww_benchmark.h"

zmqpp::context bdm::TestWWData::ctx_{};
size_t bdm::TestWWData::n_messages_(10000);
bool bdm::TestWWData::verbose_(false);

int main(int argc, const char **argv) {
  bdm::TestWWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : bdm::TestWWData::n_messages_);
  bdm::TestWWData::verbose_ = (argc == 2 && strcmp(argv[1], "-v") == 0) ||
                              (argc == 3 && strcmp(argv[2], "-v") == 0);

  std::thread rworker(bdm::RWorker);
  std::thread lworker(bdm::LWorker);

  rworker.join();
  lworker.join();

  return 0;
}
