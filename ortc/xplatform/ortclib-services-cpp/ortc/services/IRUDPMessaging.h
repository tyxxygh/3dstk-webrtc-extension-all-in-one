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

#include <ortc/services/types.h>
#include <ortc/services/IRUDPChannel.h>

#include <zsLib/IPAddress.h>
#include <zsLib/Proxy.h>
#include <zsLib/String.h>

#define ORTC_SERVICES_IRDUPMESSAGING_MAX_MESSAGE_SIZE_IN_BYTES (0xFFFFF)

namespace ortc
{
  namespace services
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRUDPMessaging
    #pragma mark

    interaction IRUDPMessaging
    {
      typedef IRUDPChannel::Shutdown Shutdown;

      enum RUDPMessagingStates
      {
        RUDPMessagingState_Connecting =   IRUDPChannel::RUDPChannelState_Connecting,
        RUDPMessagingState_Connected =    IRUDPChannel::RUDPChannelState_Connected,
        RUDPMessagingState_ShuttingDown = IRUDPChannel::RUDPChannelState_ShuttingDown,
        RUDPMessagingState_Shutdown =     IRUDPChannel::RUDPChannelState_Shutdown,
      };

      static const char *toString(RUDPMessagingStates state);

      enum RUDPMessagingShutdownReasons
      {
        RUDPMessagingShutdownReason_None                        = IRUDPChannel::RUDPChannelShutdownReason_None,

        RUDPMessagingShutdownReason_OpenFailure                 = IRUDPChannel::RUDPChannelShutdownReason_OpenFailure,
        RUDPMessagingShutdownReason_DelegateGone                = IRUDPChannel::RUDPChannelShutdownReason_DelegateGone,
        RUDPMessagingShutdownReason_Timeout                     = IRUDPChannel::RUDPChannelShutdownReason_Timeout,
        RUDPMessagingShutdownReason_IllegalStreamState          = IRUDPChannel::RUDPChannelShutdownReason_IllegalStreamState,
      };

      static const char *toString(RUDPMessagingShutdownReasons reason);

      //-----------------------------------------------------------------------
      // PURPOSE: returns a debug object containing internal object state
      static ElementPtr toDebug(IRUDPMessagingPtr messaging);

      //-----------------------------------------------------------------------
      // PURPOSE: Creates a messaging object from an RUDP listener by accepting
      //          the channel and processing it.
      // NOTE:    Will return NULL if the accept failed to open (i.e. the
      //          accepted channel closed before the accept was called).
      static IRUDPMessagingPtr acceptChannel(
                                             IMessageQueuePtr queue,
                                             IRUDPListenerPtr listener,
                                             IRUDPMessagingDelegatePtr delegate,
                                             ITransportStreamPtr receiveStream,
                                             ITransportStreamPtr sendStream,
                                             size_t maxMessageSizeInBytes = ORTC_SERVICES_IRDUPMESSAGING_MAX_MESSAGE_SIZE_IN_BYTES
                                             );

      //-----------------------------------------------------------------------
      // PURPOSE: Creates a messaging object from an RUDP socket session
      //          by accepting the channel and processing it.
      // NOTE:    Will return NULL if the accept failed to open (i.e. the
      //          accepted channel closed before the accept was called).
      static IRUDPMessagingPtr acceptChannel(
                                             IMessageQueuePtr queue,
                                             IRUDPTransportPtr session,
                                             IRUDPMessagingDelegatePtr delegate,
                                             ITransportStreamPtr receiveStream,
                                             ITransportStreamPtr sendStream,
                                             size_t maxMessageSizeInBytes = ORTC_SERVICES_IRDUPMESSAGING_MAX_MESSAGE_SIZE_IN_BYTES
                                             );

      //-----------------------------------------------------------------------
      // PURPOSE: Creates a messaging object from an RUDP socket session
      //          by issuing a openChannel and then processing the results.
      // NOTE:    Will return NULL if the accept failed to open (i.e. the
      //          RUDP session was closed before this was called).
      static IRUDPMessagingPtr openChannel(
                                           IMessageQueuePtr queue,
                                           IRUDPTransportPtr session,
                                           IRUDPMessagingDelegatePtr delegate,
                                           const char *connectionInfo,
                                           ITransportStreamPtr receiveStream,
                                           ITransportStreamPtr sendStream,
                                           size_t maxMessageSizeInBytes = ORTC_SERVICES_IRDUPMESSAGING_MAX_MESSAGE_SIZE_IN_BYTES
                                           );

      virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Get the current state of the messaging
      virtual RUDPMessagingStates getState(
                                           WORD *outLastErrorCode = NULL,
                                           String *outLastErrorReason = NULL
                                           ) const = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: This closes the session gracefully.
      virtual void shutdown() = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: This shutsdown the send/receive state but does not close the
      //          channel session. Use shutdown() to actually shutdown the
      //          channel fully.
      virtual void shutdownDirection(Shutdown state) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Set the maximum size of a message expecting to receive
      virtual void setMaxMessageSizeInBytes(size_t maxMessageSizeInBytes) = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Get the IP address of the connected remote party.
      // NOTE:    IP address will be empty until the session is connected.
      virtual IPAddress getConnectedRemoteIP() = 0;

      //-----------------------------------------------------------------------
      // PURPOSE: Return connection information as reported by the remote party
      // NOTE:    Will return an empty string until connected.
      virtual String getRemoteConnectionInfo() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRUDPMessagingDelegate
    #pragma mark

    interaction IRUDPMessagingDelegate
    {
      typedef services::IRUDPMessagingPtr IRUDPMessagingPtr;
      typedef IRUDPMessaging::RUDPMessagingStates RUDPMessagingStates;

      virtual void onRUDPMessagingStateChanged(
                                               IRUDPMessagingPtr messaging,
                                               RUDPMessagingStates state
                                               ) = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::services::IRUDPMessagingDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::IRUDPMessagingPtr, IRUDPMessagingPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::IRUDPMessagingDelegate::RUDPMessagingStates, RUDPMessagingStates)
ZS_DECLARE_PROXY_METHOD_2(onRUDPMessagingStateChanged, IRUDPMessagingPtr, RUDPMessagingStates)
ZS_DECLARE_PROXY_END()
