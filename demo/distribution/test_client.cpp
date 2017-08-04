#include <iostream>
#include <unistd.h>
#include <zmqpp/zmqpp.hpp>

#include "client.hpp"

using namespace mdp;

const size_t n_msg = 100000;

int main(int argc, char *argv[])
{
    bool verbose = (argc == 2 && strcmp(argv[1], "-v") == 0);

    zmqpp::context ctx;
    Client client = Client(&ctx, "tcp://localhost:5555", verbose);

    std::string command, worker = "W1";
    
    size_t remaining = n_msg;
    while (remaining > 0) {
        zmqpp::message msg; 
        msg.push_back("Hello world");

        client.Send(worker, msg);

        if ( !client.Recv(&command, nullptr, msg) ) {
            std::cout << "Interrupted..." << std::endl;
            break;
        }

        if (command == MDPC_NAK) {
            std::cout << "E: invalid worker " << worker << std::endl;
            sleep(HEARTBEAT_INTERVAL.count() / 1000);
            continue;
        }

        remaining--;
    }

    std::cout << n_msg  - remaining << " replies received!" << std::endl;

    return 0;
}
