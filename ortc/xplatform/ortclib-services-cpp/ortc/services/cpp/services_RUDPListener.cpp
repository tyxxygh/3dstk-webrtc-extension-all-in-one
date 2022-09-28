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

#include <ortc/services/internal/services_RUDPListener.h>
#include <ortc/services/internal/services_RUDPChannel.h>
#include <ortc/services/STUNPacket.h>
#include <ortc/services/RUDPPacket.h>
#include <ortc/services/ISTUNRequesterManager.h>
#include <ortc/services/IICESocket.h>
#include <ortc/services/IHelper.h>

#include <zsLib/Exception.h>
#include <zsLib/helpers.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>
#include <zsLib/Stringize.h>

#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/queue.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/secblock.h>

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

#include <algorithm>

#define ORTC_SERVICES_RUDPLISTENER_BUFFER_SIZE (1 << (sizeof(WORD)*8))

#define ORTC_SERVICES_RUDPLISTENER_MAX_NONCE_LIFETIME_IN_SECONDS (5*60)

#define ORTC_SERVICES_RUDPLISTENER_MAX_ATTEMPTS_TO_FIND_FREE_CHANNEL_NUMBER (5)

namespace ortc { namespace services { ZS_DECLARE_SUBSYSTEM(org_ortc_services_rudp) } }

namespace ortc
{
  namespace services
  {
    namespace internal
    {
      using CryptoPP::StringSink;
      using CryptoPP::Weak::MD5;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helper)
      #pragma mark

      //-----------------------------------------------------------------------
      String convertToHex(
                          const BYTE *buffer,
                          size_t bufferLengthInBytes
                          )
      {
        std::string output;
        HexEncoder encoder(new StringSink(output));
        encoder.Put(buffer, bufferLengthInBytes);
        encoder.MessageEnd();
        return output;
      }

      //-----------------------------------------------------------------------
      void convertFromHex(
                          const String &input,
                          SecureByteBlock &output
                          )
      {
        ByteQueue queue;
        queue.Put((BYTE *)input.c_str(), input.size());

        ByteQueue *outputQueue = new ByteQueue;
        HexDecoder decoder(outputQueue);
        queue.CopyTo(decoder);
        decoder.MessageEnd();

        size_t outputLengthInBytes = static_cast<size_t>(outputQueue->CurrentSize());
        output.CleanNew(outputLengthInBytes);

        outputQueue->Get(output, outputLengthInBytes);
      }

      //-----------------------------------------------------------------------
      void computeNonceHash(
                            BYTE *hashOut,
                            time_t current,
                            const IPAddress &remoteIP,
                            STUNPacketPtr &stun,
                            const BYTE *magic,
                            size_t magicLengthInBytes
                            )
      {
        MD5 md5;

        BYTE key[16];
        memset(&(key[0]), 0, sizeof(key));

        md5.Update((const BYTE *)&current, sizeof(current));

        String ipAsStr = remoteIP.string();
        md5.Update((const BYTE *)((CSTR)ipAsStr), ipAsStr.length());
        md5.Update((const BYTE *)((CSTR)":"), strlen(":"));
        md5.Update((const BYTE *)((CSTR)stun->mRealm), stun->mRealm.length());
        md5.Update((const BYTE *)((CSTR)":"), strlen(":"));
        md5.Update(magic, magicLengthInBytes);

        ZS_THROW_INVALID_ASSUMPTION_IF(sizeof(key) != md5.DigestSize())
        md5.Final(&(key[0]));

        memcpy(hashOut, &(key[0]), sizeof(key));
      }

      //-----------------------------------------------------------------------
      bool isNonceValid(
                        const IPAddress &remoteIP,
                        STUNPacketPtr &stun,
                        const BYTE *magic,
                        size_t magicLengthInBytes
                        )
      {
        String nonce = stun->mNonce;

        SecureByteBlock output;
        convertFromHex(stun->mNonce, output);
        if (output.size() != sizeof(time_t) + 16) return false;

        time_t temp = 0;
        memcpy(&temp, &(output[0]), sizeof(time_t));

        BYTE key[16];
        computeNonceHash(&(key[0]), temp, remoteIP, stun, magic, magicLengthInBytes);

        if (0 != memcmp(&(key[0]), &(output[sizeof(time_t)]), sizeof(key))) return false;

        time_t now = time(NULL);
        if (temp + ORTC_SERVICES_RUDPLISTENER_MAX_NONCE_LIFETIME_IN_SECONDS < now) return false;

        return true;
      }

