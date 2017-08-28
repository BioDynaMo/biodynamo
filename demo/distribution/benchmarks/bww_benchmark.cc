#include "bww_benchmark.h"

#include "TROOT.h"

zmqpp::context bdm::TestBWWData::ctx_{};
std::string bdm::TestBWWData::worker1_("W1");
std::string bdm::TestBWWData::worker2_("W2");
size_t bdm::TestBWWData::n_messages_(10000);
bdm::LoggingLevel bdm::TestBWWData::level_(LoggingLevel::kDebug);

int main(int argc, const char **argv) {
  ROOT::EnableThreadSafety();

  bdm::TestBWWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : bdm::TestBWWData::n_messages_);

  bdm::TestBWWData::level_ = ((argc == 2 && strcmp(argv[1], "-v") == 0) ||
                              (argc == 3 && strcmp(argv[2], "-v") == 0))
                                 ? bdm::LoggingLevel::kDebug
                                 : bdm::LoggingLevel::kInfo;

  std::thread client_t(bdm::TestBWWClientTask);
  std::thread broker_t(bdm::TestBWWBrokerTask);
  std::thread worker1_t(bdm::TestBWWWorker1Task);
  std::thread worker2_t(bdm::TestBWWWorker2Task);

  client_t.join();
  worker1_t.join();
  worker2_t.join();
  broker_t.join();

  return 0;
}
