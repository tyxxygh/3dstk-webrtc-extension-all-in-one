/*
 
 Copyright (c) 2013, SMB Phone Inc.
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


#include <zsLib/IMessageQueueThread.h>
#include <zsLib/Exception.h>
#include <zsLib/Socket.h>
#include <zsLib/ITimer.h>
#include <ortc/services/IRUDPListener.h>
#include <ortc/services/IRUDPMessaging.h>
#include <ortc/services/IHelper.h>
#include <ortc/services/IDNS.h>
#include <ortc/services/ITransportStream.h>


#include "config.h"
#include "testing.h"

namespace ortc { namespace services { namespace test { ZS_DECLARE_SUBSYSTEM(org_ortc_services_test) } } }

using zsLib::BYTE;
using zsLib::WORD;
using zsLib::ULONG;
using zsLib::CSTR;
using zsLib::Socket;
using zsLib::SocketPtr;
using zsLib::IPAddress;
using zsLib::IMessageQueue;
using zsLib::AutoRecursiveLock;
using ortc::services::IRUDPListener;
using ortc::services::IRUDPListenerPtr;
using ortc::services::IRUDPListenerDelegate;
using ortc::services::IRUDPMessaging;
using ortc::services::IRUDPMessagingPtr;
using ortc::services::IRUDPMessagingDelegate;
using ortc::services::IHelper;
using ortc::services::IDNS;

namespace ortc
{
  namespace services
  {
    namespace test
    {
      ZS_DECLARE_CLASS_PTR(TestRUDPListenerCallback)

      class TestRUDPListenerCallback : public zsLib::MessageQueueAssociator,
                                       public IRUDPListenerDelegate,
                                       public IRUDPMessagingDelegate,
                                       public ITransportStreamWriterDelegate,
                                       public ITransportStreamReaderDelegate
      {
      private:
        //---------------------------------------------------------------------
        TestRUDPListenerCallback(zsLib::IMessageQueuePtr queue) :
          zsLib::MessageQueueAssociator(queue),
          mReceiveStream(ITransportStream::create()->getReader()),
          mSendStream(ITransportStream::create()->getWriter())
        {
        }

        //---------------------------------------------------------------------
        void init(WORD port)
        {
          AutoRecursiveLock lock(mLock);
          mListener = IRUDPListener::create(getAssociatedMessageQueue(), mThisWeak.lock(), port);

          mReceiveStream->notifyReaderReadyToRead();
          mReceiveStreamSubscription = mReceiveStream->subscribe(mThisWeak.lock());
        }

      public:
        //---------------------------------------------------------------------
        static TestRUDPListenerCallbackPtr create(
                                                  zsLib::IMessageQueuePtr queue,
                                                  WORD port
                                                  )
        {
          TestRUDPListenerCallbackPtr pThis(new TestRUDPListenerCallback(queue));
          pThis->mThisWeak = pThis;
          pThis->init(port);
          return pThis;
        }

        //---------------------------------------------------------------------
        ~TestRUDPListenerCallback()
        {
        }

        //---------------------------------------------------------------------
        virtual void onRUDPListenerStateChanged(
                                                IRUDPListenerPtr listener,
                                                RUDPListenerStates state
                                                )
        {
          AutoRecursiveLock lock(mLock);
        }

        //---------------------------------------------------------------------
        virtual void onRUDPListenerChannelWaiting(IRUDPListenerPtr listener)
        {
          zsLib::AutoRecursiveLock lock(mLock);
          mMessaging = IRUDPMessaging::acceptChannel(
                                                     getAssociatedMessageQueue(),
                                                     mListener,
                                                     mThisWeak.lock(),
                                                     mReceiveStream->getStream(),
                                                     mSendStream->getStream()
                                                     );

        }

        //---------------------------------------------------------------------
        virtual void onRUDPMessagingStateChanged(
                                                 IRUDPMessagingPtr session,
                                                 RUDPMessagingStates state
                                                 )
        {
        }

        //---------------------------------------------------------------------
        virtual void onTransportStreamReaderReady(ITransportStreamReaderPtr reader)
        {
          AutoRecursiveLock lock(mLock);
          if (reader != mReceiveStream) return;

          while (true) {
            SecureByteBlockPtr buffer = mReceiveStream->read();
            if (!buffer) return;

            zsLib::String str = (CSTR)(buffer->BytePtr());
            ZS_LOG_BASIC("-------------------------------------------------------------------------------")
            ZS_LOG_BASIC("-------------------------------------------------------------------------------")
            ZS_LOG_BASIC("-------------------------------------------------------------------------------")
            ZS_LOG_BASIC(zsLib::String("RECEIVED: \"") + str + "\"")
            ZS_LOG_BASIC("-------------------------------------------------------------------------------")
            ZS_LOG_BASIC("-------------------------------------------------------------------------------")
            ZS_LOG_BASIC("-------------------------------------------------------------------------------")

            zsLib::String add = "(SERVER->" + IHelper::randomString(10) + ")";

            size_t messageSize = buffer->SizeInBytes() - sizeof(char);

            size_t newMessageSize = messageSize + add.length();
            SecureByteBlockPtr newBuffer(new SecureByteBlock(newMessageSize));

            memcpy(newBuffer->BytePtr(), buffer->BytePtr(), messageSize);
            memcpy(newBuffer->BytePtr() + messageSize, (const zsLib::BYTE *)(add.c_str()), add.length());

            mSendStream->write(newBuffer);
          }
        }

        //---------------------------------------------------------------------
        virtual void onTransportStreamWriterReady(ITransportStreamWriterPtr writer)
        {
          // IGNORED
        }

      private:
        mutable zsLib::RecursiveLock mLock;
        TestRUDPListenerCallbackWeakPtr mThisWeak;

        IRUDPMessagingPtr mMessaging;
        IRUDPListenerPtr mListener;

        ITransportStreamReaderPtr mReceiveStream;
        ITransportStreamWriterPtr mSendStream;

        ITransportStreamReaderSubscriptionPtr mReceiveStreamSubscription;
      };
    }
  }
}

using namespace ortc::services::test;

using ortc::services::test::TestRUDPListenerCallback;
using ortc::services::test::TestRUDPListenerCallbackPtr;

void doTestRUDPListener()
{
  if (!ORTC_SERVICE_TEST_DO_RUDPICESOCKET_CLIENT_TO_SERVER_TEST) return;
  if (!ORTC_SERVICE_TEST_RUNNING_RUDP_REMOTE_SERVER) return;

  TESTING_INSTALL_LOGGER();

  zsLib::IMessageQueueThreadPtr thread(zsLib::IMessageQueueThread::createBasic());

  TestRUDPListenerCallbackPtr testObject1 = TestRUDPListenerCallback::create(thread, ORTC_SERVICE_TEST_RUDP_SERVER_PORT);

  ZS_LOG_BASIC("WAITING:      Waiting for RUDP Listener testing to complete (max wait is 60 minutes).");

  {
    ULONG totalWait = 0;
    do
    {
      TESTING_SLEEP(1000)
      ++totalWait;
      if (totalWait >= (60*60))
        break;
    } while(true);
  }

  testObject1.reset();

  ZS_LOG_BASIC("WAITING:      All RUDP listeners have finished. Waiting for 'bogus' events to process (10 second wait).");

  TESTING_SLEEP(10000)

  // wait for shutdown
  {
    IMessageQueue::size_type count = 0;
    do
    {
      count = thread->getTotalUnprocessedMessages();
      //    count += mThreadNeverCalled->getTotalUnprocessedMessages();
      if (0 != count)
        std::this_thread::yield();
    } while (count > 0);

    thread->waitForShutdown();
  }
  TESTING_UNINSTALL_LOGGER();
  zsLib::proxyDump();
  TESTING_EQUAL(zsLib::proxyGetTotalConstructed(), 0);
}
