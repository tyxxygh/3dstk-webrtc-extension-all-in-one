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


#include <ortc/services/ITCPMessaging.h>
#include <ortc/services/ITransportStream.h>
#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/Exception.h>
#include <zsLib/Socket.h>
#include <zsLib/ITimer.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

namespace ortc { namespace services { namespace test { ZS_DECLARE_SUBSYSTEM(org_ortc_services_test) } } }

using zsLib::AutoPUID;
using zsLib::ULONG;
using zsLib::string;
using zsLib::String;
using zsLib::Time;
using zsLib::AutoRecursiveLock;
using zsLib::RecursiveLock;
using zsLib::IMessageQueue;
using zsLib::ULONG;

using namespace ortc::services;
using namespace ortc::services::test;

namespace ortc
{
  namespace services
  {
    namespace test
    {
      ZS_DECLARE_CLASS_PTR(TestTCPMessagingLoopback)

      class TestTCPMessagingLoopback : public zsLib::MessageQueueAssociator,
                                       public ITCPMessagingDelegate,
                                       public ITransportStreamReaderDelegate,
                                       public ITransportStreamWriterDelegate,
                                       public zsLib::ITimerDelegate,
                                       public ISocketDelegate
      {
      protected:
        struct Message
        {
          DWORD mChannelNumber;
          SecureByteBlockPtr mBuffer;
        };

        typedef std::list<Message> BufferList;

      private:
        //---------------------------------------------------------------------
        TestTCPMessagingLoopback(
                                 zsLib::IMessageQueuePtr queue,
                                 bool hasChannelNumbers
                                 ) :
          zsLib::MessageQueueAssociator(queue),
          mHasChannelNumbers(hasChannelNumbers),
          mServerBuffersReceived(0),
          mClientBuffersReceived(0)
        {
        }

        //---------------------------------------------------------------------
        void init(IPAddress serverIP)
        {
          AutoRecursiveLock lock(mLock);

          mServerReceiveStream = ITransportStream::create(ITransportStreamWriterDelegatePtr(), mThisWeak.lock())->getReader();
          mServerSendStream = ITransportStream::create(mThisWeak.lock(), ITransportStreamReaderDelegatePtr())->getWriter();

          mClientReceiveStream = ITransportStream::create(ITransportStreamWriterDelegatePtr(), mThisWeak.lock())->getReader();
          mClientSendStream = ITransportStream::create(mThisWeak.lock(), ITransportStreamReaderDelegatePtr())->getWriter();
          
          mListenSocket = Socket::createTCP();
          mListenSocket->setOptionFlag(Socket::SetOptionFlag::NonBlocking, true);
          mListenSocket->bind(serverIP);
          mListenSocket->listen();
          mListenSocket->setDelegate(mThisWeak.lock());

          mTimer = zsLib::ITimer::create(mThisWeak.lock(), zsLib::Milliseconds(rand()%400+200));

          mClientMessaging = ITCPMessaging::connect(mThisWeak.lock(), mClientReceiveStream->getStream(), mClientSendStream->getStream(), mHasChannelNumbers, serverIP);
        }

      public:
        //---------------------------------------------------------------------
        static TestTCPMessagingLoopbackPtr create(
                                                   zsLib::IMessageQueuePtr queue,
                                                   IPAddress serverIP,
                                                   bool hasChannelNumbers
                                                   )
        {
          TestTCPMessagingLoopbackPtr pThis(new TestTCPMessagingLoopback(queue, hasChannelNumbers));
          pThis->mThisWeak = pThis;
          pThis->init(serverIP);
          return pThis;
        }

        //---------------------------------------------------------------------
        ~TestTCPMessagingLoopback()
        {
          if (mTimer) {
            mTimer->cancel();
            mTimer.reset();
          }
        }

        //---------------------------------------------------------------------
        virtual void onTCPMessagingStateChanged(
                                                ITCPMessagingPtr messaging,
                                                SessionStates state
                                                )
        {
          AutoRecursiveLock lock(mLock);
          switch (state) {
            case ITCPMessaging::SessionState_Pending:
            {
              break;
            }
            case ITCPMessaging::SessionState_Connected:
            {
              if (messaging == mServerMessaging) {
                mServerConnectTime = zsLib::now();
                mServerReceiveStream->notifyReaderReadyToRead();
              }
              if (messaging == mClientMessaging) {
                mClientConnectTime = zsLib::now();
                mClientReceiveStream->notifyReaderReadyToRead();
              }
              break;
            }
            case ITCPMessaging::SessionState_ShuttingDown:
            {
              break;
            }
            case ITCPMessaging::SessionState_Shutdown:
            {
              if (messaging == mServerMessaging) {
                mServerShutdownTime = zsLib::now();
              }
              if (messaging == mClientMessaging) {
                mClientShutdownTime = zsLib::now();
              }
              break;
            }
            default:  break;
          }
        }

        //---------------------------------------------------------------------
        virtual void onTransportStreamWriterReady(ITransportStreamWriterPtr writer)
        {
          AutoRecursiveLock lock(mLock);
          SecureByteBlockPtr random = IHelper::random(IHelper::random(50, 5000));
          SecureByteBlockPtr send = IHelper::clone(random);

          if (0 == IHelper::random(0, 25)) {
            ZS_LOG_DETAIL("sending zero sized buffer")
            random = SecureByteBlockPtr(new SecureByteBlock(0));
            send = SecureByteBlockPtr(new SecureByteBlock(0));
          }

          Message info;
          info.mBuffer = random;

          if (mHasChannelNumbers) {
            info.mChannelNumber = static_cast<decltype(info.mChannelNumber)>(IHelper::random(0, 100000));
          }

          const char *type = NULL;

          if (writer == mServerSendStream) {
            mClientBuffers.push_back(info);
            type = "server";
          }
          if (writer == mClientSendStream) {
            mServerBuffers.push_back(info);
            type = "client";
          }

          if (!type) return;

          ZS_LOG_DEBUG(log("buffer written") + ZS_PARAM("from", type) + ZS_PARAM("size", send->SizeInBytes()) + ZS_PARAM("writer", ITransportStream::toDebug(writer->getStream())))

          ITCPMessaging::ChannelHeaderPtr header;
          if (mHasChannelNumbers) {
            header = ITCPMessaging::ChannelHeaderPtr(new ITCPMessaging::ChannelHeader);
            header->mChannelID = info.mChannelNumber;
          }
          writer->write(send, header);
        }

        //---------------------------------------------------------------------
        virtual void onTransportStreamReaderReady(ITransportStreamReaderPtr reader)
        {
          AutoRecursiveLock lock(mLock);
          Message info;

          ITransportStream::StreamHeaderPtr header;

          SecureByteBlockPtr buffer = reader->read(&header);
          ITCPMessaging::ChannelHeaderPtr channelHeader = std::dynamic_pointer_cast<ITCPMessaging::ChannelHeader>(header);

          if (!buffer) {
            return;
          }

          const char *type = NULL;

          if (reader == mServerReceiveStream) {
            TESTING_CHECK(mServerBuffers.size() > 0)

            info = mServerBuffers.front();
            mServerBuffers.pop_front();
            type = "server";
            ++mServerBuffersReceived;
          }
          if (reader == mClientReceiveStream) {
            TESTING_CHECK(mClientBuffers.size() > 0)

            info = mClientBuffers.front();
            mClientBuffers.pop_front();
            type = "client";
            ++mClientBuffersReceived;
          }

          if (!type) return;

          if (mHasChannelNumbers) {
            TESTING_CHECK(((bool)header))
            TESTING_CHECK(((bool)channelHeader))

            TESTING_EQUAL(channelHeader->mChannelID, info.mChannelNumber)
          }

          ZS_LOG_DEBUG(log("buffer read") + ZS_PARAM("to", type) + ZS_PARAM("size", buffer->SizeInBytes()) + ZS_PARAM("reader", ITransportStream::toDebug(reader->getStream())))

          TESTING_CHECK(info.mBuffer)
          TESTING_CHECK(buffer)

          TESTING_CHECK(0 == IHelper::compare(*buffer, *info.mBuffer))
        }

        //---------------------------------------------------------------------
        virtual void onTimer(zsLib::ITimerPtr timer)
        {
          AutoRecursiveLock lock(mLock);
          if (timer != mTimer) return;
        }

        //---------------------------------------------------------------------
        virtual void onReadReady(SocketPtr socket)
        {
          AutoRecursiveLock lock(mLock);
          TESTING_CHECK(Time() == mAcceptTime)

          mAcceptTime = zsLib::now();

          mServerMessaging = ITCPMessaging::accept(mThisWeak.lock(), mServerReceiveStream->getStream(), mServerSendStream->getStream(), mHasChannelNumbers, mListenSocket);
        }

        //---------------------------------------------------------------------
        virtual void onWriteReady(SocketPtr socket)
        {
        }

        //---------------------------------------------------------------------
        virtual void onException(SocketPtr socket)
        {
        }

        //---------------------------------------------------------------------
        bool isComplete() const
        {
          AutoRecursiveLock lock(mLock);
          return (Time() != mShutdownTime) &&
                 (Time() != mServerShutdownTime) &&
                 (Time() != mClientShutdownTime);
        }

        //---------------------------------------------------------------------
        bool expectationsOkay() const
        {
          AutoRecursiveLock lock(mLock);
          return (0 != mServerBuffersReceived) &&
                 (0 != mClientBuffersReceived) &&
                 (Time() != mAcceptTime) &&
                 (Time() != mServerConnectTime) &&
                 (Time() != mClientConnectTime) &&
                 (Time() != mServerShutdownTime) &&
                 (Time() != mClientShutdownTime) &&
          true;
        }

        //---------------------------------------------------------------------
        void shutdown()
        {
          AutoRecursiveLock lock(mLock);
          if (Time() != mShutdownTime) return;

          mShutdownTime = zsLib::now();

          mServerMessaging->shutdown();
          mClientMessaging->shutdown();
        }

      protected:
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("TestTCPMessagingLoopback");
          IHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }

