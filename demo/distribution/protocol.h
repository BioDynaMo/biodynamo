#ifndef DEMO_DISTRIBUTION_PROTOCOL_H_
#define DEMO_DISTRIBUTION_PROTOCOL_H_

#include <Rtypes.h>
#include <TBufferFile.h>
#include <TStorage.h>

#include <string>

#include "common.h"

namespace bdm {

const std::string PROTOCOL_CLIENT = "BDM/0.1C";  //!< Client protocol version
const std::string PROTOCOL_WORKER = "BDM/0.1W";  //!< Worker protocol version

///
/// @brief Utility class for network messages
///
class MessageUtil {
 public:
  ///
  /// @brief Pops and deserializes the first frame from a ZMQ message
  ///
  /// @param[in]  msg   Pointer to the ZMQ message
  /// @tparam           The serialized object type
  ///
  /// @returns Unique pointer to the deserialized object
  ///
  template <typename T>
  inline static std::unique_ptr<T> PopFrontObject(zmqpp::message* msg) {
    auto obj = MessageUtil::Deserialize<T>(msg->raw_data(0), msg->size(0));
    msg->pop_front();

    return obj;
  }

  ///
  /// @brief Serializes and pushes an object at the beggining of a ZMQ message
  ///
  /// @param[in]  msg   Pointer to the ZMQ message
  /// @param[in]  obj   The object to be serialized and added to the message
  /// @tparam           Type of the object
  ///
  template <typename T>
  inline static void PushFrontObject(zmqpp::message* msg, const T& obj) {
    size_t obj_sz;
    std::unique_ptr<const char[]> obj_bin =
        MessageUtil::Serialize(obj, &obj_sz);
    msg->push_front(obj_bin.get(), obj_sz);
  }

  ///
  /// @brief  Deserializes an object from a binary string
  ///
  /// The binary string must have been previously serialized by ROOT I/O.
  ///
  /// @param[in]  data    Pointer to the binary string
  /// @param[in]  size    Size of the binary string
  /// @tparam             Type of the serialized object
  ///
  /// @returns  Unique pointer to the deserialized object
  ///
  template <typename T>
  inline static std::unique_ptr<T> Deserialize(const void* data, size_t size) {
    TBufferFile buf(TBufferFile::EMode::kRead, size, const_cast<void*>(data),
                    kFALSE);
    return std::unique_ptr<T>(
        reinterpret_cast<T*>(buf.ReadObjectAny(T::Class())));
  }

