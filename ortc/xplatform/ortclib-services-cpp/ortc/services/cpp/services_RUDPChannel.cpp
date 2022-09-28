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

#include <ortc/services/internal/services_RUDPChannel.h>
#include <ortc/services/internal/services_ICESocket.h>
#include <ortc/services/internal/services_IRUDPChannelStream.h>
#include <ortc/services/internal/services_Helper.h>

#include <ortc/services/RUDPPacket.h>

#include <zsLib/Exception.h>
#include <zsLib/helpers.h>
#include <zsLib/Stringize.h>

#include <algorithm>

#define ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS (10*60)

namespace ortc { namespace services { ZS_DECLARE_SUBSYSTEM(org_ortc_services_rudp) } }

namespace ortc
{
  namespace services
  {
    namespace internal
    {
      typedef zsLib::ITimerDelegateProxy ITimerDelegateProxy;

      ZS_DECLARE_TYPEDEF_PTR(IRUDPChannelForRUDPTransport::ForRUDPTransport, ForRUDPTransport)
      ZS_DECLARE_TYPEDEF_PTR(IRUDPChannelForRUDPListener::ForRUDPListener, ForRUDPListener)

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helpers)
      #pragma mark

      //-----------------------------------------------------------------------
      static String sequenceToString(QWORD value)
      {
        return string(value) + " (" + string(value & 0xFFFFFF) + ")";
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IRUDPChannelForRUDPTransport
      #pragma mark

      //-----------------------------------------------------------------------
      ForRUDPTransportPtr IRUDPChannelForRUDPTransport::createForRUDPTransportIncoming(
                                                                                       IMessageQueuePtr queue,
                                                                                       IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                                       const IPAddress &remoteIP,
                                                                                       WORD incomingChannelNumber,
                                                                                       const char *localUsernameFrag,
                                                                                       const char *localPassword,
                                                                                       const char *remoteUsernameFrag,
                                                                                       const char *remotePassword,
                                                                                       STUNPacketPtr channelOpenPacket,
                                                                                       STUNPacketPtr &outResponse
                                                                                       )
      {
        return IRUDPChannelFactory::singleton().createForRUDPTransportIncoming(queue, master, remoteIP, incomingChannelNumber, localUsernameFrag, localPassword, remoteUsernameFrag, remotePassword, channelOpenPacket, outResponse);
      }

      //-----------------------------------------------------------------------
      ForRUDPTransportPtr IRUDPChannelForRUDPTransport::createForRUDPTransportOutgoing(
                                                                                       IMessageQueuePtr queue,
                                                                                       IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                                       IRUDPChannelDelegatePtr delegate,
                                                                                       const IPAddress &remoteIP,
                                                                                       WORD incomingChannelNumber,
                                                                                       const char *localUsernameFrag,
                                                                                       const char *localPassword,
                                                                                       const char *remoteUsernameFrag,
                                                                                       const char *remotePassword,
                                                                                       const char *connectionInfo,
                                                                                       ITransportStreamPtr receiveStream,
                                                                                       ITransportStreamPtr sendStream
                                                                                       )
      {
        return IRUDPChannelFactory::singleton().createForRUDPTransportOutgoing(queue, master, delegate, remoteIP, incomingChannelNumber, localUsernameFrag, localPassword, remoteUsernameFrag, remotePassword, connectionInfo, receiveStream, sendStream);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IRUDPChannelForRUDPListener
      #pragma mark

      ForRUDPListenerPtr IRUDPChannelForRUDPListener::createForListener(
                                                                        IMessageQueuePtr queue,
                                                                        IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                        const IPAddress &remoteIP,
                                                                        WORD incomingChannelNumber,
                                                                        STUNPacketPtr channelOpenPacket,
                                                                        STUNPacketPtr &outResponse
                                                                        )
      {
        return IRUDPChannelFactory::singleton().createForListener(queue, master, remoteIP, incomingChannelNumber, channelOpenPacket, outResponse);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel
      #pragma mark

      //-----------------------------------------------------------------------
      RUDPChannel::RUDPChannel(
                               IMessageQueuePtr queue,
                               IRUDPChannelDelegateForSessionAndListenerPtr master,
                               const IPAddress &remoteIP,
                               const char *localUsernameFrag,
                               const char *localPassword,
                               const char *remoteUsernameFrag,
                               const char *remotePassword,
                               DWORD minimumRTT,
                               DWORD lifetime,
                               WORD incomingChannelNumber,
                               QWORD localSequenceNumber,
                               const char *localChannelInfo,
                               WORD outgoingChannelNumber,
                               QWORD remoteSequenceNumber,
                               const char *remoteChannelInfo
                               ) :
        MessageQueueAssociator(queue),
        mCurrentState(RUDPChannelState_Connecting),
        mMasterDelegate(IRUDPChannelDelegateForSessionAndListenerProxy::createWeak(queue, master)),
        mRemoteIP(remoteIP),
        mLocalUsernameFrag(localUsernameFrag),
        mLocalPassword(localPassword),
        mRemoteUsernameFrag(remoteUsernameFrag),
        mRemotePassword(remotePassword),
        mIncomingChannelNumber(incomingChannelNumber),
        mOutgoingChannelNumber(outgoingChannelNumber),
        mLocalSequenceNumber(localSequenceNumber),
        mRemoteSequenceNumber(remoteSequenceNumber),
        mMinimumRTT(minimumRTT),
        mLifetime(lifetime),
        mLocalChannelInfo(localChannelInfo),
        mRemoteChannelInfo(remoteChannelInfo),
        mLastSentData(zsLib::now()),
        mLastReceivedData(zsLib::now())
      {
        ZS_LOG_DETAIL(log("created"))
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::init()
      {
        AutoRecursiveLock lock(mLock);
        step();
      }

      //-----------------------------------------------------------------------
      RUDPChannel::~RUDPChannel()
      {
        if(isNoop()) return;
        
        mThisWeak.reset();
        ZS_LOG_DETAIL(log("destroyed"))
        cancel(false);
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr RUDPChannel::convert(IRUDPChannelPtr channel)
      {
        return ZS_DYNAMIC_PTR_CAST(RUDPChannel, channel);
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr RUDPChannel::convert(ForRUDPTransportPtr channel)
      {
        return ZS_DYNAMIC_PTR_CAST(RUDPChannel, channel);
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr RUDPChannel::convert(ForRUDPListenerPtr channel)
      {
        return ZS_DYNAMIC_PTR_CAST(RUDPChannel, channel);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => IRUDPChannel
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr RUDPChannel::toDebug(IRUDPChannelPtr channel)
      {
        if (!channel) return ElementPtr();

        RUDPChannelPtr pThis = RUDPChannel::convert(channel);
        return pThis->toDebug();
      }

      //-----------------------------------------------------------------------
      IRUDPChannel::RUDPChannelStates RUDPChannel::getState(
                                                            WORD *outLastErrorCode,
                                                            String *outLastErrorReason
                                                            ) const
      {
        AutoRecursiveLock lock(mLock);
        if (outLastErrorCode) *outLastErrorCode = mLastError;
        if (outLastErrorReason) *outLastErrorReason = mLastErrorReason;
        return mCurrentState;
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::shutdown()
      {
        ZS_LOG_DETAIL(log("shutdown called"))
        AutoRecursiveLock lock(mLock);
        cancel(true);
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::shutdownDirection(Shutdown state)
      {
        AutoRecursiveLock lock(mLock);
        ZS_LOG_DETAIL(log("shutdown direction called") + ZS_PARAM("state", IRUDPChannel::toString(state)) + ZS_PARAM("current shutdown", IRUDPChannel::toString(mShutdownDirection)))
        mShutdownDirection = static_cast<IRUDPChannelStream::Shutdown>(mShutdownDirection | state);
        if (!mStream) return;
        mStream->shutdownDirection(mShutdownDirection);
      }

      //-----------------------------------------------------------------------
      IPAddress RUDPChannel::getConnectedRemoteIP()
      {
        AutoRecursiveLock lock(mLock);
        ZS_LOG_DEBUG(log("get connected remote IP called") + ZS_PARAM("ip", mRemoteIP.string()))
        return mRemoteIP;
      }

      //-----------------------------------------------------------------------
      String RUDPChannel::getRemoteConnectionInfo()
      {
        AutoRecursiveLock lock(mLock);
        ZS_LOG_DEBUG(log("get connection info called") + ZS_PARAM("remote channel info", mRemoteChannelInfo))
        return mRemoteChannelInfo;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => IRUDPChannelForRUDPTransport
      #pragma mark

      //-----------------------------------------------------------------------
      RUDPChannelPtr RUDPChannel::createForRUDPTransportIncoming(
                                                                 IMessageQueuePtr queue,
                                                                 IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                 const IPAddress &remoteIP,
                                                                 WORD incomingChannelNumber,
                                                                 const char *localUsernameFrag,
                                                                 const char *localPassword,
                                                                 const char *remoteUsernameFrag,
                                                                 const char *remotePassword,
                                                                 STUNPacketPtr stun,
                                                                 STUNPacketPtr &outResponse
                                                                 )
      {
        QWORD sequenceNumber = 0;
        DWORD minimumRTT = 0;
        IRUDPChannelStream::CongestionAlgorithmList localAlgorithms;
        IRUDPChannelStream::CongestionAlgorithmList remoteAlgorithms;
        IRUDPChannelStream::getRecommendedStartValues(sequenceNumber, minimumRTT, localAlgorithms, remoteAlgorithms);

        DWORD lifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;
        if (stun->hasAttribute(STUNPacket::Attribute_Lifetime)) {
          lifetime = stun->mLifetime;
        }
        // do not ever negotiate higher
        if (lifetime > ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS)
          lifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;

        if (stun->hasAttribute(STUNPacket::Attribute_MinimumRTT)) {
          minimumRTT = (minimumRTT > stun->mMinimumRTT ? minimumRTT : stun->mMinimumRTT);
        }

        RUDPChannelPtr pThis(new RUDPChannel(
                                             queue,
                                             master,
                                             remoteIP,
                                             localUsernameFrag,
                                             localPassword,
                                             remoteUsernameFrag,
                                             remotePassword,
                                             minimumRTT,
                                             lifetime,
                                             incomingChannelNumber,
                                             sequenceNumber,
                                             NULL,
                                             stun->mChannelNumber,
                                             stun->mNextSequenceNumber,
                                             stun->mConnectionInfo
                                             ));

        pThis->mThisWeak = pThis;

        AutoRecursiveLock lock(pThis->mLock);
        pThis->mIncoming = true;
        pThis->init();
        // do not allow sending to the remote party until we receive an ACK or data
        pThis->mStream = IRUDPChannelStream::create(queue, pThis, pThis->mLocalSequenceNumber, pThis->mRemoteSequenceNumber, pThis->mOutgoingChannelNumber, pThis->mIncomingChannelNumber, pThis->mMinimumRTT);
        pThis->mStream->holdSendingUntilReceiveSequenceNumber(stun->mNextSequenceNumber);
        pThis->handleSTUN(stun, outResponse, localUsernameFrag, remoteUsernameFrag);
        if (!outResponse) {
          ZS_LOG_WARNING(Detail, pThis->log("failed to create a STUN response for the incoming channel so channel must be closed"))
          pThis->setError(RUDPChannelShutdownReason_OpenFailure, "open channel failure");
          pThis->cancel(false);
        }
        if (outResponse) {
          if (STUNPacket::Class_ErrorResponse == outResponse->mClass) {
            ZS_LOG_WARNING(Detail, pThis->log("failed to create an incoming channel as STUN response was a failure"))
            pThis->setError(RUDPChannelShutdownReason_OpenFailure, "open channel failure");
            pThis->cancel(false);
          }
        }
        ZS_LOG_DETAIL(pThis->log("created for socket session incoming") + ZS_PARAM("localUsernameFrag", localUsernameFrag) + ZS_PARAM("remoteUsernameFrag", remoteUsernameFrag) + ZS_PARAM("local password", localPassword) + ZS_PARAM("remote password", remotePassword) + ZS_PARAM("incoming channel", incomingChannelNumber))
        return pThis;
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr RUDPChannel::createForRUDPTransportOutgoing(
                                                                 IMessageQueuePtr queue,
                                                                 IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                 IRUDPChannelDelegatePtr delegate,
                                                                 const IPAddress &remoteIP,
                                                                 WORD incomingChannelNumber,
                                                                 const char *localUsernameFrag,
                                                                 const char *localPassword,
                                                                 const char *remoteUsernameFrag,
                                                                 const char *remotePassword,
                                                                 const char *connectionInfo,
                                                                 ITransportStreamPtr receiveStream,
                                                                 ITransportStreamPtr sendStream
                                                                 )
      {
        QWORD sequenceNumber = 0;
        DWORD minimumRTT = 0;
        IRUDPChannelStream::CongestionAlgorithmList localAlgorithms;
        IRUDPChannelStream::CongestionAlgorithmList remoteAlgorithms;
        IRUDPChannelStream::getRecommendedStartValues(sequenceNumber, minimumRTT, localAlgorithms, remoteAlgorithms);

        RUDPChannelPtr pThis(new RUDPChannel(
                                             queue,
                                             master,
                                             remoteIP,
                                             localUsernameFrag,
                                             localPassword,
                                             remoteUsernameFrag,
                                             remotePassword,
                                             minimumRTT,
                                             ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS,
                                             incomingChannelNumber,
                                             sequenceNumber,
                                             connectionInfo
                                             ));

        pThis->mThisWeak = pThis;
        pThis->mDelegate = IRUDPChannelDelegateProxy::createWeak(queue, delegate);
        pThis->mReceiveStream = receiveStream;
        pThis->mSendStream = sendStream;
        pThis->init();
        // do not allow sending to the remote party until we receive an ACK or data
        ZS_LOG_DETAIL(pThis->log("created for socket session outgoing") + ZS_PARAM("localUserFrag", localUsernameFrag) + ZS_PARAM("remoteUsernameFrag", remoteUsernameFrag) + ZS_PARAM("local password", localPassword) + ZS_PARAM("remote password", remotePassword) + ZS_PARAM("incoming channel", incomingChannelNumber))
        return pThis;
      }
      
      //-----------------------------------------------------------------------
      void RUDPChannel::setDelegate(IRUDPChannelDelegatePtr delegate)
      {
        ZS_LOG_DEBUG(log("set delegate called"))

        AutoRecursiveLock lock(mLock);

        mDelegate = IRUDPChannelDelegateProxy::createWeak(getAssociatedMessageQueue(), delegate);

        try
        {
          if (RUDPChannelState_Connecting != mCurrentState) {
            ZS_LOG_DEBUG(log("delegate notified of channel state change"))
            mDelegate->onRDUPChannelStateChanged(mThisWeak.lock(), mCurrentState);
          }
        } catch(IRUDPChannelDelegateProxy::Exceptions::DelegateGone &) {
          ZS_LOG_ERROR(Basic, log("delegate destroyed during the set"))
          setError(RUDPChannelShutdownReason_DelegateGone, "delegate gone");
          cancel(false);
          return;
        }
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::setStreams(
                                   ITransportStreamPtr receiveStream,
                                   ITransportStreamPtr sendStream
                                   )
      {
        ZS_THROW_INVALID_ARGUMENT_IF(!receiveStream)
        ZS_THROW_INVALID_ARGUMENT_IF(!sendStream)

        ZS_LOG_DEBUG(log("set streams called"))
        AutoRecursiveLock lock(mLock);

        mReceiveStream = receiveStream;
        mSendStream = sendStream;

        if (mStream) {
          mStream->setStreams(receiveStream, sendStream);
        }
      }

      //-----------------------------------------------------------------------
      bool RUDPChannel::handleSTUN(
                                   STUNPacketPtr stun,
                                   STUNPacketPtr &outResponse,
                                   const String &localUsernameFrag,
                                   const String &remoteUsernameFrag
                                   )
      {
        AutoRecursiveLock lock(mLock);
        if (!mMasterDelegate) {
          ZS_LOG_TRACE(log("no master delegate, thus ignoring"))
          return false;
        }

        if ((localUsernameFrag != mLocalUsernameFrag) &&
            (remoteUsernameFrag != mRemoteUsernameFrag)) {
          ZS_LOG_TRACE(log("the request local/remote username frag does not match, thus ignoring") + ZS_PARAM("local:remote username frag", localUsernameFrag + ":" + remoteUsernameFrag) + ZS_PARAM("expected local:remote username frag", mLocalUsernameFrag + ":" + mRemoteUsernameFrag))
          return false;
        }

        if (remoteUsernameFrag.size() < 1) {
          ZS_LOG_TRACE(log("missing remote username frag, thus ignoring") + ZS_PARAM("local:remote username frag", localUsernameFrag + ":" + remoteUsernameFrag) + ZS_PARAM("expected local:remote username frag", mLocalUsernameFrag + ":" + mRemoteUsernameFrag))
          return false;
        }

        // do not allow a change in credentials!
        if ((stun->mRealm != mRealm) ||
            (stun->mNonce != mNonce)) {

          ZS_LOG_ERROR(Detail, log("STUN message credentials mismatch") + ZS_PARAM("expecting username", (mLocalUsernameFrag + ":" + mRemoteUsernameFrag)) + ZS_PARAM("received username", stun->mUsername) + ZS_PARAM("expecting realm", mRealm) + ZS_PARAM("received realm", stun->mRealm) + ZS_PARAM("expecting nonce", mNonce) + ZS_PARAM("received nonce", stun->mNonce))
          if (STUNPacket::Class_Request == stun->mClass) {
            stun->mErrorCode = STUNPacket::ErrorCode_Unauthorized;  // the username doesn't match
            outResponse = STUNPacket::createErrorResponse(stun);
            fix(outResponse);
          }
          return true;
        }

        // make sure the message integrity is correct!
        if (!isValidIntegrity(stun)) {
          ZS_LOG_ERROR(Detail, log("STUN message integrity failed"))
          if (STUNPacket::Class_Request == stun->mClass) {
            stun->mErrorCode = STUNPacket::ErrorCode_Unauthorized;  // the password doesn't match
            outResponse = STUNPacket::createErrorResponse(stun);
            fix(outResponse);
          }
          return true;
        }

        if (STUNPacket::Method_ReliableChannelOpen == stun->mMethod) {
          if (STUNPacket::Class_Request != stun->mClass) {
            ZS_LOG_ERROR(Debug, log("received illegal class on STUN request") + ZS_PARAM("class", (int)stun->mClass))
            return false;  // illegal unless it is a request, responses will come through a different method
          }

          DWORD lifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;
          if (stun->hasAttribute(STUNPacket::Attribute_Lifetime)) {
            lifetime = stun->mLifetime;
          }

          if (0 == lifetime) {
            ZS_LOG_DETAIL(log("received open channel with lifetime of 0 thus closing channel"))

            if (!mStream) {
              // we were never connected, so return an error code...
              ZS_LOG_WARNING(Detail, log("received open channel with lifetime of 0 but the channel is not open"))
              stun->mErrorCode = STUNPacket::ErrorCode_Unauthorized;  // the password doesn't match
              outResponse = STUNPacket::createErrorResponse(stun);
              return true;
            }

            // a lifetime of zero means they are closing the channel
            outResponse = STUNPacket::createResponse(stun);
            fix(stun);
            outResponse->mLifetimeIncluded = true;
            outResponse->mLifetime = 0;
            outResponse->mUsername = mLocalUsernameFrag + ":" + mRemoteUsernameFrag;
            outResponse->mPassword = mLocalPassword;
            outResponse->mRealm = stun->mRealm;
            if (!mRealm.isEmpty()) {
              outResponse->mCredentialMechanism = STUNPacket::CredentialMechanisms_LongTerm;
            } else {
              outResponse->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
            }

            if (mOpenRequest) {
              mOpenRequest->cancel();
              mOpenRequest.reset();
            }

            setError(RUDPChannelShutdownReason_RemoteClosed, "remote channel disconnection received");
            cancel(false);
            return true;
          }

          if (lifetime > ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS)
            lifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;

          // if the lifetime is too low we can't keep up with keep alives so reject it
          if (lifetime < 20) {
            ZS_LOG_ERROR(Detail, log("received open channel with lifetime too low"))
            stun->mErrorCode = STUNPacket::ErrorCode_BadRequest;
            outResponse = STUNPacket::createErrorResponse(stun);
            fix(outResponse);
            return true;
          }

          QWORD sequenceNumber = 0;
          DWORD minimumRTT = 0;
          IRUDPChannelStream::CongestionAlgorithmList localAlgorithms;
          IRUDPChannelStream::CongestionAlgorithmList remoteAlgorithms;
          IRUDPChannelStream::getRecommendedStartValues(sequenceNumber, minimumRTT, localAlgorithms, remoteAlgorithms);

          if (stun->hasAttribute(STUNPacket::Attribute_MinimumRTT)) {
            minimumRTT = (minimumRTT > stun->mMinimumRTT ? minimumRTT : stun->mMinimumRTT);
          }

          // verify all attributes match or we will flag as unauthorized as we don't support renegotiation at this time
          if ((stun->mChannelNumber != mOutgoingChannelNumber) ||
              (minimumRTT != mMinimumRTT) ||
              (stun->mConnectionInfo != mRemoteChannelInfo) ||
              (lifetime != mLifetime) ||
              (stun->mLocalCongestionControl.size() < 1) ||
              (stun->mRemoteCongestionControl.size() < 1)) {
            ZS_LOG_WARNING(Detail, log("received open channel with non supported renegociation"))
            stun->mErrorCode = STUNPacket::ErrorCode_AllocationMismatch;  // sorry, not going to support renegotiation at this time...
            outResponse = STUNPacket::createErrorResponse(stun);
            fix(outResponse);
            return true;
          }

          // do not try to sneak in new congestion controls... we do not support anything but the default right now...
          if ((stun->mLocalCongestionControl.end() == find(stun->mLocalCongestionControl.begin(), stun->mLocalCongestionControl.end(), IRUDPChannel::CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp)) ||
              (stun->mRemoteCongestionControl.end() == find(stun->mRemoteCongestionControl.begin(), stun->mRemoteCongestionControl.end(), IRUDPChannel::CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp))) {
            ZS_LOG_ERROR(Detail, log("received open channel with unsupported congrestion controls"))
            stun->mErrorCode = STUNPacket::ErrorCode_UnsupportedTransportProtocol;
            outResponse = STUNPacket::createErrorResponse(stun);
            fix(outResponse);
            return true;
          }

          mLastReceivedData = zsLib::now();
          outResponse = STUNPacket::createResponse(stun);
          fix(outResponse);
          outResponse->mUsername = mLocalUsernameFrag + ":" + mRemoteUsernameFrag;
          outResponse->mPassword = mLocalPassword;
          outResponse->mRealm = stun->mRealm;
          if (!mRealm.isEmpty()) {
            outResponse->mCredentialMechanism = STUNPacket::CredentialMechanisms_LongTerm;
          } else {
            outResponse->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
          }

          outResponse->mNextSequenceNumber = mLocalSequenceNumber;
          outResponse->mMinimumRTTIncluded = true;
          outResponse->mMinimumRTT = minimumRTT;
          outResponse->mLifetime = lifetime;
          outResponse->mChannelNumber = mIncomingChannelNumber;

          IRUDPChannelStream::getResponseToOfferedAlgorithms(
                                                             stun->mRemoteCongestionControl,        // the remote applies to us
                                                             stun->mLocalCongestionControl,         // the local applies to them
                                                             outResponse->mLocalCongestionControl,  // in reply, local applies to us
                                                             outResponse->mRemoteCongestionControl  // in reply, remote applies to them
                                                             );

          ZS_LOG_DETAIL(log("received open channel succeeded"))
          (IWakeDelegateProxy::create(mThisWeak.lock()))->onWake();
          return true;
        }

        // this has to be a reliable channel ACK or it is not legal
        if (STUNPacket::Method_ReliableChannelACK != stun->mMethod) {
          ZS_LOG_ERROR(Debug, log("received illegal method on STUN request") + ZS_PARAM("method", (int)stun->mMethod))
          return false;
        }

        // only legal to be a request or an indication
        if ((STUNPacket::Class_Request != stun->mClass) &&
            (STUNPacket::Class_Indication != stun->mClass)) {
          ZS_LOG_ERROR(Debug, log("was expecting a request or an indication only for the class on the reliable ACK") + ZS_PARAM("class", (int)stun->mClass))
          return false;
        }

        if (!mStream) return false; // not possible to handle a stream if there is no stream attached

        if ((!stun->hasAttribute(STUNPacket::Attribute_NextSequenceNumber)) ||
            (!stun->hasAttribute(STUNPacket::Attribute_GSNR)) ||
            (!stun->hasAttribute(STUNPacket::Attribute_GSNFR)) ||
            (!stun->hasAttribute(STUNPacket::Attribute_RUDPFlags))) {

          ZS_LOG_ERROR(Detail, log("received external ACK with missing attributes"))
          // all of these attributes are manditory or it is not legal
          if (STUNPacket::Class_Request == stun->mClass) {
            stun->mErrorCode = STUNPacket::ErrorCode_BadRequest;
            outResponse = STUNPacket::createErrorResponse(stun);
            fix(outResponse);
          }
          return true;
        }

        ZS_LOG_TRACE(log("received external ACK request or indication") + ZS_PARAM("sequence number", sequenceToString(stun->mNextSequenceNumber)) + ZS_PARAM("GSNR", sequenceToString(stun->mGSNR)) + ZS_PARAM("GSNFR", sequenceToString(stun->mGSNFR)))

        mLastReceivedData = zsLib::now();
        mStream->handleExternalAck(
                                   0,
                                   stun->mNextSequenceNumber,
                                   stun->mGSNR,
                                   stun->mGSNFR,
                                   stun->hasAttribute(STUNPacket::Attribute_ACKVector) ? stun->mACKVector.get() : NULL,
                                   stun->hasAttribute(STUNPacket::Attribute_ACKVector) ? stun->mACKVectorLength : 0,
                                   (0 != (stun->mReliabilityFlags & RUDPPacket::Flag_VP_VectorParity)),
                                   (0 != (stun->mReliabilityFlags & RUDPPacket::Flag_PG_ParityGSNR)),
                                   (0 != (stun->mReliabilityFlags & RUDPPacket::Flag_XP_XORedParityToGSNFR)),
                                   (0 != (stun->mReliabilityFlags & RUDPPacket::Flag_DP_DuplicatePacket)),
                                   (0 != (stun->mReliabilityFlags & RUDPPacket::Flag_EC_ECNPacket))
                                   );

        if (STUNPacket::Class_Request != stun->mClass) return true; // handled now

        outResponse = STUNPacket::createResponse(stun);
        fix(outResponse);
        fillACK(outResponse);

        return true;
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::handleRUDP(
                                   RUDPPacketPtr rudp,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   )
      {
        IRUDPChannelStreamPtr stream;
        SecureByteBlockPtr newBuffer;

        // scope: do the work in the context of a lock but call the stream outside the lock
        {
          AutoRecursiveLock lock(mLock);
          if (!mStream) {
            ZS_LOG_WARNING(Trace, log("no attached stream to channel thus ignoring RUDP packet"))
            return;
          }

          stream = mStream;

          ZS_LOG_TRACE(log("received RUDP packet") + ZS_PARAM("stream ID", stream->getID()) + ZS_PARAM("length", bufferLengthInBytes))

          newBuffer = IHelper::convertToBuffer(buffer, bufferLengthInBytes);

          // fix the pointer to point to the newly constructed buffer
          if (NULL != rudp->mData)
            rudp->mData = ((*newBuffer) + (rudp->mData - buffer));

          mLastReceivedData = zsLib::now();
        }

        stream->handlePacket(rudp, newBuffer, false);
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::notifyWriteReady()
      {
        IRUDPChannelStreamPtr stream;

        // scope: do the work in the context of a lock but call the stream outside the lock
        {
          AutoRecursiveLock lock(mLock);
          if (!mStream) {
            ZS_LOG_WARNING(Trace, log("received notify write ready but no stream is attached"))
            return;
          }

          stream = mStream;

          ZS_LOG_TRACE(log("received notify write ready") + ZS_PARAM("stream ID", stream->getID()))
        }
        stream->notifySocketWriteReady();
      }

      //-----------------------------------------------------------------------
      WORD RUDPChannel::getIncomingChannelNumber() const
      {
        AutoRecursiveLock lock(mLock);
        return mIncomingChannelNumber;
      }

      //-----------------------------------------------------------------------
      WORD RUDPChannel::getOutgoingChannelNumber() const
      {
        AutoRecursiveLock lock(mLock);
        return mOutgoingChannelNumber;
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::issueConnectIfNotIssued()
      {
        AutoRecursiveLock lock(mLock);

        if ((isShuttingDown()) ||
            (isShutdown())) return;

        if (mOpenRequest) return;

        IRUDPChannelStream::CongestionAlgorithmList local;
        IRUDPChannelStream::CongestionAlgorithmList remote;

        QWORD ignore1 = 0;
        DWORD ignore2 = 0;
        IRUDPChannelStream::getRecommendedStartValues(ignore1, ignore2, local, remote);

        STUNPacketPtr stun = STUNPacket::createRequest(STUNPacket::Method_ReliableChannelOpen);
        fix(stun);
        fillCredentials(stun);
        stun->mLifetimeIncluded = true;
        stun->mLifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;
        stun->mNextSequenceNumber = mLocalSequenceNumber;
        stun->mChannelNumber = mIncomingChannelNumber;
        stun->mMinimumRTTIncluded = true;
        stun->mMinimumRTT = mMinimumRTT;
        stun->mConnectionInfo = mLocalChannelInfo;
        stun->mLocalCongestionControl = local;
        stun->mRemoteCongestionControl = remote;
        if (mRemotePassword.isEmpty()) {
          // we are attempting a server connect, we won't actually put the username on the request but we will generate a username for later
          if (mRemoteUsernameFrag.isEmpty()) {
            mRemoteUsernameFrag = IHelper::randomString(20);
          }
          mRemotePassword = mRemoteUsernameFrag;
          mLocalPassword = mLocalUsernameFrag;
          stun->mUsername.clear();
          stun->mPassword.clear();
          stun->mCredentialMechanism = STUNPacket::CredentialMechanisms_None;
        } else {
          stun->mUsername = (mRemoteUsernameFrag + ":" + mLocalUsernameFrag);
          stun->mPassword = mRemotePassword;
          stun->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
        }
        ZS_LOG_DETAIL(log("issuing channel open request"))
        mOpenRequest = ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), mRemoteIP, stun, STUNPacket::RFC_draft_RUDP);
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::shutdownFromTimeout()
      {
        ZS_LOG_DEBUG(log("shutdown from timeout called"))
        AutoRecursiveLock lock(mLock);
        mSTUNRequestPreviouslyTimedOut = true;
        setError(RUDPChannelShutdownReason_Timeout, "Timeout failure");
        cancel(false);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => IRUDPChannelForRUDPListener
      #pragma mark

      //-----------------------------------------------------------------------
      RUDPChannelPtr RUDPChannel::createForListener(
                                                    IMessageQueuePtr queue,
                                                    IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                    const IPAddress &remoteIP,
                                                    WORD incomingChannelNumber,
                                                    STUNPacketPtr stun,
                                                    STUNPacketPtr &outResponse
                                                    )
      {
        String localUsernameFrag;
        String remoteUsernameFrag;
        size_t pos = stun->mUsername.find(":");
        if (String::npos == pos) {
          localUsernameFrag = stun->mUsername;
          remoteUsernameFrag = stun->mUsername;
        } else {
          // split the string at the fragments
          localUsernameFrag = stun->mUsername.substr(0, pos); // this would be our local username
          remoteUsernameFrag = stun->mUsername.substr(pos+1);  // this would be the remote username
        }

        QWORD sequenceNumber = 0;
        DWORD minimumRTT = 0;
        IRUDPChannelStream::CongestionAlgorithmList localAlgorithms;
        IRUDPChannelStream::CongestionAlgorithmList remoteAlgorithms;
        IRUDPChannelStream::getRecommendedStartValues(sequenceNumber, minimumRTT, localAlgorithms, remoteAlgorithms);

        DWORD lifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;
        if (stun->hasAttribute(STUNPacket::Attribute_Lifetime)) {
          lifetime = stun->mLifetime;
        }
        // do not ever negotiate higher
        if (lifetime > ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS)
          lifetime = ORTC_SERVICES_RUDPCHANNEL_DEFAULT_LIFETIME_IN_SECONDS;

        if (stun->hasAttribute(STUNPacket::Attribute_MinimumRTT)) {
          minimumRTT = (minimumRTT > stun->mMinimumRTT ? minimumRTT : stun->mMinimumRTT);
        }

        RUDPChannelPtr pThis(new RUDPChannel(
                                             queue,
                                             master,
                                             remoteIP,
                                             localUsernameFrag,
                                             localUsernameFrag,
                                             remoteUsernameFrag,
                                             remoteUsernameFrag,
                                             minimumRTT,
                                             lifetime,
                                             incomingChannelNumber,
                                             sequenceNumber,
                                             NULL,
                                             stun->mChannelNumber,
                                             stun->mNextSequenceNumber,
                                             stun->mConnectionInfo
                                             ));

        pThis->mThisWeak = pThis;

        AutoRecursiveLock lock(pThis->mLock);
        pThis->mIncoming = true;
        pThis->mRealm = stun->mRealm;
        pThis->mNonce = stun->mNonce;
        pThis->init();
        // do not allow sending to the remote party until we receive an ACK or data
        pThis->mStream = IRUDPChannelStream::create(queue, pThis, pThis->mLocalSequenceNumber, pThis->mRemoteSequenceNumber, pThis->mOutgoingChannelNumber, pThis->mIncomingChannelNumber, pThis->mMinimumRTT);
        pThis->mStream->holdSendingUntilReceiveSequenceNumber(stun->mNextSequenceNumber);
        pThis->handleSTUN(stun, outResponse, localUsernameFrag, remoteUsernameFrag);
        if (!outResponse) {
          ZS_LOG_WARNING(Detail, pThis->log("failed to provide a STUN response so channel must be closed"))
          pThis->setError(RUDPChannelShutdownReason_OpenFailure, "channel open failure");
          pThis->cancel(false);
        }
        if (outResponse) {
          if (STUNPacket::Class_ErrorResponse == outResponse->mClass) {
            ZS_LOG_WARNING(Detail, pThis->log("channel could not be opened as response was a failure"))
            pThis->setError(RUDPChannelShutdownReason_OpenFailure, "channel open failure");
            pThis->cancel(false);
          }
        }
        ZS_LOG_BASIC(pThis->log("created for listener") + ZS_PARAM("localUserFrag", localUsernameFrag) + ZS_PARAM("remoteUserFrag", remoteUsernameFrag) + ZS_PARAM("incoming channel", string(incomingChannelNumber)))
        return pThis;
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => IWakeDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RUDPChannel::onWake()
      {
        AutoRecursiveLock lock(mLock);
        ZS_LOG_DETAIL(log("on wake"))

        step();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => IRUDPChannelStreamDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RUDPChannel::onRUDPChannelStreamStateChanged(
                                                        IRUDPChannelStreamPtr stream,
                                                        RUDPChannelStreamStates state
                                                        )
      {
        AutoRecursiveLock lock(mLock);
        ZS_LOG_DETAIL(log("notify channel stream shutdown") + ZS_PARAM("stream ID", stream->getID()))

        if (stream != mStream) return;

        if (IRUDPChannelStream::RUDPChannelStreamState_Shutdown != state) return;

        WORD errorCode = 0;
        String reason;
        mStream->getState(&errorCode, &reason);

        if (0 != errorCode) {
          setError(errorCode, reason);
        }

        // the channel stream is shutdown, stop the channel
        cancel(false);
      }

      //-----------------------------------------------------------------------
      bool RUDPChannel::notifyRUDPChannelStreamSendPacket(
                                                          IRUDPChannelStreamPtr stream,
                                                          const BYTE *packet,
                                                          size_t packetLengthInBytes
                                                          )
      {
        ZS_LOG_TRACE(log("notify channel stream send packet") + ZS_PARAM("stream ID", stream->getID()) + ZS_PARAM("length", packetLengthInBytes))
        IRUDPChannelDelegateForSessionAndListenerPtr master;
        IPAddress remoteIP;

        {
          AutoRecursiveLock lock(mLock);
          if (!mMasterDelegate) return false;
          master = mMasterDelegate;
          remoteIP = mRemoteIP;
          mLastSentData = zsLib::now();
        }

        try {
          return master->notifyRUDPChannelSendPacket(mThisWeak.lock(), remoteIP, packet, packetLengthInBytes);
        } catch(IRUDPChannelDelegateForSessionAndListenerProxy::Exceptions::DelegateGone &) {
          ZS_LOG_WARNING(Detail, log("master delegate gone for sent packet"))
          setError(RUDPChannelShutdownReason_DelegateGone, "delegate gone");
          cancel(false);
        }
        return false;
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::onRUDPChannelStreamSendExternalACKNow(
                                                              IRUDPChannelStreamPtr stream,
                                                              bool guarenteeDelivery,
                                                              PUID guarenteeDeliveryRequestID
                                                              )
      {
        ZS_LOG_TRACE(log("notify channel stream send external ACK now") + ZS_PARAM("stream ID", stream->getID()) + ZS_PARAM("gaurantee", guarenteeDelivery))

        IRUDPChannelDelegateForSessionAndListenerPtr master;
        STUNPacketPtr stun;

        IPAddress remoteIP;
        SecureByteBlockPtr packet;

        {
          AutoRecursiveLock lock(mLock);
          if (!mMasterDelegate) return;
          if (!mStream) return;

          master = mMasterDelegate;
          remoteIP = mRemoteIP;

          stun = (guarenteeDelivery ? STUNPacket::createRequest(STUNPacket::Method_ReliableChannelACK) : STUNPacket::createIndication(STUNPacket::Method_ReliableChannelACK));
          fix(stun);
          fillACK(stun);

          if (guarenteeDelivery) {
            ZS_LOG_DETAIL(log("STUN ACK sent via STUN requester") + ZS_PARAM("method", "request"))
            ISTUNRequesterPtr request = ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), mRemoteIP, stun, STUNPacket::RFC_draft_RUDP);
            if (0 == guarenteeDeliveryRequestID) {
              guarenteeDeliveryRequestID = zsLib::createPUID();
            }

            mOutstandingACKs[guarenteeDeliveryRequestID] = request;
            return;
          }

          packet = stun->packetize(STUNPacket::RFC_draft_RUDP);
          ZS_LOG_TRACE(log("STUN ACK sent") + ZS_PARAM("method", "indication") + ZS_PARAM("stun packet size", packet->SizeInBytes()))
          mLastSentData = zsLib::now();
        }

        try {
          master->notifyRUDPChannelSendPacket(mThisWeak.lock(), remoteIP, *packet, packet->SizeInBytes());
        } catch(IRUDPChannelDelegateForSessionAndListenerProxy::Exceptions::DelegateGone &) {
          ZS_LOG_WARNING(Detail, log("master delegate gone for send external ack now"))
          setError(RUDPChannelShutdownReason_DelegateGone, "delegate gone");
          cancel(false);
          return;
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => ISTUNRequesterDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RUDPChannel::onSTUNRequesterSendPacket(
                                                  ISTUNRequesterPtr requester,
                                                  IPAddress destination,
                                                  SecureByteBlockPtr packet
                                                  )
      {
        ZS_LOG_DEBUG(log("notify requester send packet") + ZS_PARAM("ip", destination.string()) + ZS_PARAM("length", packet->SizeInBytes()))

        IRUDPChannelDelegateForSessionAndListenerPtr master;
        bool isShuttdingDown = false;

        {
          AutoRecursiveLock lock(mLock);
          if (!mMasterDelegate) {
            ZS_LOG_WARNING(Detail, log("cannot send STUN packet as master delegate gone"))
            return;
          }
          master = mMasterDelegate;
          mLastSentData = zsLib::now();
          isShuttdingDown = isShuttingDown();
        }

        try {
          bool result = master->notifyRUDPChannelSendPacket(mThisWeak.lock(), destination, *packet, packet->SizeInBytes());
          if ((!result) &&
              (isShuttdingDown)) {
            goto do_shutdown;
          }
        } catch(IRUDPChannelDelegateForSessionAndListenerProxy::Exceptions::DelegateGone &) {
          ZS_LOG_WARNING(Detail, log("cannot send STUN packet as master delegate is now gone"))
          goto do_shutdown;
        }

        return;

      do_shutdown:
        {
          AutoRecursiveLock lock(mLock);
          if (isShuttdingDown) {
            if (mStream) {
              mStream->shutdown(false);
            }
            if (mShutdownRequest) {
              mShutdownRequest->cancel();
            }
          }
          setError(RUDPChannelShutdownReason_DelegateGone, "delegate gone");
          cancel(false);
        }
      }

      //-----------------------------------------------------------------------
      bool RUDPChannel::handleSTUNRequesterResponse(
                                                    ISTUNRequesterPtr requester,
                                                    IPAddress fromIPAddress,
                                                    STUNPacketPtr response
                                                    )
      {
        ZS_LOG_DEBUG(log("notify requester received reply") + ZS_PARAM("ip", fromIPAddress.string()) + response->toDebug())

        AutoRecursiveLock lock(mLock);
        if (!mMasterDelegate) return false;

        if (STUNPacket::Class_Response == response->mClass) {
          if (!isValidIntegrity(response)) {
            ZS_LOG_ERROR(Detail, log("requester response failed integrity"))
            return false;  // nope, not legal reply
          }
          mLastReceivedData = zsLib::now();
        }

        if (requester == mShutdownRequest) {
          if (handleStaleNonce(mShutdownRequest, response)) return true;

          ZS_LOG_DETAIL(log("shutdown requester completed"))

          if (mStream) {
            mStream->shutdown(false);
          }

          (IWakeDelegateProxy::create(mThisWeak.lock()))->onWake();
          return true;
        }

        if (requester == mOpenRequest) {
          if (handleStaleNonce(mOpenRequest, response)) return true;

          if (STUNPacket::Class_ErrorResponse == response->mClass) {
            if (STUNPacket::ErrorCode_Unauthorized == response->mErrorCode) {
              // this request is unauthorized but perhaps because we have to reissue the request now that server has given us a nonce?
              STUNPacketPtr originalRequest = requester->getRequest();
              if (originalRequest->mUsername.isEmpty()) {
                ZS_LOG_DETAIL(log("open unauthorized-challenge") + ZS_PARAM("realm", response->mRealm) + ZS_PARAM("nonce", response->mNonce))

                // correct, we have to re-issue the request now
                STUNPacketPtr newRequest = originalRequest->clone(true);
                mNonce = response->mNonce;
                mRealm = response->mRealm;
                fillCredentials(newRequest);
                // reissue the request
                mOpenRequest = ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), mRemoteIP, newRequest, STUNPacket::RFC_draft_RUDP);
                return true;
              }
              // nope, that's not the issue, we have failed...
            }

            if (mOpenRequest) {
              mOpenRequest->cancel();
              mOpenRequest.reset();
            }

            ZS_LOG_WARNING(Detail, log("failed to open as error response was received"))
            setError(RUDPChannelShutdownReason_OpenFailure, "open failure");
            cancel(false);  // failed to open, we are done...
            return true;
          }

          // these attributes must be present in the response of it is illegal
          if ((!response->hasAttribute(STUNPacket::Attribute_ChannelNumber)) ||
              (!response->hasAttribute(STUNPacket::Attribute_NextSequenceNumber))) {
            ZS_LOG_ERROR(Detail, log("open missing attributes") + ZS_PARAM("channel", response->mChannelNumber) + ZS_PARAM("sequence number", sequenceToString(response->mNextSequenceNumber)) + ZS_PARAM("congestion local", response->mLocalCongestionControl.size()) + ZS_PARAM("congestion remote", response->mRemoteCongestionControl.size()))
            return false;
          }

          if (response->hasAttribute(STUNPacket::Attribute_MinimumRTT)) {
            if (response->mMinimumRTT < mMinimumRTT) {
              ZS_LOG_ERROR(Detail, log("remote party attempting lower min RTT") + ZS_PARAM("min", mMinimumRTT) + ZS_PARAM("reply min", response->mMinimumRTT))
              return false;  // not legal to negotiate a lower minimum RRT
            }
            mMinimumRTT = response->mMinimumRTT;
          }

          if (response->hasAttribute(STUNPacket::Attribute_Lifetime)) {
            if (response->mLifetime > mLifetime) {
              ZS_LOG_ERROR(Detail, log("remote party attempting raise lifetime") + ZS_PARAM("lifetime", mLifetime) + ZS_PARAM("reply min", response->mLifetime))
              return false;  // not legal to negotiate a longer lifetime
            }
            mLifetime = response->mLifetime;
          }

          if (response->mLocalCongestionControl.size() > 0) {
            if (response->mLocalCongestionControl.begin() != find(response->mLocalCongestionControl.begin(), response->mLocalCongestionControl.end(), IRUDPChannel::CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp)) {
              ZS_LOG_ERROR(Detail, log("remote party did not offer valid local congestion control"))
              return false;
            }
          }

          if (response->mRemoteCongestionControl.size() > 0) {
            if (response->mRemoteCongestionControl.begin() != find(response->mRemoteCongestionControl.begin(), response->mRemoteCongestionControl.end(), IRUDPChannel::CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp)) {
              ZS_LOG_ERROR(Detail, log("remote party did not offer valid remote congestion control"))
              return false;
            }
          }

          // this is not legal if they don't support our encoders/decoders as the first preference since that is what we asked for
          if ((response->mLocalCongestionControl.begin() != find(response->mLocalCongestionControl.begin(), response->mLocalCongestionControl.end(), IRUDPChannel::CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp)) ||
              (response->mRemoteCongestionControl.begin() != find(response->mRemoteCongestionControl.begin(), response->mRemoteCongestionControl.end(), IRUDPChannel::CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp))) {
            ZS_LOG_ERROR(Detail, log("remote party did not select our congestion control algorithm"))
            return false;
          }

          mRemoteSequenceNumber = response->mNextSequenceNumber;
          mOutgoingChannelNumber = response->mChannelNumber;

          ZS_LOG_DETAIL(log("open") + ZS_PARAM("local sequence number", sequenceToString(mLocalSequenceNumber)) + ZS_PARAM("local sequence number", sequenceToString(mRemoteSequenceNumber)) + ZS_PARAM("outgoing channel number", mOutgoingChannelNumber) + ZS_PARAM("incoming channel number", mIncomingChannelNumber))

          // we should have enough to open our stream now!
          mStream = IRUDPChannelStream::create(
                                               getAssociatedMessageQueue(),
                                               mThisWeak.lock(),
                                               mLocalSequenceNumber,
                                               mRemoteSequenceNumber,
                                               mOutgoingChannelNumber,
                                               mIncomingChannelNumber,
                                               mMinimumRTT
                                               );

          if ((mReceiveStream) &&
              (mSendStream)) {
            mStream->setStreams(mReceiveStream, mSendStream);
          }

          if (IRUDPChannel::Shutdown_None != mShutdownDirection)
            mStream->shutdownDirection(mShutdownDirection);

          step();
          return true;
        }

        for (ACKRequestMap::iterator iter = mOutstandingACKs.begin(); iter != mOutstandingACKs.end(); ++iter) {
          if ((*iter).second == requester) {
            if (handleStaleNonce((*iter).second, response)) return true;

            if (STUNPacket::Class_ErrorResponse == response->mClass) {
              ZS_LOG_ERROR(Detail, log("shutting down channel as ACK failed") + ZS_PARAM("error code", response->mErrorCode) + ZS_PARAM("reason", STUNPacket::toString((STUNPacket::ErrorCodes)response->mErrorCode)))
              mOutstandingACKs.erase(iter);
              if (mStream) {
                mStream->shutdown(false);
              }
              setError(RUDPChannelShutdownReason_IllegalStreamState, "illegal stream state");
              cancel(false);
              return true;
            }

            if ((!response->hasAttribute(STUNPacket::Attribute_NextSequenceNumber)) ||
                (!response->hasAttribute(STUNPacket::Attribute_GSNR)) ||
                (!response->hasAttribute(STUNPacket::Attribute_GSNFR)) ||
                (!response->hasAttribute(STUNPacket::Attribute_RUDPFlags))) {
              ZS_LOG_ERROR(Detail, log("ACK reply missing attributes (thus being ignored)") + ZS_PARAM("sequence number", sequenceToString(response->mNextSequenceNumber)) + ZS_PARAM("GSNR", sequenceToString(response->mGSNR)) + ZS_PARAM("GSNFR", sequenceToString(response->mGSNFR)) + ZS_PARAM("flags", ((WORD)response->mReliabilityFlags)))
              // all of these attributes are manditory or it is not legal - so ignore the response
              return false;
            }

            ZS_LOG_DETAIL(log("handle external ACK"))
            mStream->handleExternalAck(
                                       (*iter).first, // the ACK ID
                                       response->mNextSequenceNumber,
                                       response->mGSNR,
                                       response->mGSNFR,
                                       response->hasAttribute(STUNPacket::Attribute_ACKVector) ? response->mACKVector.get() : NULL,
                                       response->hasAttribute(STUNPacket::Attribute_ACKVector) ? response->mACKVectorLength : 0,
                                       (0 != (response->mReliabilityFlags & RUDPPacket::Flag_VP_VectorParity)),
                                       (0 != (response->mReliabilityFlags & RUDPPacket::Flag_PG_ParityGSNR)),
                                       (0 != (response->mReliabilityFlags & RUDPPacket::Flag_XP_XORedParityToGSNFR)),
                                       (0 != (response->mReliabilityFlags & RUDPPacket::Flag_DP_DuplicatePacket)),
                                       (0 != (response->mReliabilityFlags & RUDPPacket::Flag_EC_ECNPacket))
                                       );
            mOutstandingACKs.erase(iter);
            // the response has been processed
            return true;
          }
        }

        return false;
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::onSTUNRequesterTimedOut(ISTUNRequesterPtr requester)
      {
        AutoRecursiveLock lock(mLock);
        ZS_LOG_ERROR(Detail, log("STUN requester timeout"))

        // any timeout is considered fatal
        if (mStream) {
          mStream->shutdown(false);
        }

        mSTUNRequestPreviouslyTimedOut = true;
        setError(RUDPChannelShutdownReason_Timeout, "channel timeout");
        cancel(false);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => ITimerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RUDPChannel::onTimer(ITimerPtr timer)
      {
        AutoRecursiveLock lock(mLock);
        if (!mStream) return;

        if (mOutstandingACKs.size() > 0) return;  // don't add to the queue

        Time current = zsLib::now();

        DWORD actualLifeTime = (mLifetime > 120 ? mLifetime - 60 : mLifetime);

        if (mLastSentData + Seconds(actualLifeTime) < current) {
          ZS_LOG_DEBUG(log("timer haven't sent data in a while fired"))
          onRUDPChannelStreamSendExternalACKNow(mStream, true, 0);
          mLastSentData = current;
        }

        if (mOutstandingACKs.size() > 0) return;  // don't add to the queue

        actualLifeTime = (mLifetime > 90 ? mLifetime - 60 : mLifetime);
        if (mLastReceivedData + Seconds(actualLifeTime) < current) {
          ZS_LOG_DEBUG(log("timer lifetime refresh required"))
          onRUDPChannelStreamSendExternalACKNow(mStream, true, 0);
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RUDPChannel => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RUDPChannel::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("RUDPChannel");
        IHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      Log::Params RUDPChannel::debug(const char *message) const
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::fix(STUNPacketPtr stun) const
      {
        stun->mLogObject = "RUDPChannel";
        stun->mLogObjectID = mID;
      }

      //-----------------------------------------------------------------------
      ElementPtr RUDPChannel::toDebug() const
      {
        AutoRecursiveLock lock(mLock);
        ElementPtr resultEl = Element::create("RUDPChannel");

        IHelper::debugAppend(resultEl, "id", mID);
        IHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

        IHelper::debugAppend(resultEl, "incoming", mIncoming);

        IHelper::debugAppend(resultEl, "state", IRUDPChannel::toString(mCurrentState));
        IHelper::debugAppend(resultEl, "last error", mLastError);
        IHelper::debugAppend(resultEl, "last reason", mLastErrorReason);

        IHelper::debugAppend(resultEl, "delegate", (bool)mDelegate);
        IHelper::debugAppend(resultEl, "master delegate", (bool)mMasterDelegate);

        IHelper::debugAppend(resultEl, "stream", (bool)mStream);
        IHelper::debugAppend(resultEl, "open request", (bool)mOpenRequest);
        IHelper::debugAppend(resultEl, "shutdown request", (bool)mShutdownRequest);
        IHelper::debugAppend(resultEl, "stun request previously timed out", mSTUNRequestPreviouslyTimedOut);

        IHelper::debugAppend(resultEl, "timer", (bool)mTimer);

        IHelper::debugAppend(resultEl, "shutdown direction", IRUDPChannel::toString(mShutdownDirection));

        IHelper::debugAppend(resultEl, "remote ip", !mRemoteIP.isEmpty() ? mRemoteIP.string() : String());

        IHelper::debugAppend(resultEl, "local username frag", mLocalUsernameFrag);
        IHelper::debugAppend(resultEl, "local password", mLocalPassword);
        IHelper::debugAppend(resultEl, "remote username frag", mRemoteUsernameFrag);
        IHelper::debugAppend(resultEl, "remote password", mRemotePassword);

        IHelper::debugAppend(resultEl, "realm", mRealm);
        IHelper::debugAppend(resultEl, "nonce", mNonce);

        IHelper::debugAppend(resultEl, "incoming channel number", mIncomingChannelNumber);
        IHelper::debugAppend(resultEl, "outgoing channel number", mOutgoingChannelNumber);

        IHelper::debugAppend(resultEl, "local sequence number", mLocalSequenceNumber);
        IHelper::debugAppend(resultEl, "remote sequence number", mRemoteSequenceNumber);

        IHelper::debugAppend(resultEl, "minimum RTT", mMinimumRTT);
        IHelper::debugAppend(resultEl, "lifetime", mLifetime);

        IHelper::debugAppend(resultEl, "local channel info", mLocalChannelInfo);
        IHelper::debugAppend(resultEl, "remote channel info", mRemoteChannelInfo);

        IHelper::debugAppend(resultEl, "last sent data", mLastSentData);
        IHelper::debugAppend(resultEl, "last received data", mLastReceivedData);

        IHelper::debugAppend(resultEl, "outstanding acks", mOutstandingACKs.size());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::cancel(bool waitForDataToBeSent)
      {
        AutoRecursiveLock lock(mLock); // just in case...

        if (isShutdown()) {
          ZS_LOG_DEBUG(log("already shutdown"))
          return;
        }

        ZS_LOG_DEBUG(log("cancel called"))

        if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

        // we don't need any external ACKs going on to clutter the traffic
        for (ACKRequestMap::iterator iter = mOutstandingACKs.begin(); iter != mOutstandingACKs.end(); ++iter)
        {
          ((*iter).second)->cancel();
        }
        mOutstandingACKs.clear();

        if (mTimer) {
          mTimer->cancel();
          mTimer.reset();
        }

        setState(RUDPChannelState_ShuttingDown);

        if (mStream) {
          mStream->shutdown(waitForDataToBeSent);
        }

        if (mOpenRequest) {
          mOpenRequest->cancel();
          // WARNING: DO NOT CALL RESET ON mOpenRequest HERE
        }

        if (mGracefulShutdownReference) {

          if (mStream) {
            if (IRUDPChannelStream::RUDPChannelStreamState_Shutdown != mStream->getState()) {
              ZS_LOG_DEBUG(log("waiting for RUDP channel stream to shutdown"))
              return;
            }

            if (!mSTUNRequestPreviouslyTimedOut) {

              // if we had a successful open request then we must shutdown
              if ((mOpenRequest) &&
                  (!mShutdownRequest)) {
                // create the shutdown request...
                STUNPacketPtr stun = STUNPacket::createRequest(STUNPacket::Method_ReliableChannelOpen);
                fix(stun);
                fillCredentials(stun);
                stun->mLifetimeIncluded = true;
                stun->mLifetime = 0;
                stun->mChannelNumber = mIncomingChannelNumber;
                mShutdownRequest = ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), mRemoteIP, stun, STUNPacket::RFC_draft_RUDP);
              }
            }

            if (mShutdownRequest) {
              // see if the master is still around (if not there is no possibility to send the packet so cancel the object immediately)
              if (!(IRUDPChannelDelegateForSessionAndListenerProxy::create(mMasterDelegate))) {
                mShutdownRequest->cancel();
              }

              if (!mShutdownRequest->isComplete()) {
                ZS_LOG_DEBUG(log("waiting for RUDP channel shutdown request to complete"))
                return;
              }
            }
          }
        }

        setState(RUDPChannelState_Shutdown);

        // stop sending any more open requests
        if (mOpenRequest) {
          mOpenRequest->cancel();
          mOpenRequest.reset();
        }

        mGracefulShutdownReference.reset();
        mMasterDelegate.reset();
        mDelegate.reset();

        if (mShutdownRequest) {
          mShutdownRequest->cancel();
          mShutdownRequest.reset();
        }

        if (mStream) {
          mStream->shutdown(false);
          mStream.reset();
        }

        ZS_LOG_DEBUG(log("cancel complete"))
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::step()
      {
        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_DEBUG(log("step passing to cancel due to shutting down/shutdown state"))
          cancel(true);
          return;
        }

        ZS_LOG_DEBUG(debug("step"))

        if (mStream) {
          // we need to create a timer now we have a stream
          if (!mTimer) {
            mTimer = ITimer::create(mThisWeak.lock(), Seconds(20));  // fire ever 20 seconds
            (ITimerDelegateProxy::create(mThisWeak.lock()))->onTimer(mTimer);
          }

          setState(RUDPChannelState_Connected);
        }
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::setState(RUDPChannelStates state)
      {
        if (mCurrentState == state) return;
        ZS_LOG_DETAIL(log("state changed") + ZS_PARAM("old state", toString(mCurrentState)) + ZS_PARAM("new state", toString(state)))

        mCurrentState = state;

        RUDPChannelPtr pThis = mThisWeak.lock();

        if (pThis) {
          if (mMasterDelegate) {
            try {
              mMasterDelegate->onRUDPChannelStateChanged(pThis, mCurrentState);
            } catch(IRUDPChannelDelegateForSessionAndListenerProxy::Exceptions::DelegateGone &) {
            }
          }

          if (mDelegate) {
            try {
              mDelegate->onRDUPChannelStateChanged(pThis, mCurrentState);
            } catch(IRUDPChannelDelegateProxy::Exceptions::DelegateGone &) {
            }
          }
        }
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::setError(WORD errorCode, const char *inReason)
      {
        String reason(inReason ? String(inReason) : String());
        if (reason.isEmpty()) {
          reason = IHTTP::toString(IHTTP::toStatusCode(errorCode));
        }

        if ((isShuttingDown()) ||
            (isShutdown())) {
          ZS_LOG_WARNING(Detail, debug("already shutting down thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
          return;
        }

        if (0 != mLastError) {
          ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
          return;
        }

        mLastError = errorCode;
        mLastErrorReason = reason;

        ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("code", mLastError) + ZS_PARAM("reason", mLastErrorReason))
      }

      //-----------------------------------------------------------------------
      bool RUDPChannel::isValidIntegrity(STUNPacketPtr stun)
      {
        if ((STUNPacket::Class_Request == stun->mClass) ||
            (STUNPacket::Class_Indication == stun->mClass)) {

          // this is an incoming request
          if (!mRealm.isEmpty())
            return stun->isValidMessageIntegrity(mLocalPassword, (mLocalUsernameFrag + ":" + mRemoteUsernameFrag).c_str(), mRealm);

          return stun->isValidMessageIntegrity(mLocalPassword);
        }

        // this is a reply to an outgoing request
        if (!mRealm.isEmpty())
          return stun->isValidMessageIntegrity(mRemotePassword, (mRemoteUsernameFrag + ":" + mLocalUsernameFrag).c_str(), mRealm);

        return stun->isValidMessageIntegrity(mRemotePassword);
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::fillCredentials(STUNPacketPtr &outSTUN)
      {
        if ((STUNPacket::Class_Request == outSTUN->mClass) ||
            (STUNPacket::Class_Indication == outSTUN->mClass)) {
          outSTUN->mUsername = mRemoteUsernameFrag + ":" + mLocalUsernameFrag;
          outSTUN->mPassword = mRemotePassword;
        } else {
          outSTUN->mUsername = mLocalUsernameFrag + ":" + mRemoteUsernameFrag;
          outSTUN->mPassword = mLocalPassword;
        }
        outSTUN->mRealm = mRealm;
        outSTUN->mNonce = mNonce;
        if (!mRealm.isEmpty()) {
          outSTUN->mCredentialMechanism = STUNPacket::CredentialMechanisms_LongTerm;
        } else {
          outSTUN->mCredentialMechanism = STUNPacket::CredentialMechanisms_ShortTerm;
        }
      }

      //-----------------------------------------------------------------------
      void RUDPChannel::fillACK(STUNPacketPtr &outSTUN)
      {
        ZS_THROW_INVALID_ASSUMPTION_IF(!mStream)

        fillCredentials(outSTUN);

        outSTUN->mSoftware.clear(); // do not include the software attribute

        // we need to pre-fill with bogus data so we can calculate the room available for a vector
        outSTUN->mNextSequenceNumber = 1;
        outSTUN->mGSNR = 1;
        outSTUN->mGSNFR = 1;
        outSTUN->mChannelNumber = mIncomingChannelNumber;
        outSTUN->mReliabilityFlagsIncluded = true;
        outSTUN->mReliabilityFlags = 0;

        size_t available = outSTUN->getTotalRoomAvailableForData(ORTC_SERVICES_RUDP_MAX_PACKET_SIZE_WHEN_PMTU_IS_NOT_KNOWN, STUNPacket::RFC_draft_RUDP);

        std::unique_ptr<BYTE[]> vector;
        if (available > 0) {
          vector = std::unique_ptr<BYTE[]>(new BYTE[available]);
        }

        QWORD nextSequenceNumber = 0;
        QWORD gsnr = 0;
        QWORD gsnfr = 0;
        size_t vectorOutputSize = 0;
        bool vpFlag = false;
        bool pgFlag = false;
        bool xpFlag = false;
        bool dpFlag = false;
        bool ecFlag = false;
        mStream->getState(
                          nextSequenceNumber,
                          gsnr,
                          gsnfr,
                          (0 != available ? vector.get() : NULL),
                          vectorOutputSize,
                          (0 != available ? available : 0),
                          vpFlag,
                          pgFlag,
                          xpFlag,
                          dpFlag,
                          ecFlag
                          );

        mStream->notifyExternalACKSent(gsnr);

        outSTUN->mNextSequenceNumber = nextSequenceNumber;
        outSTUN->mGSNR = gsnr;
        outSTUN->mGSNFR = gsnfr;
        if (0 != vectorOutputSize) {
          outSTUN->mACKVector = std::move(vector);
        } else {
          outSTUN->mACKVector.reset();
        }
        outSTUN->mACKVectorLength = vectorOutputSize;
        outSTUN->mReliabilityFlagsIncluded = true;
        outSTUN->mReliabilityFlags = ((vpFlag ? RUDPPacket::Flag_VP_VectorParity : 0) |
                                      (pgFlag ? RUDPPacket::Flag_PG_ParityGSNR : 0) |
                                      (xpFlag ? RUDPPacket::Flag_XP_XORedParityToGSNFR : 0) |
                                      (dpFlag ? RUDPPacket::Flag_DP_DuplicatePacket : 0) |
                                      (ecFlag ? RUDPPacket::Flag_EC_ECNPacket : 0));
      }

      //-----------------------------------------------------------------------
      bool RUDPChannel::handleStaleNonce(
                                         ISTUNRequesterPtr &originalRequestVariable,
                                         STUNPacketPtr response
                                         )
      {
        if (STUNPacket::Class_ErrorResponse != response->mClass) return false;
        if (STUNPacket::ErrorCode_StaleNonce != response->mErrorCode) return false;

        if (!response->mNonce) return false;

        STUNPacketPtr originalSTUN = originalRequestVariable->getRequest();
        if (originalSTUN->mTotalRetries > 0) return false;

        STUNPacketPtr stun = originalSTUN->clone(true);
        mNonce = response->mNonce;
        if (!response->mRealm.isEmpty())
          mRealm = response->mRealm;

        stun->mTotalRetries = originalSTUN->mTotalRetries;
        ++(stun->mTotalRetries);
        stun->mNonce = mNonce;
        stun->mRealm = mRealm;

        originalRequestVariable = ISTUNRequester::create(getAssociatedMessageQueue(), mThisWeak.lock(), mRemoteIP, stun, STUNPacket::RFC_draft_RUDP);
        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IRUDPChannelFactory
      #pragma mark

      //-----------------------------------------------------------------------
      IRUDPChannelFactory &IRUDPChannelFactory::singleton()
      {
        return RUDPChannelFactory::singleton();
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr IRUDPChannelFactory::createForRUDPTransportIncoming(
                                                                         IMessageQueuePtr queue,
                                                                         IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                         const IPAddress &remoteIP,
                                                                         WORD incomingChannelNumber,
                                                                         const char *localUserFrag,
                                                                         const char *localPassword,
                                                                         const char *remoteUserFrag,
                                                                         const char *remotePassword,
                                                                         STUNPacketPtr channelOpenPacket,
                                                                         STUNPacketPtr &outResponse
                                                                         )
      {
        if (this) {}
        return RUDPChannel::createForRUDPTransportIncoming(queue, master, remoteIP, incomingChannelNumber, localUserFrag, localPassword, remoteUserFrag, remotePassword, channelOpenPacket, outResponse);
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr IRUDPChannelFactory::createForRUDPTransportOutgoing(
                                                                         IMessageQueuePtr queue,
                                                                         IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                                         IRUDPChannelDelegatePtr delegate,
                                                                         const IPAddress &remoteIP,
                                                                         WORD incomingChannelNumber,
                                                                         const char *localUserFrag,
                                                                         const char *localPassword,
                                                                         const char *remoteUserFrag,
                                                                         const char *remotePassword,
                                                                         const char *connectionInfo,
                                                                         ITransportStreamPtr receiveStream,
                                                                         ITransportStreamPtr sendStream
                                                                         )
      {
        if (this) {}
        return RUDPChannel::createForRUDPTransportOutgoing(queue, master, delegate, remoteIP, incomingChannelNumber, localUserFrag, localPassword, remoteUserFrag, remotePassword, connectionInfo, receiveStream, sendStream);
      }

      //-----------------------------------------------------------------------
      RUDPChannelPtr IRUDPChannelFactory::createForListener(
                                                            IMessageQueuePtr queue,
                                                            IRUDPChannelDelegateForSessionAndListenerPtr master,
                                                            const IPAddress &remoteIP,
                                                            WORD incomingChannelNumber,
                                                            STUNPacketPtr channelOpenPacket,
                                                            STUNPacketPtr &outResponse
                                                            )
      {
        if (this) {}
        return RUDPChannel::createForListener(queue, master, remoteIP, incomingChannelNumber, channelOpenPacket, outResponse);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRUDPChannel
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IRUDPChannel::toString(RUDPChannelStates state)
    {
      switch (state)
      {
        case RUDPChannelState_Connecting:   return "Connecting";
        case RUDPChannelState_Connected:    return "Connected";
        case RUDPChannelState_ShuttingDown: return "Shutting down";
        case RUDPChannelState_Shutdown:     return "Shutdown";
        default: break;
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    const char *IRUDPChannel::toString(RUDPChannelShutdownReasons reason)
    {
      switch (reason)
      {
        case RUDPChannelShutdownReason_None:                return "None";

        case RUDPChannelShutdownReason_RemoteClosed:        return "Remote closed";
        case RUDPChannelShutdownReason_OpenFailure:         return "Open failure";
        case RUDPChannelShutdownReason_DelegateGone:        return "Delegate gone";
        case RUDPChannelShutdownReason_Timeout:             return "Timeout";
        case RUDPChannelShutdownReason_IllegalStreamState:  return "Illegal stream state";
      }
      return IHTTP::toString(IHTTP::toStatusCode(reason));
    }

    //-------------------------------------------------------------------------
    const char *IRUDPChannel::toString(Shutdown value)
    {
      switch (value)
      {
        case Shutdown_None:     return "None";
        case Shutdown_Send:     return "Send";
        case Shutdown_Receive:  return "Receive";
        case Shutdown_Both:     return "Both";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    const char *IRUDPChannel::toString(CongestionAlgorithms value)
    {
      switch (value)
      {
        case CongestionAlgorithm_None:                          return "None";
        case CongestionAlgorithm_TCPLikeWindowWithSlowCreepUp:  return "TCP-like Window with slow creep up";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    ElementPtr IRUDPChannel::toDebug(IRUDPChannelPtr channel)
    {
      return internal::RUDPChannel::toDebug(channel);
    }

  }
}
