#ifndef DEMO_DISTRIBUTION_PROTOCOL_H_
#define DEMO_DISTRIBUTION_PROTOCOL_H_

#include <Rtypes.h>
#include <TBufferFile.h>
#include <TStorage.h>

#include <string>

#include "common.h"

namespace bdm {

const std::string PROTOCOL_CLIENT = "BDM/0.1C";
const std::string PROTOCOL_WORKER = "BDM/0.1W";

class MessageUtil {
 public:
  template <typename T>
  inline static std::unique_ptr<T> PopFrontObject(zmqpp::message* msg) {
    auto obj = MessageUtil::Deserialize<T>(msg->raw_data(0), msg->size(0));
    msg->pop_front();

    return obj;
  }

  template <typename T>
  inline static void PushFrontObject(zmqpp::message* msg, const T& obj) {
    size_t obj_sz;
    std::unique_ptr<const char[]> obj_bin =
        MessageUtil::Serialize(obj, &obj_sz);
    msg->push_front(obj_bin.get(), obj_sz);
  }

  template <typename T>
  inline static std::unique_ptr<T> Deserialize(const void* data, size_t size) {
    TBufferFile buf(TBufferFile::EMode::kRead, size, const_cast<void*>(data),
                    kFALSE);
    return std::unique_ptr<T>(
        reinterpret_cast<T*>(buf.ReadObjectAny(T::Class())));
  }

  template <typename T>
  inline static std::unique_ptr<const char[]> Serialize(const T& obj,
                                                        size_t* sz_out) {
    auto data_sz = TBuffer::kInitialSize;  //+ TBuffer::kExtraSpace;
    std::unique_ptr<char[]> data(new char[data_sz]);
    memset(data.get(), 0, data_sz);  // initialize bytes

    TBufferFile buf(TBufferFile::EMode::kWrite, data_sz, data.get(), kFALSE,
                    TStorage::ReAllocChar);
    assert(buf.WriteObjectAny(&obj, T::Class()) == 1);
    assert(buf.CheckObject(&obj, T::Class()));

    *sz_out = buf.Length();
    return std::move(data);
  }
};

// ###### Application level protocol definition ########## //
// ------------------------------------------------------- //

enum class AppProtocolCmd : std::uint8_t {
  kInvalid = 0,
  kDebug,
  kRequestHaloRegion,
  kReportHaloRegion,

