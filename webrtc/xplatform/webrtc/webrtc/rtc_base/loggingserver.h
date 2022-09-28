#ifndef WEBRTC_BASE_LOGGINGSERVER_H_
#define WEBRTC_BASE_LOGGINGSERVER_H_

#include <list>
#include <utility>
#include "webrtc/rtc_base/sigslot.h"
#include "webrtc/rtc_base/logging.h"
#include "webrtc/rtc_base/asyncsocket.h"
#include "webrtc/rtc_base/socketstream.h"

namespace rtc {
class AsyncSocket;
class SocketAddress;
class SocketStream;
class Thread;

class PlatformThread;

class LogSinkImpl
  : public LogSink {
public:
  LogSinkImpl() {}

  explicit LogSinkImpl(AsyncSocket* socket) {
    socketStream_.reset(new SocketStream(socket));
  }

  void Detach() {
    socketStream_->Detach();
  }

private:
  void OnLogMessage(const std::string& message) override {
    socketStream_->WriteAll(
      message.data(), message.size(), nullptr, nullptr);
  }

  std::unique_ptr<SocketStream> socketStream_;
};

// Inherit from has_slots class to use signal and slots.
class LoggingServer : public sigslot::has_slots<sigslot::multi_threaded_local> {
 public:
  LoggingServer();
  virtual ~LoggingServer();

  int Listen(const SocketAddress& addr, LoggingSeverity level);

 protected:
  void OnAcceptEvent(AsyncSocket* socket);
  void OnCloseEvent(AsyncSocket* socket, int err);

 private:
  static bool processMessages(void* args);

 private:
  LoggingSeverity level_;
  std::unique_ptr<AsyncSocket> listener_;
  std::list<std::pair<AsyncSocket*, LogSinkImpl*> > connections_;
  Thread* thread_;
  std::unique_ptr<rtc::PlatformThread> tw_;
};

}  //  namespace rtc

#endif  //  WEBRTC_BASE_LOGGINGSERVER_H_