  ///
  /// @brief  Serializes an object to a binary string
  ///
  /// Serializes an object to a binary string ready for network transmission,
  /// using the ROOT I/O framework.
  ///
  /// @param[in]  obj     Object to be serialized
  /// @param[out] sz_out  Size of the resulting binary string
  /// @tparam             Type of the object to be serialized
  ///
  /// @returns  Unique pointer to the serialized binary string
  ///
  template <typename T>
  inline static std::unique_ptr<const char[]> Serialize(const T& obj,
                                                        size_t* sz_out) {
    auto data_sz = TBuffer::kInitialSize;
    std::unique_ptr<char[]> data(new char[data_sz]);
    memset(data.get(), 0, data_sz);  // Initialize bytes to zero

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

///
/// @brief Application protocol commands enumerator
///
enum class AppProtocolCmd : std::uint8_t {
  kInvalid = 0,
  kDebug,
  kRequestHaloRegion,
  kReportHaloRegion,

  kCount,
  kMinValue = kDebug,
  kMaxValue = kRequestHaloRegion,
};
/// @brief  String repr of the AppProtocolCmdStr enumerator
const std::string AppProtocolCmdStr[] = {
    "[Invalid]", "[Debug]", "[RequestHaloRegion]", "[ReportHaloRegion]"};

/// @brief  Overloaded stream operator for AppProtocolCmd
inline std::ostream& operator<<(std::ostream& stream,
                                const AppProtocolCmd& cmd) {
  stream << AppProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

///
/// @brief Application message header class
///
/// Holds information about the application level protocol
///
class AppMessageHeader {
 public:
  ///
  /// @brief  Create empty header
  ///
  AppMessageHeader() {}

  ///
  /// @brief Create header with \p cmd command
  ///
  explicit AppMessageHeader(AppProtocolCmd cmd) : cmd_(cmd) {}

  ///
  /// @brief Virtual destructor (needed by ROOT I/O)
  ///
  virtual ~AppMessageHeader() {}

  AppProtocolCmd cmd_;  //!< Application command

  /// @brief Overloaded stream operator for #AppMessageHeader
  inline friend std::ostream& operator<<(std::ostream& stream,
                                         const AppMessageHeader& header) {
    stream << "[Command]: " << header.cmd_ << std::endl;
    return stream;
  }

  /// Directive to ROOT I/O to serialize this class
  ClassDef(AppMessageHeader, 1);
};

// ###### Middleware protocol definition ########## //
// ------------------------------------------------------- //

///
/// @brief Client middleware protocol commands enumerator
///
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
/// @brief  String repr of the #ClientProtocolCmd enumerator
const std::string ClientProtocolCmdStr[] = {
    "[Invalid]", "[Request]",     "[Report]",     "[ACK]",
    "[NAK]",     "[CheckWorker]", "ReqTerminate]"};

/// @brief  Overloaded stream operator for #ClientProtocolCmd
inline std::ostream& operator<<(std::ostream& stream,
                                const ClientProtocolCmd& cmd) {
  stream << ClientProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

// -------------------------------------------------------- //
///
/// @brief Worker middleware protocol commands enumerator
///
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
/// @brief  String repr of the #WorkerProtocolCmd enumerator
const std::string WorkerProtocolCmdStr[] = {"[Invalid]",   "[Ready]",
                                            "[Request]",   "[Report]",
                                            "[Heartbeat]", "[Disconnect]"};

/// @brief  Overloaded stream operator for #WorkerProtocolCmd
inline std::ostream& operator<<(std::ostream& stream,
                                const WorkerProtocolCmd& cmd) {
  stream << WorkerProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

// -------------------------------------------------------- //

// Macro is used for named arguments/parameters
#define optional_arg(class_, type, name, default_value) \
 public:                                                \
  class_& name(type name) {                             \
    name##_ = name;                                     \
    return static_cast<class_&>(*this);                 \
  }                                                     \
  type name##_ = default_value

///
/// @brief  Middleware message header class
///
/// Holds information about the middleware level protocol. Makes use of the
/// Curiously Recurring Template Pattern/Idiom (CRTP) to reduce code rewritting
///
/// @tparam Type of the derived message type
///
template <typename T>
class MiddlewareMessageHeader {
 public:
  /// @brief  Create empty header
  MiddlewareMessageHeader()
      : sender_(CommunicatorId::kUndefined),
        receiver_(CommunicatorId::kUndefined) {}

  ///
  /// @brief Create new and valid header
  ///
  /// @param[in]  sender    Message origin
  /// @param[in]  receiver  Message final destination
  ///
  MiddlewareMessageHeader(CommunicatorId sender, CommunicatorId receiver)
      : sender_(sender), receiver_(receiver) {}

  /// @brief Virtual destructor (needed by ROOT I/O)
  virtual ~MiddlewareMessageHeader() {}

  // Required fields
  CommunicatorId sender_;    //!< Message origin
  CommunicatorId receiver_;  //!< Message final destination

  // Optional fields
  optional_arg(T, std::string, client_id, "");  //!< Identifier of the client
  optional_arg(T, std::string, worker_id, "");  //!< Identifier of the worker

  /// @brief  Overloaded stream operator for #MiddlewareMessageHeader
  inline friend std::ostream& operator<<(
      std::ostream& stream, const MiddlewareMessageHeader& header) {
    stream << "[Sender    ]: " << header.sender_ << std::endl
           << "[Receiver  ]: " << header.receiver_ << std::endl
           << "[Client id ]: " << header.client_id_ << std::endl
           << "[Worker id ]: " << header.worker_id_ << std::endl;

    return stream;
  }

  /// Directive to ROOT I/O to serialize this class
  ClassDef(MiddlewareMessageHeader, 1);
};

///
/// @brief Middleware message header used for client -- broker communication
///
class ClientMiddlewareMessageHeader
    : public MiddlewareMessageHeader<ClientMiddlewareMessageHeader> {
 public:
  /// @brief  Create empty & invalid header
  ClientMiddlewareMessageHeader() : cmd_(ClientProtocolCmd::kInvalid) {}

  ///
  /// @brief Create new valid header
  ///
  /// @param[in]  cmd       Command of this message
  /// @param[in]  sender    Message origin
  /// @param[in]  receiver  Message final destination
  ///
  ClientMiddlewareMessageHeader(ClientProtocolCmd cmd, CommunicatorId sender,
                                CommunicatorId receiver)
      : MiddlewareMessageHeader(sender, receiver), cmd_(cmd) {}

  /// @brief Virtual destructor (needed by ROOT I/O)
  virtual ~ClientMiddlewareMessageHeader() {}

  ClientProtocolCmd cmd_;  //!< Message command for the middleware

  /// @brief  Overloaded stream operator for #ClientMiddlewareMessageHeader
  inline friend std::ostream& operator<<(
      std::ostream& stream, const ClientMiddlewareMessageHeader& header) {
    stream << std::endl << "{ClientMiddlewareMessageHeader}" << std::endl;

    stream << "[Command   ]: " << header.cmd_ << std::endl;
    stream << static_cast<MiddlewareMessageHeader>(header);

    return stream;
  }

  /// Directive to ROOT I/O to serialize this class
  ClassDef(ClientMiddlewareMessageHeader, 1);
};

///
/// @brief Middleware message header used for client -- broker communication
///
class WorkerMiddlewareMessageHeader
    : public MiddlewareMessageHeader<WorkerMiddlewareMessageHeader> {
 public:
  /// @brief  Create empty & invalid header
  WorkerMiddlewareMessageHeader() : cmd_(WorkerProtocolCmd::kInvalid) {}

  ///
  /// @brief Create new valid header
  ///
  /// @param[in]  cmd       Command of this message
  /// @param[in]  sender    Message origin
  /// @param[in]  receiver  Message final destination
  ///
  WorkerMiddlewareMessageHeader(WorkerProtocolCmd cmd, CommunicatorId sender,
                                CommunicatorId receiver)
      : MiddlewareMessageHeader(sender, receiver), cmd_(cmd) {}

  /// @brief Virtual destructor (needed by ROOT I/O)
  virtual ~WorkerMiddlewareMessageHeader() {}

  WorkerProtocolCmd cmd_;  //!< Message command for the middleware

  /// @brief  Overloaded stream operator for #WorkerMiddlewareMessageHeader
  inline friend std::ostream& operator<<(
      std::ostream& stream, const WorkerMiddlewareMessageHeader& header) {
    stream << std::endl << "{WorkerMiddlewareMessageHeader}" << std::endl;

    stream << "[Command   ]: " << header.cmd_ << std::endl;
    stream << static_cast<MiddlewareMessageHeader>(header);

    return stream;
  }

  /// Directive to ROOT I/O to serialize this class
  ClassDef(WorkerMiddlewareMessageHeader, 1);
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_PROTOCOL_H_
