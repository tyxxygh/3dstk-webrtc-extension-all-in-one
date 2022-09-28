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
#include <ortc/services/IICESocket.h>
#include <ortc/services/IICESocketSession.h>

#include "config.h"
#include "testing.h"

#include <set>
#include <list>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <cstring>

namespace ortc { namespace services { namespace test { ZS_DECLARE_SUBSYSTEM(org_ortc_services_test) } } }

using zsLib::BYTE;
using zsLib::WORD;
using zsLib::ULONG;
using zsLib::Socket;
using zsLib::SocketPtr;
using zsLib::IPAddress;
using zsLib::String;
using zsLib::IMessageQueue;
using ortc::services::IDNS;
using ortc::services::IDNSQuery;
using ortc::services::ITURNSocket;
using ortc::services::ITURNSocketPtr;
using ortc::services::ITURNSocketDelegate;
using ortc::services::IICESocket;
using ortc::services::IICESocketPtr;
using ortc::services::IICESocketSessionPtr;

namespace ortc
{
  namespace services
  {
    namespace test
    {
      ZS_DECLARE_CLASS_PTR(TestICESocketCallback);

      class TestICESocketCallback : public zsLib::MessageQueueAssociator,
                                    public IICESocketDelegate,
                                    public IICESocketSessionDelegate,
                                    public zsLib::ITimerDelegate
      {
      protected:
        typedef std::list<IICESocketSessionPtr> SessionList;

      private:
        TestICESocketCallback(zsLib::IMessageQueuePtr queue) :
          zsLib::MessageQueueAssociator(queue)
        {
        }

        void init(
                  WORD port,
                  const char *srvNameTURN,
                  const char *srvNameSTUN
                  )
        {
          zsLib::AutoRecursiveLock lock(getLock());

          IICESocket::TURNServerInfoList turnServers;
          IICESocket::STUNServerInfoList stunServers;

          IICESocket::TURNServerInfoPtr turnInfo = IICESocket::TURNServerInfo::create();
          turnInfo->mTURNServer = srvNameTURN;
          turnInfo->mTURNServerUsername = ORTC_SERVICE_TEST_TURN_USERNAME;
          turnInfo->mTURNServerPassword = ORTC_SERVICE_TEST_TURN_PASSWORD;

          IICESocket::STUNServerInfoPtr stunInfo = IICESocket::STUNServerInfo::create();
          stunInfo->mSTUNServer = srvNameSTUN;

          turnServers.push_back(turnInfo);
          stunServers.push_back(stunInfo);

          mICESocket = IICESocket::create(
                                          getAssociatedMessageQueue(),
                                          mThisWeak.lock(),
                                          turnServers,
                                          stunServers,
                                          port
                                          );

          mTimer = zsLib::ITimer::create(mThisWeak.lock(), zsLib::Milliseconds(rand()%400+200));
        }

      public:
        static TestICESocketCallbackPtr create(
                                               zsLib::IMessageQueuePtr queue,
                                               WORD port,
                                               const char *srvNameTURN,
                                               const char *srvNameSTUN,
                                               bool expectConnected = true,
                                               bool expectGracefulShutdown = true,
                                               bool expectErrorShutdown = false,
                                               bool expectedSessionConnected = true,
                                               bool expectedSessionClosed = true
                                               )
        {
          TestICESocketCallbackPtr pThis(new TestICESocketCallback(queue));
          pThis->mThisWeak = pThis;
          pThis->mExpectConnected = expectConnected;
          pThis->mExpectGracefulShutdown = expectGracefulShutdown;
          pThis->mExpectErrorShutdown = expectErrorShutdown;
          pThis->mExpectedSessionConnected = expectedSessionConnected;
          pThis->mExpectedSessionClosed = expectedSessionClosed;
          pThis->init(port, srvNameTURN, srvNameSTUN);
          return pThis;
        }

        ~TestICESocketCallback()
        {
          if (mTimer) {
            mTimer->cancel();
            mTimer.reset();
          }
          mSessions.clear();
          mICESocket.reset();
        }

        virtual void onICESocketStateChanged(
                                             IICESocketPtr socket,
                                             ICESocketStates state
                                             )
        {
          zsLib::AutoRecursiveLock lock(getLock());
          switch (state) {
            case IICESocket::ICESocketState_Ready:
            {
              TESTING_CHECK(mExpectConnected);
              mConnected = true;

              IICESocket::CandidateList candidates;
              socket->getLocalCandidates(candidates);

              TestICESocketCallbackPtr remote = mRemote.lock();

              if (remote) {
                remote->updateCandidates(candidates);  // give final list of candidates
                remote->notifyEndOfCandidates();
              }
              break;
            }
            case IICESocket::ICESocketState_Shutdown:
            {
              if (mShutdownCalled) {
                TESTING_CHECK(mExpectGracefulShutdown);
                mGracefulShutdown = true;
              } else {
                TESTING_CHECK(mExpectErrorShutdown);
                mErrorShutdown = true;
              }
              mICESocket.reset();
              break;
            }
            default:  break;
          }
        }

        virtual void onICESocketCandidatesChanged(IICESocketPtr socket)
        {
          zsLib::AutoRecursiveLock lock(getLock());

          TestICESocketCallbackPtr remote = mRemote.lock();
          if (!remote) return;

          if (!mICESocket) return;

          IICESocket::CandidateList candidates;
          socket->getLocalCandidates(candidates);

          remote->updateCandidates(candidates);
        }

        virtual void onICESocketSessionStateChanged(
                                                    IICESocketSessionPtr session,
                                                    ICESocketSessionStates state
                                                    )
        {
          zsLib::AutoRecursiveLock lock(getLock());

          switch(state) {
            case IICESocketSession::ICESocketSessionState_Nominated:
            case IICESocketSession::ICESocketSessionState_Completed:
            {
              TESTING_CHECK(mExpectedSessionConnected);
              mSessionConnected = true;

              SessionList::iterator found = find(mSessions.begin(), mSessions.end(), session);
              TESTING_CHECK(found != mSessions.end())
              break;
            }
            case IICESocketSession::ICESocketSessionState_Shutdown:
            {
              TESTING_CHECK(mExpectedSessionClosed);
              mSessionClosed = true;

              SessionList::iterator found = find(mSessions.begin(), mSessions.end(), session);
              TESTING_CHECK(found != mSessions.end())
              mSessions.erase(found);
            }
            default: break;
          }
        }

        virtual void handleICESocketSessionReceivedPacket(
                                                          IICESocketSessionPtr session,
                                                          const zsLib::BYTE *buffer,
                                                          size_t bufferLengthInBytes
                                                          )
        {
          zsLib::AutoRecursiveLock lock(getLock());
        }

        virtual bool handleICESocketSessionReceivedSTUNPacket(
                                                              IICESocketSessionPtr session,
                                                              STUNPacketPtr stun,
                                                              const zsLib::String &localUsernameFrag,
                                                              const zsLib::String &remoteUsernameFrag
                                                              )
        {
          zsLib::AutoRecursiveLock lock(getLock());
          return false;
        }

        virtual void onICESocketSessionWriteReady(IICESocketSessionPtr session)
        {
          zsLib::AutoRecursiveLock lock(getLock());
        }

        virtual void onICESocketSessionNominationChanged(IICESocketSessionPtr session)
        {
          zsLib::AutoRecursiveLock lock(getLock());
        }

        virtual void onTimer(zsLib::ITimerPtr timer)
        {
          zsLib::AutoRecursiveLock lock(getLock());
          if (timer != mTimer) return;
        }

        void shutdown()
        {
          zsLib::AutoRecursiveLock lock(getLock());

          mRemote.reset();

          if (!mICESocket) return;
          if (mShutdownCalled) return;
          mShutdownCalled = true;
          for (SessionList::iterator iter = mSessions.begin(); iter != mSessions.end(); ++iter) {
            IICESocketSessionPtr &session = (*iter);
            session->close();
          }
          mICESocket->shutdown();
          if (mTimer) {
            mTimer->cancel();
            mTimer.reset();
          }
        }

        bool isComplete()
        {
          return (mExpectConnected ? true : (mExpectConnected == mConnected)) &&
                 (mExpectGracefulShutdown == mGracefulShutdown) &&
                 (mExpectErrorShutdown == mErrorShutdown) &&
                 (mExpectedSessionConnected == mSessionConnected) &&
                 (mExpectedSessionClosed == mSessionClosed);
        }

        void expectationsOkay() {
          zsLib::AutoRecursiveLock lock(getLock());
          if (mExpectConnected) {
//            TESTING_CHECK(mConnected); // invalid assumption for connected as a non routable IP will not be capable of allocating TURN
          } else {
            TESTING_CHECK(!mConnected);
          }

          if (mExpectGracefulShutdown) {
            TESTING_CHECK(mGracefulShutdown);
          } else {
            TESTING_CHECK(!mGracefulShutdown);
          }

          if (mExpectErrorShutdown) {
            TESTING_CHECK(mErrorShutdown);
          } else {
            TESTING_CHECK(!mErrorShutdown);
          }

          if (mExpectedSessionConnected) {
            TESTING_CHECK(mSessionConnected);
          } else {
            TESTING_CHECK(!mSessionConnected);
          }

          if (mExpectedSessionClosed) {
            TESTING_CHECK(mSessionClosed);
          } else {
            TESTING_CHECK(!mSessionClosed);
          }
        }

        void getLocalCandidates(IICESocket::CandidateList &outCandidates)
        {
          zsLib::AutoRecursiveLock lock(getLock());
          if (!mICESocket) return;
          mICESocket->getLocalCandidates(outCandidates);
        }

        String getLocalUsernameFrag()
        {
          zsLib::AutoRecursiveLock lock(getLock());
          if (!mICESocket) return String();
          return mICESocket->getUsernameFrag();
        }

        String getLocalPassword()
        {
          zsLib::AutoRecursiveLock lock(getLock());
          if (!mICESocket) return String();
          return mICESocket->getPassword();
        }

        IICESocketSessionPtr createSessionFromRemoteCandidates(IICESocket::ICEControls control)
        {
          zsLib::AutoRecursiveLock lock(getLock());
          if (!mICESocket) return IICESocketSessionPtr();

          TestICESocketCallbackPtr remote = mRemote.lock();
          if (!remote) return IICESocketSessionPtr();

          String remoteUsernameFrag = remote->getLocalUsernameFrag();
          String remotePassword = remote->getLocalPassword();
          IICESocket::CandidateList remoteCandidates;
          remote->getLocalCandidates(remoteCandidates);

          IICESocketSessionPtr session = IICESocketSession::create(mThisWeak.lock(), mICESocket, remoteUsernameFrag, remotePassword, remoteCandidates, control);
          mSessions.push_back(session);

          return session;
        }

        void setRemote(TestICESocketCallbackPtr remote)
        {
          zsLib::AutoRecursiveLock lock(getLock());
          mRemote = remote;
        }

        void updateCandidates(const IICESocket::CandidateList &candidates)
        {
          zsLib::AutoRecursiveLock lock(getLock());
          for (SessionList::iterator iter = mSessions.begin(); iter != mSessions.end(); ++iter)
          {
            IICESocketSessionPtr session = (*iter);
            session->updateRemoteCandidates(candidates);
          }
        }

        void notifyEndOfCandidates()
        {
          zsLib::AutoRecursiveLock lock(getLock());
          for (SessionList::iterator iter = mSessions.begin(); iter != mSessions.end(); ++iter)
          {
            IICESocketSessionPtr session = (*iter);
            session->endOfRemoteCandidates();
          }
        }

        RecursiveLock &getLock() const
        {
          static RecursiveLock lock;
          return lock;
        }

      private:
        TestICESocketCallbackWeakPtr mThisWeak;

        TestICESocketCallbackWeakPtr mRemote;

        zsLib::ITimerPtr mTimer;

        IICESocketPtr mICESocket;
        SessionList mSessions;

        bool mExpectConnected {};
        bool mExpectGracefulShutdown {};
        bool mExpectErrorShutdown {};
        bool mExpectedSessionConnected {};
        bool mExpectedSessionClosed {};

        bool mConnected {};
        bool mGracefulShutdown {};
        bool mErrorShutdown {};
        bool mSessionConnected {};
        bool mSessionClosed {};

        bool mShutdownCalled {};
      };
    }
  }
}

