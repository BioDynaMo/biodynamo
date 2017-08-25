#include "cbw_benchmark.h"

#include "TROOT.h"

zmqpp::context bdm::TestCBWData::ctx_{};
std::string bdm::TestCBWData::worker_("W1");
size_t bdm::TestCBWData::n_messages_(1000);
bdm::LoggingLevel bdm::TestCBWData::level_(LoggingLevel::kDebug);

int main(int argc, const char **argv) {
  ROOT::EnableThreadSafety();

  bdm::TestCBWData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : bdm::TestCBWData::n_messages_);
  bdm::TestCBWData::level_ = ((argc == 2 && strcmp(argv[1], "-v") == 0) ||
                              (argc == 3 && strcmp(argv[2], "-v") == 0))
                                 ? bdm::LoggingLevel::kDebug
                                 : bdm::LoggingLevel::kInfo;

  std::thread broker_t(bdm::BrokerTask);
  std::thread worker_t(bdm::WorkerTask);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::thread client_t(bdm::ClientTask);

  client_t.join();
  worker_t.join();

  return 0;
}
