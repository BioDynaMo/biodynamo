#include "cbw_benchmark.h"

namespace bdm {

zmqpp::context TestCBWData::ctx_{};
std::string TestCBWData::worker_("W1");
size_t TestCBWData::n_messages_(1000);
bool TestCBWData::verbose_(false);

int main(int argc, char *argv[]) {
  TestCBWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : TestCBWData::n_messages_);
  TestCBWData::verbose_ = (argc == 2 && strcmp(argv[1], "-v") == 0) ||
                          (argc == 3 && strcmp(argv[2], "-v") == 0);

  std::thread broker_t(BrokerTask);
  std::thread worker_t(WorkerTask);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::thread client_t(ClientTask);

  client_t.join();
  worker_t.join();

  return 0;
}

}  // namespace bdm
