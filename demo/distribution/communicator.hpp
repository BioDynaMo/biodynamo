#ifndef __COMMUNICATOR__
#define __COMMUNICATOR__

#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.hpp"

namespace mdp {

class Communicator {
  public:
    Communicator () { };
    Communicator (std::uint8_t comm_id) : comm_id_(comm_id) { }

    virtual ~Communicator() {
        if (socket_) {
            delete socket_;
        }
    }

    virtual void RequestTimedOut() { }
    virtual void RequestCompleted() { }

    virtual void HandleOutgoingMessage(zmqpp::message& msg) = 0;
    virtual void HandleIncomingMessage() = 0;


    virtual std::uint8_t GetCommunicationId() {
        assert(comm_id_);
        return comm_id_;
    }

    virtual zmqpp::socket* GetSocketPtr() {
        return socket_;
    }

    virtual bool isConnected() {
        return is_connected_;
    }

    template<typename T>
    void SetSocketOption(const zmqpp::socket_option& option, const T& value) {
        socket_->set(option, value);
    }

    template<typename T>
    T GetSocketOption(const zmqpp::socket_option& option) {
        T value;
        GetSocketOption(option, &value);
        return value;
    }

    template<typename T>
    void GetSocketOption(const zmqpp::socket_option& option, T *value) {
        socket_->get(option, *value);
    }

  protected:
    DistSharedInfo* info_;

    zmqpp::socket *socket_ = nullptr;
    std::string endpoint_;

    std::uint8_t comm_id_;              // Type of communicator
    bool is_connected_;
};

}

#endif //__COMMUNICATOR__
