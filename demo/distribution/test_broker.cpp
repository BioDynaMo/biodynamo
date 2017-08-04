#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "broker.hpp"

using namespace mdp;

int main(int argc, char *argv[])
{
    bool verbose = (argc == 2 && strcmp(argv[1], "-v") == 0);
    zmqpp::context ctx;
    Broker broker (&ctx, "tcp://*:5555", verbose);
    broker.Run();

    return 0;
}
