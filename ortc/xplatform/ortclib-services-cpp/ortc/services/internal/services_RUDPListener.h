/*

 Copyright (c) 2014, Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#pragma once

#include <ortc/services/internal/types.h>
#include <ortc/services/IRUDPListener.h>
#include <ortc/services/internal/services_RUDPChannel.h>

#include <zsLib/Socket.h>

#include <list>
#include <map>
#include <utility>

#define ORTC_SERVICES_RUDPLISTENER_CHANNEL_RANGE_START (0x4000)
#define ORTC_SERVICES_RUDPLISTENER_CHANNEL_RANGE_END   (0x7FFF)

namespace ortc
{
  namespace services
  {
    namespace internal
    {
      interaction IRUDPChannelForRUDPListener;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener
      #pragma mark

      class RUDPListener : public Noop,
                           public MessageQueueAssociator,
                           public IRUDPListener,
                           public ISocketDelegate,
                           public IRUDPChannelDelegateForSessionAndListener
      {
      protected:
        struct make_private {};

      public:
        friend interaction IRUDPListenerFactory;

        ZS_DECLARE_TYPEDEF_PTR(IRUDPChannelForRUDPListener, UseRUDPChannel)

        class CompareChannelPair;

        typedef IPAddress RemoteIP;
        typedef WORD ChannelNumber;
        typedef std::pair<RemoteIP, WORD> ChannelPair;
        typedef std::map<ChannelPair, UseRUDPChannelPtr, CompareChannelPair> SessionMap;

        typedef std::list<UseRUDPChannelPtr> PendingSessionList;

      public:
        RUDPListener(
                     const make_private &,
                     IMessageQueuePtr queue,
                     IRUDPListenerDelegatePtr delegate,
                     WORD port,
                     const char *realm
                     );

      protected:
        RUDPListener(Noop) : Noop(true), MessageQueueAssociator(IMessageQueuePtr()) {};

        void init();

      public:
        ~RUDPListener();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RUDPListener => IRUDPListener
        #pragma mark

        virtual PUID getID() const {return mID;}

        static RUDPListenerPtr create(
                                      IMessageQueuePtr queue,
                                      IRUDPListenerDelegatePtr delegate,
                                      WORD port,
                                      const char *realm
                                      );

        virtual IPAddress getListenerIP();

        virtual RUDPListenerStates getState() const;

        virtual void shutdown();

        virtual IRUDPChannelPtr acceptChannel(
                                              IRUDPChannelDelegatePtr delegate,
                                              ITransportStreamPtr receiveStream,
                                              ITransportStreamPtr sendStream
                                              );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RUDPListener => ISocketDelegate
        #pragma mark

        virtual void onReadReady(SocketPtr socket);
        virtual void onWriteReady(SocketPtr socket);
        virtual void onException(SocketPtr socket);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RUDPListener => IRUDPChannelDelegateForSessionAndListener
        #pragma mark

        virtual void onRUDPChannelStateChanged(
                                               RUDPChannelPtr channel,
                                               RUDPChannelStates state
                                               );

        virtual bool notifyRUDPChannelSendPacket(
                                                 RUDPChannelPtr channel,
                                                 const IPAddress &remoteIP,
                                                 const BYTE *packet,
                                                 size_t packetLengthInBytes
                                                 );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RUDPListener => (internal)
        #pragma mark

        Log::Params log(const char *message) const;
        void fix(STUNPacketPtr stun) const;

        bool isShuttingDown() {return RUDPListenerState_ShuttingDown == mCurrentState;}
        bool isShutdown() {return RUDPListenerState_Shutdown == mCurrentState;}

        void cancel();
        void setState(RUDPListenerStates state);

        bool bindUDP();

        bool sendTo(
                    const IPAddress &destination,
                    STUNPacketPtr stun
                    );

        bool sendTo(
                    const IPAddress &destination,
                    const BYTE *buffer,
                    size_t bufferLengthInBytes
                    );

        bool handledNonce(
                          const IPAddress &remoteIP,
                          STUNPacketPtr &stun,
                          STUNPacketPtr &response
                          );

        bool handleUnknownChannel(
                                  const IPAddress &remoteIP,
                                  STUNPacketPtr &stun,
                                  STUNPacketPtr &outResponse
                                  );

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RUDPListener::CompareChannelPair
        #pragma mark

        class CompareChannelPair { // simple comparison function
        public:
          bool operator()(const ChannelPair &op1, const ChannelPair &op2) const;
        };

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RUDPListener => (internal)
        #pragma mark

        mutable RecursiveLock mLock;
        RUDPListenerWeakPtr mThisWeak;
        IRUDPListenerDelegatePtr mDelegate;
        AutoPUID mID;

        RUDPListenerPtr mGracefulShutdownReference;

        RUDPListenerStates mCurrentState {RUDPListenerState_Listening};

        WORD mBindPort;

        SocketPtr mUDPSocket;

        SessionMap mLocalChannelNumberSessions;   // local channel numbers are the channel numbers we expect to receive from the remote party
        SessionMap mRemoteChannelNumberSessions;  // remote channel numbers are the channel numbers we expect to send to the remote party

        PendingSessionList mPendingSessions;

        BYTE mMagic[16];
        String mRealm;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IRUDPListenerFactory
      #pragma mark

      interaction IRUDPListenerFactory
      {
        static IRUDPListenerFactory &singleton();

        virtual RUDPListenerPtr create(
                                       IMessageQueuePtr queue,
                                       IRUDPListenerDelegatePtr delegate,
                                       WORD port,
                                       const char *realm
                                       );
      };

      class RUDPListenerFactory : public IFactory<IRUDPListenerFactory> {};
    }
  }
}
