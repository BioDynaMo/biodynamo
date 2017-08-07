#include <iostream>
#include <string>

#include <zmqpp/zmqpp.hpp>

#include "dist_worker_api.hpp"

using namespace mdp;

int main(int argc, char *argv[]) {
    bool verbose = (argc == 2 && strcmp(argv[1], "-v") == 0);

    zmqpp::context ctx;
    DistWorkerAPI api = DistWorkerAPI(&ctx, "W1", verbose);

    std::string app_addr = "inproc://app";
    zmqpp::socket app_sock = zmqpp::socket(ctx, zmqpp::socket_type::pair);
    app_sock.bind(app_addr);

    api.StartThread(app_addr);
    
    zmqpp::message msg;
    while (1) {
        // Wait for message
        app_sock.receive(msg);

        //std::cout << "APP: Received message: " << msg << std::endl;

        // Echo that message
        app_sock.send(msg);
    }

    return 0;
}