      //-----------------------------------------------------------------------
      String createNonce(
                         const IPAddress &remoteIP,
                         STUNPacketPtr &stun,
                         const BYTE *magic,
                         size_t magicLengthInBytes
                         )
      {
        BYTE output[sizeof(time_t) + 16];
        memset(&(output[0]), 0, sizeof(output));

        time_t temp = time(NULL);
        memcpy(&(output[0]), &temp, sizeof(time_t));

        BYTE key[16];
        computeNonceHash(&(key[0]), temp, remoteIP, stun, magic, magicLengthInBytes);

        memcpy(&(output[sizeof(time_t)]), &(key[0]), sizeof(key));

        return convertToHex(&(output[0]), sizeof(output));
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener
      #pragma mark

      //-----------------------------------------------------------------------
      RUDPListener::RUDPListener(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 IRUDPListenerDelegatePtr delegate,
                                 WORD port,
                                 const char *realm
                                 ) :
        MessageQueueAssociator(queue),
        mDelegate(IRUDPListenerDelegateProxy::createWeak(delegate)),
        mBindPort(port),
        mRealm(realm)
      {
        CryptoPP::AutoSeededRandomPool rng;
        rng.GenerateBlock(&(mMagic[0]), sizeof(mMagic));

        ZS_LOG_BASIC(log("started"));// + ZS_PARAM("compiled date", __DATE__) + ZS_PARAM("time", __TIME__))
      }

      //-----------------------------------------------------------------------
      void RUDPListener::init()
      {
        AutoRecursiveLock lock(mLock);
        bindUDP();
      }

      //-----------------------------------------------------------------------
      RUDPListener::~RUDPListener()
      {
        if(isNoop()) return;
        
        mThisWeak.reset();
        ZS_LOG_BASIC(log("destroyed"))
        cancel();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener => IRUDPListener
      #pragma mark

      //-----------------------------------------------------------------------
      RUDPListenerPtr RUDPListener::create(
                                           IMessageQueuePtr queue,
                                           IRUDPListenerDelegatePtr delegate,
                                           WORD port,
                                           const char *realm
                                           )
      {
        RUDPListenerPtr pThis(make_shared<RUDPListener>(make_private{}, queue, delegate, port, realm));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      IPAddress RUDPListener::getListenerIP()
      {
        AutoRecursiveLock lock(mLock);
        try {
          IPAddress local = mUDPSocket->getLocalAddress();
          return local;
        } catch(Socket::Exceptions::Unspecified &) {
        }
        return IPAddress();
      }

      IRUDPListener::RUDPListenerStates RUDPListener::getState() const
      {
        AutoRecursiveLock lock(mLock);
        return mCurrentState;
      }

      //-----------------------------------------------------------------------
      void RUDPListener::shutdown()
      {
        AutoRecursiveLock lock(mLock);
        cancel();
      }

      //-----------------------------------------------------------------------
      IRUDPChannelPtr RUDPListener::acceptChannel(
                                                  IRUDPChannelDelegatePtr delegate,
                                                  ITransportStreamPtr receiveStream,
                                                  ITransportStreamPtr sendStream
                                                  )
      {
        ZS_THROW_INVALID_ARGUMENT_IF(!receiveStream)
        ZS_THROW_INVALID_ARGUMENT_IF(!sendStream)

        AutoRecursiveLock lock(mLock);
        if (mPendingSessions.size() < 1) return IRUDPChannelPtr();

        UseRUDPChannelPtr session = mPendingSessions.front();
        mPendingSessions.pop_front();
        session->setDelegate(delegate);
        session->setStreams(receiveStream, sendStream);
        return RUDPChannel::convert(session);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener => ISocketDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RUDPListener::onReadReady(SocketPtr socket)
      {
        IPAddress remote;
        STUNPacketPtr stun;

        std::unique_ptr<BYTE[]> buffer(new BYTE[ORTC_SERVICES_RUDPLISTENER_BUFFER_SIZE]);

        size_t bytesRead = 0;

        // scope: read from the socket
        {
          AutoRecursiveLock lock(mLock);
          if (!mDelegate) return;

          try {
            bytesRead = mUDPSocket->receiveFrom(remote, buffer.get(), ORTC_SERVICES_RUDPLISTENER_BUFFER_SIZE);
          } catch(Socket::Exceptions::Unspecified &) {
            cancel();
            return;
          }

          if (0 == bytesRead) return;
        }

        STUNPacketPtr response;

        stun = STUNPacket::parseIfSTUN(buffer.get(), bytesRead, STUNPacket::ParseOptions(static_cast<STUNPacket::RFCs>(STUNPacket::RFC_5389_STUN | STUNPacket::RFC_draft_RUDP), false, "RUDPListener", mID));
        while (stun)  // NOTE: using this as a scope that can be broken rather than a loop
        {
          String localUsernameFrag;
          String remoteUsernameFrag;

          if (stun->hasAttribute(STUNPacket::Attribute_Username)) {
            size_t pos = stun->mUsername.find(":");
            if (String::npos == pos) {
              localUsernameFrag = stun->mUsername;
              remoteUsernameFrag = stun->mUsername;
            } else {
              // split the string at the fragments
              localUsernameFrag = stun->mUsername.substr(0, pos); // this would be our local username
              remoteUsernameFrag = stun->mUsername.substr(pos+1);  // this would be the remote username
            }
          }

          // first thing to check is if this is a response to an outstanding request
          if (ISTUNRequesterManager::handleSTUNPacket(remote, stun)) return;

          // next we ignore all responses/error responses because they would have been handled by a requester
          if ((STUNPacket::Class_Response == stun->mClass) ||
              (STUNPacket::Class_ErrorResponse == stun->mClass)) return;

          // now we check for a binding request and respond accordingly
          if (STUNPacket::Method_Binding == stun->mMethod) {
            if (STUNPacket::Class_Indication == stun->mClass) return;  // we do not allow indication requests

            if (0 != stun->mErrorCode) {
              // the request might be binding but we have an error
              response = STUNPacket::createErrorResponse(stun);
              fix(response);
            } else {
              response = STUNPacket::createResponse(stun);
              fix(response);
              if ((remoteUsernameFrag.hasData()) &&
                  (localUsernameFrag.hasData())) {
                if (stun->isValidMessageIntegrity(localUsernameFrag)) {
                  response->mUsername = remoteUsernameFrag + ":" + localUsernameFrag;
                  response->mPassword = localUsernameFrag;
                  response->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;

                  STUNPacketPtr request = STUNPacket::createRequest(STUNPacket::Method_Binding);
                  request->mUsername = remoteUsernameFrag + ":" + localUsernameFrag;
                  request->mPassword = remoteUsernameFrag;
                  request->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
                  request->mPriorityIncluded = true;
                  request->mPriority = ((1 << 24)*(static_cast<DWORD>(IICESocket::Type_Local))) + ((1 << 8)*(static_cast<DWORD>(0))) + (256 - 0);
                  request->mIceControlledIncluded = true;
                  request->mIceControlled = 0;
                  sendTo(remote, request);
                }
              }
              response->mMappedAddress = remote;
            }
            break;
          }

          UseRUDPChannelPtr session;

          // scope: next we attempt to see if there is already a session that handles this IP/channel pairing
          if ((stun->hasAttribute(STUNPacket::Attribute_Username)) &&
              (stun->hasAttribute(STUNPacket::Attribute_ChannelNumber)))
          {
            AutoRecursiveLock lock(mLock);
            ChannelPair lookup(remote, stun->mChannelNumber);
            SessionMap::iterator found = mRemoteChannelNumberSessions.find(lookup);
            if (found != mRemoteChannelNumberSessions.end()) {
              session = (*found).second;
            }
          }

          // next we check if this is a new incoming request without a nonce or realm or username
          {
            AutoRecursiveLock lock(mLock);
            bool handled = handledNonce(remote, stun, response);
            if ((handled) && (!response)) return;
          }

          if (!response) {
            if (session) {
              bool handled = session->handleSTUN(stun, response, localUsernameFrag, remoteUsernameFrag);
              if ((handled) && (!response)) return;
              break;
            } else {
              bool handled = handleUnknownChannel(remote, stun, response);
              if ((handled) && (!response)) return;
              break;
            }
          }

          if (!response) {
            // not handled
            if (STUNPacket::Class_Request == stun->mClass) {
              stun->mErrorCode = STUNPacket::ErrorCode_BadRequest;
              response = STUNPacket::createErrorResponse(stun);
              fix(response);
            }
          }

          // make sure there is a response, if not then we abort since it was a STUN packet but we may or may not have responded
          if (!response) return;

          break;  // NOTE: do not intend to loop
        }

        if (response) {
          sendTo(remote, response);
          return;
        }

        // try and parse this as an RUDPPacket now
        RUDPPacketPtr rudp = RUDPPacket::parseIfRUDP(buffer.get(), bytesRead);
        if (rudp) {
          UseRUDPChannelPtr session;

          // scope: figure out which session this belongs
          {
            AutoRecursiveLock lock(mLock);
            ChannelPair lookup(remote, rudp->mChannelNumber);
            SessionMap::iterator found = mLocalChannelNumberSessions.find(lookup);
            if (found == mLocalChannelNumberSessions.end()) return;  // doesn't belong to any session so ignore it

            session = (*found).second;
            ZS_THROW_INVALID_ASSUMPTION_IF(!session)
          }

          // push the RUDP packet to the session to handle
          session->handleRUDP(rudp, buffer.get(), bytesRead);
        }
      }


      //-----------------------------------------------------------------------
      void RUDPListener::onWriteReady(SocketPtr socket)
      {
        AutoRecursiveLock lock(mLock);
        for (SessionMap::iterator iter = mLocalChannelNumberSessions.begin(); iter != mLocalChannelNumberSessions.end(); ++iter) {
          UseRUDPChannelPtr session = (*iter).second;
          session->notifyWriteReady();
        }
      }

      //-----------------------------------------------------------------------
      void RUDPListener::onException(SocketPtr socket)
      {
        AutoRecursiveLock lock(mLock);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener => IRUDPChannelDelegateForSessionAndListener
      #pragma mark

      //-----------------------------------------------------------------------
      void RUDPListener::onRUDPChannelStateChanged(
                                                   RUDPChannelPtr channel,
                                                   RUDPChannelStates state
                                                   )
      {
        if (IRUDPChannel::RUDPChannelState_Shutdown != state) return;

        AutoRecursiveLock lock(mLock);
        for (SessionMap::iterator iter = mLocalChannelNumberSessions.begin(); iter != mLocalChannelNumberSessions.end(); ++iter)
        {
          if ((*iter).second != channel) continue;
          mLocalChannelNumberSessions.erase(iter);
          break;
        }
        for (SessionMap::iterator iter = mRemoteChannelNumberSessions.begin(); iter != mRemoteChannelNumberSessions.end(); ++iter)
        {
          if ((*iter).second != channel) continue;
          mRemoteChannelNumberSessions.erase(iter);
          break;
        }
        for (PendingSessionList::iterator iter = mPendingSessions.begin(); iter != mPendingSessions.end(); ++iter)
        {
          if ((*iter) != channel) continue;
          mPendingSessions.erase(iter);
          break;
        }

        if (isShuttingDown()) {
          if (0 == mLocalChannelNumberSessions.size()) {
            // everything should be ready to shutdown now
            cancel();
          }
        }
      }

      //-----------------------------------------------------------------------
      bool RUDPListener::notifyRUDPChannelSendPacket(
                                                     RUDPChannelPtr channel,
                                                     const IPAddress &remoteIP,
                                                     const BYTE *packet,
                                                     size_t packetLengthInBytes
                                                     )
      {
        AutoRecursiveLock lock(mLock);
        return sendTo(remoteIP, packet, packetLengthInBytes);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RUDPListener::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("RUDPListener");
        IHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      void RUDPListener::fix(STUNPacketPtr stun) const
      {
        stun->mLogObject = "RUDPListener";
        stun->mLogObjectID = mID;
      }

      //-----------------------------------------------------------------------
      void RUDPListener::cancel()
      {
        AutoRecursiveLock lock(mLock);

        if (isShutdown()) return;

        if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

        setState(RUDPListenerState_ShuttingDown);

        if (mLocalChannelNumberSessions.size() > 0) {
          for (SessionMap::iterator iter = mLocalChannelNumberSessions.begin(); iter != mLocalChannelNumberSessions.end(); ++iter) {
            UseRUDPChannelPtr session = (*iter).second;
            session->shutdown();
          }
        }

        if (mGracefulShutdownReference) {

          if (mLocalChannelNumberSessions.size() > 0) {
            ZS_LOG_DEBUG(log("waiting for sessions to shutdown"))
            return;
          }
        }

        setState(RUDPListenerState_Shutdown);

        mDelegate.reset();
        mGracefulShutdownReference.reset();

        if (mUDPSocket) {
          mUDPSocket->close();
          mUDPSocket.reset();
        }

        mLocalChannelNumberSessions.clear();
        mRemoteChannelNumberSessions.clear();
        mPendingSessions.clear();
      }

      //-----------------------------------------------------------------------
      void RUDPListener::setState(RUDPListenerStates state)
      {
        if (mCurrentState == state) return;

        ZS_LOG_BASIC(log("state changed") + ZS_PARAM("old state", toString(mCurrentState)) + ZS_PARAM("new state", toString(state)))
        mCurrentState = state;

        if (!mDelegate) return;

        RUDPListenerPtr pThis = mThisWeak.lock();

        if (pThis) {
          try
          {
            mDelegate->onRUDPListenerStateChanged(mThisWeak.lock(), mCurrentState);
          } catch(IRUDPListenerDelegateProxy::Exceptions::DelegateGone &) {
          }
        }
      }

      //-----------------------------------------------------------------------
      bool RUDPListener::bindUDP()
      {
        mUDPSocket = Socket::createUDP();

        try {
          IPAddress any = IPAddress::anyV4();
          any.setPort(mBindPort);

          mUDPSocket->bind(any);
          mUDPSocket->setBlocking(false);
          mUDPSocket->setDelegate(mThisWeak.lock());
          IPAddress local = mUDPSocket->getLocalAddress();
          mBindPort = local.getPort();
          ZS_THROW_CUSTOM_PROPERTIES_1_IF(Socket::Exceptions::Unspecified, 0 == mBindPort, 0)
        } catch(Socket::Exceptions::Unspecified &) {
          cancel();
          return false;
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool RUDPListener::sendTo(
                                const IPAddress &destination,
                                STUNPacketPtr stun
                                )
      {
        AutoRecursiveLock lock(mLock);

        SecureByteBlockPtr packetized = stun->packetize(STUNPacket::Method_Binding == stun->mMethod ? (stun->mPriorityIncluded ? STUNPacket::RFC_5245_ICE : STUNPacket::RFC_5389_STUN) : STUNPacket::RFC_draft_RUDP);

        return sendTo(destination, *packetized, packetized->SizeInBytes());
      }

      //-----------------------------------------------------------------------
      bool RUDPListener::sendTo(
                                const IPAddress &destination,
                                const BYTE *buffer,
                                size_t bufferLengthInBytes
                                )
      {
        ZS_THROW_INVALID_USAGE_IF(!buffer)
        if (isShutdown()) return false;
        if (0 == bufferLengthInBytes) return false;

        try {
          bool wouldBlock = false;
          size_t bytesSent = mUDPSocket->sendTo(destination, buffer, bufferLengthInBytes, &wouldBlock);
          ZS_LOG_TRACE(log("sendTo called") + ZS_PARAM("destination", destination.string()) + ZS_PARAM("buffer", buffer ? true: false) + ZS_PARAM("buffer length", bufferLengthInBytes) + ZS_PARAM("bytes sent", bytesSent) + ZS_PARAM("would block", wouldBlock))
          return (bytesSent == bufferLengthInBytes);
        } catch(Socket::Exceptions::Unspecified &) {
          ZS_LOG_ERROR(Detail, log("sendTo exception") + ZS_PARAM("destination", destination.string()) + ZS_PARAM("buffer", buffer ? true : false) + ZS_PARAM("buffer length", bufferLengthInBytes))
          return false;
        }
        return false;
      }

      //-----------------------------------------------------------------------
      bool RUDPListener::handledNonce(
                                      const IPAddress &remoteIP,
                                      STUNPacketPtr &stun,
                                      STUNPacketPtr &response
                                      )
      {
        if (STUNPacket::Class_Indication == stun->mClass) return false;             // we don't perform nonce checks here on indications here
        if (STUNPacket::Method_ReliableChannelOpen != stun->mMethod) return false;  // only going to respond to reliable channel open requests for the nonce

        // make sure none of the credential attributes are present
        if (stun->hasAttribute(STUNPacket::Attribute_Nonce)) return false;
        if (stun->hasAttribute(STUNPacket::Attribute_Username)) return false;
        if (stun->hasAttribute(STUNPacket::Attribute_MessageIntegrity)) return false;
        if (stun->hasAttribute(STUNPacket::Attribute_Realm)) return false;

        stun->mErrorCode = STUNPacket::ErrorCode_Unauthorized;
        response = STUNPacket::createErrorResponse(stun);
        fix(response);
        stun->mRealm = mRealm;
        response->mNonce = createNonce(remoteIP, stun, &(mMagic[0]), sizeof(mMagic));
        response->mRealm = mRealm;

        return true;
      }

      //-----------------------------------------------------------------------
      bool RUDPListener::handleUnknownChannel(
                                              const IPAddress &remoteIP,
                                              STUNPacketPtr &stun,
                                              STUNPacketPtr &response
                                              )
      {
        AutoRecursiveLock lock(mLock);
        if (!mDelegate) return false;
        if (mGracefulShutdownReference) return false; // do not open new channels during the graceful shutdown process

        do
        {
          if (STUNPacket::Class_Indication == stun->mClass) return false;   // we don't respond to indications

          // only channel open can be used
          if (STUNPacket::Method_ReliableChannelOpen != stun->mMethod) {
            // sorry, this channel was not found therefor we discard the request
            stun->mErrorCode = STUNPacket::ErrorCode_Unauthorized;
            response = STUNPacket::createErrorResponse(stun);
            fix(response);
            break;
          }

          if ((!stun->hasAttribute(STUNPacket::Attribute_Nonce)) ||
              (!stun->hasAttribute(STUNPacket::Attribute_Username)) ||
              (!stun->hasAttribute(STUNPacket::Attribute_MessageIntegrity)) ||
              (!stun->hasAttribute(STUNPacket::Attribute_Realm)) ||
              (!stun->hasAttribute(STUNPacket::Attribute_NextSequenceNumber)) ||
              (!stun->hasAttribute(STUNPacket::Attribute_CongestionControl)) ||
              (stun->mLocalCongestionControl.size() < 1) ||
              (stun->mRemoteCongestionControl.size() < 1)) {
            // all of these attributes are manditory otherwise the request is considered bad
            stun->mErrorCode = STUNPacket::ErrorCode_BadRequest;
            response = STUNPacket::createErrorResponse(stun);
            fix(response);
            break;
          }

          // make sure the username has the right format
          size_t pos = stun->mUsername.find(":");
          if (String::npos == pos) {
            stun->mErrorCode = STUNPacket::ErrorCode_Unauthorized;
            response = STUNPacket::createErrorResponse(stun);
            fix(response);
            break;
          }

          // the channel is not used so it can be allocated but only if the nonce is still valid
          if (!isNonceValid(remoteIP, stun, &(mMagic[0]), sizeof(mMagic))) {
            stun->mErrorCode = STUNPacket::ErrorCode_StaleNonce;
            response = STUNPacket::createErrorResponse(stun);
            fix(response);
            String nonce = createNonce(remoteIP, stun, &(mMagic[0]), sizeof(mMagic));
            response->mNonce = nonce;
            response->mRealm = stun->mRealm;
            break;
          }

          CryptoPP::AutoSeededRandomPool rng;
          // we have a valid nonce, we will open the channel, but first - pick an unused channel number
          UINT tries = 0;

          WORD channelNumber = 0;
          bool valid = false;
          do
          {
            ++tries;
            if (tries > ORTC_SERVICES_RUDPLISTENER_MAX_ATTEMPTS_TO_FIND_FREE_CHANNEL_NUMBER) {
              stun->mErrorCode = STUNPacket::ErrorCode_InsufficientCapacity;
              response = STUNPacket::createErrorResponse(stun);
              fix(response);
              break;
            }

            rng.GenerateBlock((BYTE *)(&channelNumber), sizeof(channelNumber));
            channelNumber = channelNumber % (ORTC_SERVICES_RUDPLISTENER_CHANNEL_RANGE_END - ORTC_SERVICES_RUDPLISTENER_CHANNEL_RANGE_START);
            channelNumber += ORTC_SERVICES_RUDPLISTENER_CHANNEL_RANGE_START;

            // check to see if the channel was used for this IP before...
            ChannelPair search(remoteIP, channelNumber);
            SessionMap::iterator found = mLocalChannelNumberSessions.find(search);
            valid = (found == mLocalChannelNumberSessions.end());
          } while (!valid);

          if (!valid) break;

          // found a useable channel number therefor create a new session
          ChannelPair local(remoteIP, channelNumber);
          ChannelPair remote(remoteIP, stun->mChannelNumber);

          UseRUDPChannelPtr session = UseRUDPChannel::createForListener(getAssociatedMessageQueue(), mThisWeak.lock(), remoteIP, channelNumber, stun, response);
          if (!response) {
            // there must be a response or it is an error
            stun->mErrorCode = STUNPacket::ErrorCode_BadRequest;
            response = STUNPacket::createErrorResponse(stun);
            fix(response);
            break;
          }
          if (response) {
            if (STUNPacket::Class_ErrorResponse == response->mClass) {
              // do not add the session if there was an error response
              break;
            }
          }

          mLocalChannelNumberSessions[local] = session;
          mRemoteChannelNumberSessions[remote] = session;
          mPendingSessions.push_back(session);

          // inform the delegate of the new session waiting...
          try {
            mDelegate->onRUDPListenerChannelWaiting(mThisWeak.lock());
          } catch(IRUDPListenerDelegateProxy::Exceptions::DelegateGone &) {
            cancel();
            return true;
          }
        } while (false);  // using as a scope rather than as a loop

        return (bool)response;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPListener::CompareChannelPair
      #pragma mark

      //-----------------------------------------------------------------------
      bool RUDPListener::CompareChannelPair::operator()(const ChannelPair &op1, const ChannelPair &op2) const
      {
        if (op1.first < op2.first)
          return true;
        if (op1.first > op2.first)
          return false;

        if (op1.second < op2.second)
          return true;
        return false;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IRUDPListenerFactory
      #pragma mark

      //-----------------------------------------------------------------------
      IRUDPListenerFactory &IRUDPListenerFactory::singleton()
      {
        return RUDPListenerFactory::singleton();
      }

      //-----------------------------------------------------------------------
      RUDPListenerPtr IRUDPListenerFactory::create(
                                                   IMessageQueuePtr queue,
                                                   IRUDPListenerDelegatePtr delegate,
                                                   WORD port,
                                                   const char *realm
                                                   )
      {
        if (this) {}
        return RUDPListener::create(queue, delegate, port, realm);
      }

    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    const char *IRUDPListener::toString(RUDPListenerStates state)
    {
      switch (state)
      {
        case RUDPListenerState_Listening:    return "Listening";
        case RUDPListenerState_ShuttingDown: return "Shutting down";
        case RUDPListenerState_Shutdown:     return "Shutdown";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    IRUDPListenerPtr IRUDPListener::create(
                                           IMessageQueuePtr queue,
                                           IRUDPListenerDelegatePtr delegate,
                                           WORD port,
                                           const char *realm
                                           )
    {
      return internal::IRUDPListenerFactory::singleton().create(queue, delegate, port, realm);
    }
  }
}
