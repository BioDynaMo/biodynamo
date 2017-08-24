#include "bww_benchmark.h"

#include "TROOT.h"

zmqpp::context bdm::TestBWWData::ctx_{};
std::string bdm::TestBWWData::worker1_("W1");
std::string bdm::TestBWWData::worker2_("W2");
size_t bdm::TestBWWData::n_messages_(1000);
bool bdm::TestBWWData::verbose_(false);

int main(int argc, const char **argv) {
  ROOT::EnableThreadSafety();

  bdm::TestBWWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : bdm::TestBWWData::n_messages_);
  bdm::TestBWWData::verbose_ = (argc == 2 && strcmp(argv[1], "-v") == 0) ||
                               (argc == 3 && strcmp(argv[2], "-v") == 0);

  std::thread broker_t(bdm::TestBWWBrokerTask);
  std::thread worker1_t(bdm::TestBWWWorker1Task);
  std::thread worker2_t(bdm::TestBWWWorker2Task);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::thread client_t(bdm::TestBWWClientTask);

  client_t.join();
  worker1_t.join();
  worker2_t.join();

  return 0;
}
