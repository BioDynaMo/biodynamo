#include "bww_benchmark.h"

namespace bdm {

zmqpp::context TestBWWData::ctx_{};
std::string TestBWWData::worker1_("W1");
std::string TestBWWData::worker2_("W2");
size_t TestBWWData::n_messages_(1000);
bool TestBWWData::verbose_(false);

int main(int argc, char *argv[]) {
  TestBWWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : TestBWWData::n_messages_);
  TestBWWData::verbose_ = (argc == 2 && strcmp(argv[1], "-v") == 0) ||
                          (argc == 3 && strcmp(argv[2], "-v") == 0);

  std::thread broker_t(TestBWWBrokerTask);
  std::thread worker1_t(TestBWWWorker1Task);
  std::thread worker2_t(TestBWWWorker2Task);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::thread client_t(TestBWWClientTask);

  client_t.join();
  worker1_t.join();
  worker2_t.join();

  return 0;
}
}  // namespace bdm
