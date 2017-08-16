#include "ww_benchmark.h"

using namespace mdp;

zmqpp::context Info::ctx {};
size_t Info::n_messages (10000);
bool Info::verbose (false);

int main(int argc, char *argv[]) {
    Info::n_messages = (argc >= 2 ? atoi(argv[1]) : Info::n_messages);
    Info::verbose = (argc == 2 && strcmp(argv[1], "-v") == 0) ||
              (argc == 3 && strcmp(argv[2], "-v") == 0);

    std::thread rworker (RWorker);
    std::thread lworker (LWorker);

    rworker.join();
    lworker.join();

    return 0;
}

