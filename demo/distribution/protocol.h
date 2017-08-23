#ifndef DEMO_DISTRIBUTION_PROTOCOL_H_
#define DEMO_DISTRIBUTION_PROTOCOL_H_

#include <Rtypes.h>
#include <TBufferFile.h>

#include <string>

#include "common.h"

namespace bdm {

const std::string PROTOCOL_CLIENT = "BDM/0.1C";
const std::string PROTOCOL_WORKER = "BDM/0.1W";

// Distributed API commands
enum class AppProtocolCommand : std::uint8_t {
  kDebug = 0,

  kCount,
  kMinValue = kDebug,
  kMaxValue = kDebug,
};

class AppMessageHeader {
  AppProtocolCommand cmd;
  CommunicatorId sender;

  // ClassDef(AppMessageHeader, 1);
};

// -------------------------------------------------------- //
enum class ClientProtocolCmd : std::uint8_t {
  kInvalid = 0,
  kRequest,
  kReport,
  kNak,

  kMinValue = kRequest,
  kMaxValue = kNak,
  kCount = kMaxValue
};
const std::string ClientProtocolCmdStr[] = {"Invalid", "Request", "Report",
                                            "Nak"};

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
const std::string WorkerProtocolCmdStr[] = {
    "Invalid", "Ready", "Request", "Report", "Heartbeat", "Disconnect"};

inline std::ostream& operator<<(std::ostream& stream,
                                const WorkerProtocolCmd& cmd) {
  stream << WorkerProtocolCmdStr[ToUnderlying(cmd)];
  return stream;
}

class CommandHeader {
 public:
  CommandHeader() {}
  CommandHeader(CommunicatorId sender, CommunicatorId receiver)
      : sender_(sender), receiver_(receiver) {}
  virtual ~CommandHeader() {}

  CommunicatorId sender_ = CommunicatorId::kUndefined;
  CommunicatorId receiver_ = CommunicatorId::kUndefined;

  // Deserialize class
  template <typename T>
  static std::unique_ptr<T> FromString(const std::string* str) {
    TBufferFile buf(
        TBufferFile::EMode::kRead, str->size(),
        (const_cast<void*>(reinterpret_cast<const void*>(str->c_str()))));

    return std::unique_ptr<T>(
        reinterpret_cast<T*>(buf.ReadObjectAny(T::Class())));
  }

  inline friend std::ostream& operator<<(std::ostream& stream,
                                         const CommandHeader& header) {
    stream << "[Sender    ]: " << header.sender_ << std::endl
           << "[Receiver  ]: " << header.receiver_ << std::endl;
    return stream;
  }

  ClassDef(CommandHeader, 1);
};

#define optional_arg(class_, type, name, default_value) \
 public:                                                \
  class_& name(type name) {                             \
    name##_ = name;                                     \
    return *this;                                       \
  }                                                     \
  type name##_ = default_value

class ClientCommandHeader : public CommandHeader {
 public:
  ClientCommandHeader() {}
  ClientCommandHeader(ClientProtocolCmd cmd, CommunicatorId sender,
                      CommunicatorId receiver)
      : CommandHeader(sender, receiver), cmd_(cmd) {}
  virtual ~ClientCommandHeader() {}

  ClientProtocolCmd cmd_ = ClientProtocolCmd::kInvalid;

  optional_arg(ClientCommandHeader, std::string, client_id, "");
  optional_arg(ClientCommandHeader, std::string, worker_id, "");
  optional_arg(ClientCommandHeader, std::uint16_t, app_frames, 0);

  // Serialize class
  std::unique_ptr<std::string> ToString() {
    TBufferFile buf(TBufferFile::EMode::kWrite);
    assert(buf.WriteObjectAny(this, ClientCommandHeader::Class()) == 1);
    assert(buf.CheckObject(this, ClientCommandHeader::Class()));

    return std::make_unique<std::string>(
        std::string(buf.Buffer(), buf.Length()));
  }

  inline friend std::ostream& operator<<(std::ostream& stream,
                                         const ClientCommandHeader& header) {
    stream << std::endl << "{ClientCommandHeader}" << std::endl;

    stream << "[Command   ]: " << header.cmd_ << std::endl;
    stream << static_cast<CommandHeader>(header);
    stream << "[Client id ]: " << header.client_id_ << std::endl
           << "[Worker id ]: " << header.worker_id_ << std::endl
           << "[App frames]: " << header.app_frames_ << std::endl;

    return stream;
  }

  ClassDef(ClientCommandHeader, 1);
};

class WorkerCommandHeader : public CommandHeader {
 public:
  WorkerCommandHeader() {}
  WorkerCommandHeader(WorkerProtocolCmd cmd, CommunicatorId sender,
                      CommunicatorId receiver)
      : CommandHeader(sender, receiver), cmd_(cmd) {}
  virtual ~WorkerCommandHeader() {}

  WorkerProtocolCmd cmd_ = WorkerProtocolCmd::kInvalid;

  optional_arg(WorkerCommandHeader, std::string, client_id, "");
  optional_arg(WorkerCommandHeader, std::string, worker_id, "");
  optional_arg(WorkerCommandHeader, std::uint16_t, app_frames, 0);

  // Serialize class
  std::unique_ptr<std::string> ToString() {
    TBufferFile buf(TBufferFile::EMode::kWrite);
    assert(buf.WriteObjectAny(this, WorkerCommandHeader::Class()) == 1);
    assert(buf.CheckObject(this, WorkerCommandHeader::Class()));

    std::cout << *this << std::endl;

    return std::make_unique<std::string>(
        std::string(buf.Buffer(), buf.Length()));
  }

  inline friend std::ostream& operator<<(std::ostream& stream,
                                         const WorkerCommandHeader& header) {
    stream << std::endl << "{WorkerCommandHeader}" << std::endl;

    stream << "[Command   ]: " << header.cmd_ << std::endl;
    stream << static_cast<CommandHeader>(header);
    stream << "[Client id ]: " << header.client_id_ << std::endl
           << "[Worker id ]: " << header.worker_id_ << std::endl
           << "[App frames]: " << header.app_frames_ << std::endl;

    return stream;
  }

  ClassDef(WorkerCommandHeader, 1);
};

}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_PROTOCOL_H_