  kCount,
  kMinValue = kDebug,
  kMaxValue = kRequestHaloRegion,
};
const std::string AppProtocolCmdStr[] = {
    "[Invalid]", "[Debug]", "[RequestHaloRegion]", "[ReportHaloRegion]"};

inline std::ostream& operator<<(std::ostream& stream,
                                const AppProtocolCmd& cmd) {
  stream << AppProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

class AppMessageHeader {
 public:
  AppMessageHeader() {}
  explicit AppMessageHeader(AppProtocolCmd cmd) : cmd_(cmd) {}
  virtual ~AppMessageHeader() {}

  AppProtocolCmd cmd_;

  inline friend std::ostream& operator<<(std::ostream& stream,
                                         const AppMessageHeader& header) {
    stream << "[Command]: " << header.cmd_ << std::endl;
    return stream;
  }

  ClassDef(AppMessageHeader, 1);
};

// ###### Middleware protocol definition ########## //
// ------------------------------------------------------- //

enum class ClientProtocolCmd : std::uint8_t {
  kInvalid = 0,
  kRequest,
  kReport,
  kAck,
  kNak,
  kCheckWorker,
  kBrokerTerminate,

  kMinValue = kRequest,
  kMaxValue = kBrokerTerminate,
  kCount = kMaxValue
};
const std::string ClientProtocolCmdStr[] = {
    "[Invalid]", "[Request]",     "[Report]",     "[ACK]",
    "[NAK]",     "[CheckWorker]", "ReqTerminate]"};

inline std::ostream& operator<<(std::ostream& stream,
                                const ClientProtocolCmd& cmd) {
  stream << ClientProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

// -------------------------------------------------------- //

enum class WorkerProtocolCmd : std::uint8_t {
  kInvalid = 0,
  kReady,
  kRequest,
  kReport,
  kHeartbeat,
  kDisconnect,

  kMinValue = kReady,
  kMaxValue = kDisconnect,
  kCount = kMaxValue
};
const std::string WorkerProtocolCmdStr[] = {"[Invalid]",   "[Ready]",
                                            "[Request]",   "[Report]",
                                            "[Heartbeat]", "[Disconnect]"};

inline std::ostream& operator<<(std::ostream& stream,
                                const WorkerProtocolCmd& cmd) {
  stream << WorkerProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

// Macro is used for named arguments/parameters
#define optional_arg(class_, type, name, default_value) \
 public:                                                \
  class_& name(type name) {                             \
    name##_ = name;                                     \
    return static_cast<class_&>(*this);                 \
  }                                                     \
  type name##_ = default_value

// Curiously Recurring Template Pattern/Idiom (CRTP)
template <typename T>
class MiddlewareMessageHeader {
 public:
  MiddlewareMessageHeader()
      : sender_(CommunicatorId::kUndefined),
        receiver_(CommunicatorId::kUndefined) {}
  MiddlewareMessageHeader(CommunicatorId sender, CommunicatorId receiver)
      : sender_(sender), receiver_(receiver) {}
  virtual ~MiddlewareMessageHeader() {}

  // Required fields
  CommunicatorId sender_;
  CommunicatorId receiver_;

  // Optional fields
  optional_arg(T, std::string, client_id, "");
  optional_arg(T, std::string, worker_id, "");

  inline friend std::ostream& operator<<(
      std::ostream& stream, const MiddlewareMessageHeader& header) {
    stream << "[Sender    ]: " << header.sender_ << std::endl
           << "[Receiver  ]: " << header.receiver_ << std::endl
           << "[Client id ]: " << header.client_id_ << std::endl
           << "[Worker id ]: " << header.worker_id_ << std::endl;

    return stream;
  }

  ClassDef(MiddlewareMessageHeader, 1);
};

class ClientMiddlewareMessageHeader
    : public MiddlewareMessageHeader<ClientMiddlewareMessageHeader> {
 public:
  ClientMiddlewareMessageHeader() : cmd_(ClientProtocolCmd::kInvalid) {}
  ClientMiddlewareMessageHeader(ClientProtocolCmd cmd, CommunicatorId sender,
                                CommunicatorId receiver)
      : MiddlewareMessageHeader(sender, receiver), cmd_(cmd) {}
  virtual ~ClientMiddlewareMessageHeader() {}

  ClientProtocolCmd cmd_;

  inline friend std::ostream& operator<<(
      std::ostream& stream, const ClientMiddlewareMessageHeader& header) {
    stream << std::endl << "{ClientMiddlewareMessageHeader}" << std::endl;

    stream << "[Command   ]: " << header.cmd_ << std::endl;
    stream << static_cast<MiddlewareMessageHeader>(header);

    return stream;
  }

  ClassDef(ClientMiddlewareMessageHeader, 1);
};

class WorkerMiddlewareMessageHeader
    : public MiddlewareMessageHeader<WorkerMiddlewareMessageHeader> {
 public:
  WorkerMiddlewareMessageHeader() : cmd_(WorkerProtocolCmd::kInvalid) {}
  WorkerMiddlewareMessageHeader(WorkerProtocolCmd cmd, CommunicatorId sender,
                                CommunicatorId receiver)
      : MiddlewareMessageHeader(sender, receiver), cmd_(cmd) {}
  virtual ~WorkerMiddlewareMessageHeader() {}

  WorkerProtocolCmd cmd_;

  inline friend std::ostream& operator<<(
      std::ostream& stream, const WorkerMiddlewareMessageHeader& header) {
    stream << std::endl << "{WorkerMiddlewareMessageHeader}" << std::endl;

    stream << "[Command   ]: " << header.cmd_ << std::endl;
    stream << static_cast<MiddlewareMessageHeader>(header);

    return stream;
  }

  ClassDef(WorkerMiddlewareMessageHeader, 1);
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_PROTOCOL_H_