      private:
        //---------------------------------------------------------------------
        mutable zsLib::RecursiveLock mLock;
        TestTCPMessagingLoopbackWeakPtr mThisWeak;

        AutoPUID mID;

        bool mHasChannelNumbers;

        zsLib::ITimerPtr mTimer;

        SocketPtr mListenSocket;

        ITCPMessagingPtr mServerMessaging;
        ITCPMessagingPtr mClientMessaging;

        ITransportStreamReaderPtr mServerReceiveStream;
        ITransportStreamWriterPtr mServerSendStream;

        ITransportStreamReaderPtr mClientReceiveStream;
        ITransportStreamWriterPtr mClientSendStream;

        Time mAcceptTime;
        Time mServerConnectTime;
        Time mClientConnectTime;

        Time mShutdownTime;

        Time mServerShutdownTime;
        Time mClientShutdownTime;

        BufferList mServerBuffers;
        BufferList mClientBuffers;

        ULONG mServerBuffersReceived;
        ULONG mClientBuffersReceived;
      };
    }
  }
}

using ortc::services::test::TestTCPMessagingLoopback;
using ortc::services::test::TestTCPMessagingLoopbackPtr;

void doTestTCPMessagingLoopback()
{
  typedef zsLib::ULONG ULONG;

  if (!ORTC_SERVICE_TEST_DO_TCP_MESSAGING_TEST) return;

  TESTING_INSTALL_LOGGER();

  zsLib::IMessageQueueThreadPtr thread(zsLib::IMessageQueueThread::createBasic());

  TestTCPMessagingLoopbackPtr testObject1;
  TestTCPMessagingLoopbackPtr testObject2;
  TestTCPMessagingLoopbackPtr testObject3;
  TestTCPMessagingLoopbackPtr testObject4;

  TESTING_SLEEP(1000)

  ZS_LOG_BASIC("WAITING:      Waiting for TCP messaging testing to complete (max wait is 180 seconds).");

  {
    ULONG step = 0;

    IPAddress ip1("127.0.0.1");
    ip1.setPort(static_cast<zsLib::WORD>(IHelper::random(10000, 29999)));
    IPAddress ip2("127.0.0.1");
    ip2.setPort(static_cast<zsLib::WORD>(IHelper::random(30000, 49999)));

    //    IPAddress ip2("127.0.0.1:6543");

    do
    {
      ZS_LOG_BASIC(String("STEP:         ---------->>>>>>>>>> ") + string(step) + " <<<<<<<<<<----------")

      bool quit = false;
      ULONG expecting = 0;
      switch (step) {
        case 0: {
          testObject1 = TestTCPMessagingLoopback::create(thread, ip1, true);
          break;
        }
        case 1: {
          testObject1 = TestTCPMessagingLoopback::create(thread, ip2, false);
          break;
        }
        default: quit = true; break;
      }

      if (quit) break;

      expecting = 0;
      expecting += (testObject1 ? 1 : 0);
      expecting += (testObject2 ? 1 : 0);
      expecting += (testObject3 ? 1 : 0);
      expecting += (testObject4 ? 1 : 0);

      ULONG found = 0;
      ULONG lastFound = 0;
      ULONG totalWait = 0;

      while (found < expecting)
      {
        TESTING_SLEEP(1000)
        ++totalWait;
        if (totalWait >= 70)
          break;

        found = 0;

        switch (step) {
          case 0: {
            if (10 == totalWait) {
            }

            if (30 == totalWait) {
              if (testObject1) testObject1->shutdown();
              if (testObject2) testObject2->shutdown();
              if (testObject3) testObject3->shutdown();
              if (testObject4) testObject4->shutdown();
            }
            break;
          }
          case 1: {
            if (10 == totalWait) {
            }

            if (30 == totalWait) {
              if (testObject1) testObject1->shutdown();
              if (testObject2) testObject2->shutdown();
              if (testObject3) testObject3->shutdown();
              if (testObject4) testObject4->shutdown();
            }
            break;
          }
        }

        found += (testObject1 ? (testObject1->isComplete() ? 1 : 0) : 0);
        found += (testObject2 ? (testObject2->isComplete() ? 1 : 0) : 0);
        found += (testObject3 ? (testObject3->isComplete() ? 1 : 0) : 0);
        found += (testObject4 ? (testObject4->isComplete() ? 1 : 0) : 0);

        switch (step) {
          case 0: {
            break;
          }
          case 1: {
            break;
          }
        }

        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }
      TESTING_EQUAL(found, expecting);

      switch (step) {
        case 0: {
          if (testObject1) testObject1->expectationsOkay();
          if (testObject2) testObject2->expectationsOkay();
          if (testObject3) testObject3->expectationsOkay();
          if (testObject4) testObject4->expectationsOkay();

          break;
        }
        case 1: {
          if (testObject1) testObject1->expectationsOkay();
          if (testObject2) testObject2->expectationsOkay();
          if (testObject3) testObject3->expectationsOkay();
          if (testObject4) testObject4->expectationsOkay();
          break;
        }
      }
      testObject1.reset();
      testObject2.reset();
      testObject3.reset();
      testObject4.reset();

      ++step;
    } while (true);
  }

  ZS_LOG_BASIC("WAITING:      All TCP messaging have finished. Waiting for 'bogus' events to process (10 second wait).");

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
