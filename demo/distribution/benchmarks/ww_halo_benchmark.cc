#include "ww_halo_benchmark.h"

#include "distribution/common.h"
#include "TROOT.h"

zmqpp::context bdm::TestWWHaloData::ctx_{};
size_t bdm::TestWWHaloData::n_messages_(10000);
bdm::LoggingLevel bdm::TestWWHaloData::level_(bdm::LoggingLevel::kDebug);

int main(int argc, const char **argv) {
  ROOT::EnableThreadSafety();

  bdm::TestWWHaloData::n_messages_ =
      (argc >= 2 ? atoi(argv[1]) : bdm::TestWWHaloData::n_messages_);
  bdm::TestWWHaloData::level_ = ((argc == 2 && strcmp(argv[1], "-v") == 0) ||
                             (argc == 3 && strcmp(argv[2], "-v") == 0))
                                ? bdm::LoggingLevel::kDebug
                                : bdm::LoggingLevel::kInfo;

  std::thread worker1(bdm::Worker, "W1", bdm::CommunicatorId::kLeftNeighbour);
  std::thread worker2(bdm::Worker, "W2", bdm::CommunicatorId::kRightNeighbour);

  worker1.join();
  worker2.join();

  return 0;
}
