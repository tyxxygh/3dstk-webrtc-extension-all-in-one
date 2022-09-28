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

#include <ortc/services/internal/platform.h>
#include <ortc/services/types.h>

#include <zsLib/IFactory.h>
#include <zsLib/IWakeDelegate.h>

namespace CryptoPP
{
  class AutoSeededRandomPool;
  class ByteQueue;
  class HexEncoder;
  class HexDecoder;
};

namespace ortc
{
  namespace services
  {
    namespace internal
    {
      using std::make_shared;

      using zsLib::string;
      using zsLib::CSTR;
      using zsLib::AutoPUID;
      using zsLib::Noop;
      using zsLib::AutoRecursiveLock;
      using zsLib::CSTR;
      using zsLib::PTRNUMBER;
      using zsLib::Milliseconds;
      using zsLib::Seconds;
      using zsLib::Hours;
      using zsLib::MessageQueueAssociator;
      using zsLib::Subsystem;
      using zsLib::PrivateGlobalLock;
      using zsLib::Singleton;
      using zsLib::SingletonLazySharedPtr;
      using zsLib::SingletonManager;
      using zsLib::ISingletonManagerDelegate;
      using zsLib::ULONGEST;
      using zsLib::PTRNUMBER;

      using zsLib::IFactory;

      using zsLib::XML::WalkSink;

      ZS_DECLARE_USING_PTR(zsLib, ITimer);
      ZS_DECLARE_USING_PTR(zsLib, ITimerDelegate);
      ZS_DECLARE_USING_PTR(zsLib, ISocketDelegate);
      ZS_DECLARE_USING_PTR(zsLib, Socket);
      ZS_DECLARE_USING_PTR(zsLib, Thread);
      ZS_DECLARE_USING_PTR(zsLib, Event);
      ZS_DECLARE_USING_PTR(zsLib, Log);
      ZS_DECLARE_USING_PTR(zsLib, ILogOutputDelegate);
      ZS_DECLARE_USING_PTR(zsLib, IMessageQueueThread);
      ZS_DECLARE_USING_PTR(zsLib, IMessageQueueThreadPool);
      ZS_DECLARE_USING_PTR(zsLib, IMessageQueueManager);
      ZS_DECLARE_USING_PTR(zsLib, ISettings);
      ZS_DECLARE_USING_PTR(zsLib, ISettingsDelegate);
      ZS_DECLARE_USING_PTR(zsLib, ISettingsApplyDefaultsDelegate);
      ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
      ZS_DECLARE_USING_PROXY(zsLib, IWakeDelegate);

      ZS_DECLARE_USING_PTR(zsLib::XML, Attribute);
      ZS_DECLARE_USING_PTR(zsLib::XML, Document);
      ZS_DECLARE_USING_PTR(zsLib::XML, Node);
      ZS_DECLARE_USING_PTR(zsLib::XML, Text);
      ZS_DECLARE_USING_PTR(zsLib::XML, Comment);
      ZS_DECLARE_USING_PTR(zsLib::XML, Unknown);
      ZS_DECLARE_USING_PTR(zsLib::XML, Declaration);
      ZS_DECLARE_USING_PTR(zsLib::XML, Generator);

      using CryptoPP::AutoSeededRandomPool;
      using CryptoPP::HexEncoder;
      using CryptoPP::HexDecoder;

      ZS_DECLARE_TYPEDEF_PTR(CryptoPP::ByteQueue, ByteQueue);

      ZS_DECLARE_CLASS_PTR(Backgrounding);
      ZS_DECLARE_CLASS_PTR(BackOffTimer);
      ZS_DECLARE_CLASS_PTR(BackOffTimerPattern);
      ZS_DECLARE_CLASS_PTR(Cache);
      ZS_DECLARE_CLASS_PTR(DNS);
      ZS_DECLARE_CLASS_PTR(Decryptor);
      ZS_DECLARE_CLASS_PTR(DHKeyDomain);
      ZS_DECLARE_CLASS_PTR(DHPrivateKey);
      ZS_DECLARE_CLASS_PTR(DHPublicKey);
      ZS_DECLARE_CLASS_PTR(DNSMonitor);
      ZS_DECLARE_CLASS_PTR(DNSQuery);
      ZS_DECLARE_CLASS_PTR(Encryptor);
      ZS_DECLARE_CLASS_PTR(ICESocket);
      ZS_DECLARE_CLASS_PTR(ICESocketSession);
      ZS_DECLARE_CLASS_PTR(HTTP);
      ZS_DECLARE_CLASS_PTR(HTTPOverride);
      ZS_DECLARE_CLASS_PTR(MessageLayerSecurityChannel);
      ZS_DECLARE_CLASS_PTR(Reachability);
      ZS_DECLARE_CLASS_PTR(RSAPrivateKey);
      ZS_DECLARE_CLASS_PTR(RSAPublicKey);
      ZS_DECLARE_CLASS_PTR(RUDPChannel);
      ZS_DECLARE_CLASS_PTR(RUDPChannelStream);
      ZS_DECLARE_CLASS_PTR(RUDPICESocket);
      ZS_DECLARE_CLASS_PTR(RUDPTransport);
      ZS_DECLARE_CLASS_PTR(RUDPListener);
      ZS_DECLARE_CLASS_PTR(RUDPMessaging);
      ZS_DECLARE_CLASS_PTR(Settings);
      ZS_DECLARE_CLASS_PTR(STUNDiscovery);
      ZS_DECLARE_CLASS_PTR(STUNRequester);
      ZS_DECLARE_CLASS_PTR(STUNRequesterManager);
      ZS_DECLARE_CLASS_PTR(TCPMessaging);
      ZS_DECLARE_CLASS_PTR(TransportStream);
      ZS_DECLARE_CLASS_PTR(TURNSocket);

      ZS_DECLARE_INTERACTION_PTR(IRUDPChannelStream);

      ZS_DECLARE_INTERACTION_PROXY(IICESocketForICESocketSession);
      ZS_DECLARE_INTERACTION_PROXY(IRUDPChannelDelegateForSessionAndListener);
      ZS_DECLARE_INTERACTION_PROXY(IRUDPChannelStreamDelegate);
      ZS_DECLARE_INTERACTION_PROXY(IRUDPChannelStreamAsync);
      ZS_DECLARE_INTERACTION_PROXY(IRUDPICESocketForRUDPTransport);
    }
  }
}
