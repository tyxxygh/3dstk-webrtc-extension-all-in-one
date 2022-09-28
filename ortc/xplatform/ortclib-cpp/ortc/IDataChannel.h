/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/types.h>
#include <ortc/IDataChannelTypes.h>
#include <ortc/IStatsProvider.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannel
  #pragma mark
  
  interaction IDataChannel : public Any,
                             public IDataChannelTypes,
                             public IStatsProvider
  {
    static ElementPtr toDebug(IDataChannelPtr channel);

    static IDataChannelPtr create(
                                  IDataChannelDelegatePtr delegate,
                                  IDataTransportPtr transport,
                                  const Parameters &params
                                  ) throw (InvalidParameters, InvalidStateError);

    virtual PUID getID() const = 0;

    virtual IDataChannelSubscriptionPtr subscribe(IDataChannelDelegatePtr delegate) = 0;

    virtual IDataTransportPtr transport() const = 0;

    virtual ParametersPtr parameters() const = 0;

    virtual States readyState() const = 0;

    virtual size_t bufferedAmount() const = 0;
    virtual size_t bufferedAmountLowThreshold() const = 0;
    virtual void bufferedAmountLowThreshold(size_t value) = 0;

    virtual String binaryType() const = 0;
    virtual void binaryType(const char *str) = 0;

    virtual void close() = 0;

    virtual void send(const String &data) = 0;
    virtual void send(const SecureByteBlock &data) = 0;
    virtual void send(
                      const BYTE *buffer,
                      size_t bufferSizeInBytes
                      ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannelDelegate
  #pragma mark

  interaction IDataChannelDelegate
  {
    ZS_DECLARE_STRUCT_PTR(MessageEventData)

    typedef IDataChannelTypes::States States;

    struct MessageEventData
    {
      SecureByteBlockPtr mBinary;
      String mText;
    };

    virtual void onDataChannelStateChange(
                                          IDataChannelPtr channel,
                                          IDataChannelTypes::States state
                                          ) = 0;

    virtual void onDataChannelError(
                                    IDataChannelPtr channel,
                                    ErrorAnyPtr error
                                    ) = 0;

    virtual void onDataChannelBufferedAmountLow(IDataChannelPtr channel) = 0;

    virtual void onDataChannelMessage(
                                      IDataChannelPtr channel,
                                      MessageEventDataPtr data
                                      ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannelSubscription
  #pragma mark

  interaction IDataChannelSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}


ZS_DECLARE_PROXY_BEGIN(ortc::IDataChannelDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDataChannelTypes::States, States)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDataChannelDelegate::MessageEventDataPtr, MessageEventDataPtr)
ZS_DECLARE_PROXY_METHOD_2(onDataChannelStateChange, IDataChannelPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onDataChannelError, IDataChannelPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_METHOD_1(onDataChannelBufferedAmountLow, IDataChannelPtr)
ZS_DECLARE_PROXY_METHOD_2(onDataChannelMessage, IDataChannelPtr, MessageEventDataPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IDataChannelDelegate, ortc::IDataChannelSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDataChannelTypes::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ErrorAnyPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDataChannelDelegate::MessageEventDataPtr, MessageEventDataPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onDataChannelStateChange, IDataChannelPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onDataChannelError, IDataChannelPtr, ErrorAnyPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onDataChannelBufferedAmountLow, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onDataChannelMessage, IDataChannelPtr, MessageEventDataPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