using ortc::services::test::TestICESocketCallback;
using ortc::services::test::TestICESocketCallbackPtr;

void doTestICESocket()
{
  if (!ORTC_SERVICE_TEST_DO_ICE_SOCKET_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  zsLib::IMessageQueueThreadPtr thread(zsLib::IMessageQueueThread::createBasic());

  TestICESocketCallbackPtr testObject1;
  TestICESocketCallbackPtr testObject2;
  TestICESocketCallbackPtr testObject3;
  TestICESocketCallbackPtr testObject4;

  TESTING_STDOUT() << "WAITING:      Waiting for ICE testing to complete (max wait is 180 seconds).\n";

  WORD port1 = 0;
  WORD port2 = 0;
  WORD port3 = 0;
  WORD port4 = 0;

  while (true)
  {
    port1 = static_cast<decltype(port1)>(5000 + (rand() % (65525 - 5000)));
    port2 = static_cast<decltype(port2)>(5000 + (rand() % (65525 - 5000)));
    port3 = static_cast<decltype(port3)>(5000 + (rand() % (65525 - 5000)));
    port4 = static_cast<decltype(port4)>(5000 + (rand() % (65525 - 5000)));

    std::set<decltype(port1)> checkUnique;
    checkUnique.insert(port1);
    checkUnique.insert(port2);
    checkUnique.insert(port3);
    checkUnique.insert(port4);

    if (checkUnique.size() == 4) break;

    TESTING_STDOUT() << "WARNING:      Port conflict detected. Picking new port numbers.\n";
  }

  // check to see if all DNS routines have resolved
  {
    ULONG step = 0;

    do
    {
      TESTING_STDOUT() << "STEP:         ---------->>>>>>>>>> " << step << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;
      switch (step) {
        case 0: {
          testObject1 = TestICESocketCallback::create(thread, port1, ORTC_SERVICE_TEST_TURN_SERVER_DOMAIN, ORTC_SERVICE_TEST_STUN_SERVER_HOST);
          testObject2 = TestICESocketCallback::create(thread, port2, ORTC_SERVICE_TEST_TURN_SERVER_DOMAIN, ORTC_SERVICE_TEST_STUN_SERVER_HOST);

          testObject1->setRemote(testObject2);
          testObject2->setRemote(testObject1);
          break;
        }
        case 1: {
          testObject1 = TestICESocketCallback::create(thread, port3, ORTC_SERVICE_TEST_TURN_SERVER_DOMAIN, ORTC_SERVICE_TEST_STUN_SERVER_HOST, true, false, false, true, false);
          testObject2 = TestICESocketCallback::create(thread, port4, ORTC_SERVICE_TEST_TURN_SERVER_DOMAIN, ORTC_SERVICE_TEST_STUN_SERVER_HOST, true, false, false, true, false);

          testObject1->setRemote(testObject2);
          testObject2->setRemote(testObject1);
          break;
        }
        default:  quit = true; break;
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
        if (totalWait >= 40)
          break;

        found = 0;

        switch (step) {
          case 0: {
            if (1 == totalWait) {
              testObject1->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlling);
              testObject2->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlled);
            }

            if (20 == totalWait) {
              testObject1->shutdown();
              testObject2->shutdown();
            }
            break;
          }
          case 1: {
            if (10 == totalWait) {
              testObject1->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlling);
              testObject2->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlling);
            }

            if (39 == totalWait) {
              found = 2;
              testObject1->shutdown();
              testObject2->shutdown();
            }

            break;
          }
        }

        if (0 == found) {
          found += (testObject1 ? (testObject1->isComplete() ? 1 : 0) : 0);
          found += (testObject2 ? (testObject2->isComplete() ? 1 : 0) : 0);
          found += (testObject3 ? (testObject3->isComplete() ? 1 : 0) : 0);
          found += (testObject4 ? (testObject4->isComplete() ? 1 : 0) : 0);
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

          break;
        }
        case 1: {
          if (testObject1) testObject1->expectationsOkay();
          if (testObject2) testObject2->expectationsOkay();
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

  TESTING_STDOUT() << "WAITING:      All ICE sockets have finished. Waiting for 'bogus' events to process (20 second wait).\n";
  TESTING_SLEEP(20000)

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
