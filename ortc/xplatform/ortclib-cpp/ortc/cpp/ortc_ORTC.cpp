/*

 Copyright (c) 2014, Hookflash Inc.
 Copyright (c) 2017, Optical Tone Ltd.
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

#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/ortc.events.jman.h>
#include <ortc/internal/ortc.stats.events.h>
#include <ortc/internal/ortc.stats.events.jman.h>
#include <ortc/internal/ortc_MediaEngine.h>

#include <ortc/services/IHelper.h>
#include <ortc/services/ILogger.h>

#include <zsLib/IMessageQueueManager.h>
#include <zsLib/ISettings.h>
#include <zsLib/Log.h>
#include <zsLib/Socket.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::ILogger, UseServicesLogger);
  ZS_DECLARE_USING_PTR(zsLib, ISettings);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(ORTCSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(zsLib::IMessageQueueManager, UseMessageQueueManager);

    void initSubsystems();
    void installORTCSettingsDefaults();
    void installCertificateSettingsDefaults();
    void installDataChannelSettingsDefaults();
    void installDTMFSenderSettingsDefaults();
    void installDTLSTransportSettingsDefaults();
    void installICEGathererSettingsDefaults();
    void installICETransportSettingsDefaults();
    void installIdentitySettingsDefaults();
    void installMediaDevicesSettingsDefaults();
    void installMediaStreamTrackSettingsDefaults();
    void installRTPListenerSettingsDefaults();
    void installMediaChannelTraceHelperDefaults();
    void installMediaDeviceCaptureAudioSettingsDefaults();
    void installMediaDeviceCaptureVideoSettingsDefaults();
    void installRTPDecoderAudioSettingsDefaults();
    void installRTPDecoderVideoSettingsDefaults();
    void installRTPEncoderAudioSettingsDefaults();
    void installRTPEncoderVideoSettingsDefaults();
    void installMediaEngineSettingsDefaults();
    void installRTPReceiverSettingsDefaults();
    void installRTPReceiverChannelSettingsDefaults();
    void installRTPSenderSettingsDefaults();
    void installRTPSenderChannelSettingsDefaults();
    void installStatsReportSettingsDefaults();
    void installSCTPTransportSettingsDefaults();
    void installSCTPTransportListenerSettingsDefaults();
    void installSRTPTransportSettingsDefaults();
    void installSRTPSDESTransportSettingsDefaults();

    //-------------------------------------------------------------------------
    static void installAllDefaults()
    {
      installORTCSettingsDefaults();
      installCertificateSettingsDefaults();
      installDataChannelSettingsDefaults();
      installDTMFSenderSettingsDefaults();
      installDTLSTransportSettingsDefaults();
      installICEGathererSettingsDefaults();
      installICETransportSettingsDefaults();
      installIdentitySettingsDefaults();
      installMediaDevicesSettingsDefaults();
      installMediaChannelTraceHelperDefaults();
      installMediaStreamTrackSettingsDefaults();
      installRTPListenerSettingsDefaults();
      installMediaDeviceCaptureAudioSettingsDefaults();
      installMediaDeviceCaptureVideoSettingsDefaults();
      installRTPDecoderAudioSettingsDefaults();
      installRTPDecoderVideoSettingsDefaults();
      installRTPEncoderAudioSettingsDefaults();
      installRTPEncoderVideoSettingsDefaults();
      installMediaEngineSettingsDefaults();
      installRTPReceiverSettingsDefaults();
      installRTPReceiverChannelSettingsDefaults();
      installRTPSenderSettingsDefaults();
      installRTPSenderChannelSettingsDefaults();
      installStatsReportSettingsDefaults();
      installSCTPTransportSettingsDefaults();
      installSCTPTransportListenerSettingsDefaults();
      installSRTPTransportSettingsDefaults();
      installSRTPSDESTransportSettingsDefaults();
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTCSettingsDefaults
    #pragma mark

    class ORTCSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~ORTCSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static ORTCSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<ORTCSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static ORTCSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<ORTCSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_MAIN_THREAD_NAME, zsLib::toString(zsLib::ThreadPriority_NormalPriority));
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_PIPELINE_THREAD_NAME, zsLib::toString(zsLib::ThreadPriority_HighPriority));
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME, "normal");
        ISettings::setString(ORTC_SETTING_ORTC_QUEUE_CERTIFICATE_GENERATION_NAME, "low");

        for (size_t index = 0; index < ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS; ++index) {
          ISettings::setString((String(ORTC_SETTING_ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str(), "higest");
        }
        for (size_t index = 0; index < ORTC_QUEUE_TOTAL_RTP_THREADS; ++index) {
          ISettings::setString((String(ORTC_SETTING_ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str(), "higest");
        }
        ISettings::setString(ZSLIB_SETTING_SOCKET_MONITOR_THREAD_PRIORITY, zsLib::toString(zsLib::ThreadPriority_HighPriority));
      }
      
    };

    //-------------------------------------------------------------------------
    void installORTCSettingsDefaults()
    {
      ORTCSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    void IORTCForInternal::overrideQueueDelegate(IMessageQueuePtr queue)
    {
      return (ORTC::singleton())->overrideQueueDelegate(queue);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueDelegate()
    {
      return (ORTC::singleton())->queueDelegate();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueORTC()
    {
      return (ORTC::singleton())->queueORTC();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueORTCPipeline()
    {
      return (ORTC::singleton())->queueORTCPipeline();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueBlockingMediaStartStopThread()
    {
      return (ORTC::singleton())->queueBlockingMediaStartStopThread();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueMediaDevices()
    {
      return (ORTC::singleton())->queueMediaDevices();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueRTP()
    {
      return (ORTC::singleton())->queueRTP();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueCertificateGeneration()
    {
      return (ORTC::singleton())->queueCertificateGeneration();
    }

    //-------------------------------------------------------------------------
    Optional<Log::Level> IORTCForInternal::webrtcLogLevel()
    {
      return (ORTC::singleton())->webrtcLogLevel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTC::ORTC(const make_private &) :
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_REGISTER(OrtcLib);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
      ZS_EVENTING_REGISTER(OrtcLibStatsReport);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_0(x, i, Detail, OrtcCreate, ol, Ortc, Start);
      ZS_EVENTING_EXCLUSIVE(x);

      initSubsystems();
      ZS_LOG_DETAIL(log("created"));
    }

    //-------------------------------------------------------------------------
    ORTC::~ORTC()
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"));

      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_0(x, i, Detail, OrtcDestroy, ol, Ortc, Stop);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
      ZS_EVENTING_1(x, i, Detail, OrtcStatsReportCommand, ols, Stats, Start, string, command_name, "stop");
      ZS_EVENTING_ASSIGN_VALUE(OrtcStatsReportCommand, 106);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLib);
      ZS_EVENTING_UNREGISTER(OrtcLib);
      ZS_EVENTING_EXCLUSIVE(x);

      ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
      ZS_EVENTING_UNREGISTER(OrtcLibStatsReport);
      ZS_EVENTING_EXCLUSIVE(x);
    }

    //-------------------------------------------------------------------------
    void ORTC::init()
    {
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::convert(IORTCPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ORTC, object);
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::create()
    {
      ORTCPtr pThis(make_shared<ORTC>(make_private{}));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTC => IORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::singleton()
    {
      AutoRecursiveLock lock(*UseServicesHelper::getGlobalLock());
      static SingletonLazySharedPtr<ORTC> singleton(ORTC::create());
      ORTCPtr result = singleton.singleton();
      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"));
      }
      return result;
    }

    //-------------------------------------------------------------------------
    void ORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue)
    {
      {
        AutoRecursiveLock lock(mLock);

        if (defaultDelegateMessageQueue) {
          mDelegateQueue = defaultDelegateMessageQueue;
        }
      }

      UseServicesHelper::setup();
      internalSetup();
    }

#ifdef WINUWP
    //-------------------------------------------------------------------------
    void ORTC::setup(Windows::UI::Core::CoreDispatcher ^dispatcher)
    {
      UseServicesHelper::setup(dispatcher);
      internalSetup();
    }
#endif //WINUWP

    //-------------------------------------------------------------------------
    Milliseconds ORTC::ntpServerTime() const
    {
      AutoRecursiveLock(*this);
      return mNTPServerTime;
    }

    //-------------------------------------------------------------------------
    void ORTC::ntpServerTime(const Milliseconds &value)
    {
      {
        AutoRecursiveLock(*this);
        mNTPServerTime = value;
      }
      IMediaEngineForORTC::ntpServerTime(value);
    }

    //-------------------------------------------------------------------------
    void ORTC::defaultWebrtcLogLevel(Log::Level level)
    {
      AutoRecursiveLock(*this);
      mDefaultWebRTCLogLevel = level;
    }

    //-------------------------------------------------------------------------
    void ORTC::webrtcLogLevel(Log::Level level)
    {
      AutoRecursiveLock(*this);
      mWebRTCLogLevel = level;
    }

    //-------------------------------------------------------------------------
    void ORTC::startMediaTracing()
    {
      IMediaEngineForORTC::startMediaTracing();
    }

    //-------------------------------------------------------------------------
    void ORTC::stopMediaTracing()
    {
      IMediaEngineForORTC::stopMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool ORTC::isMediaTracing()
    {
      return IMediaEngineForORTC::isMediaTracing();
    }

    //-------------------------------------------------------------------------
    bool ORTC::saveMediaTrace(String filename)
    {
      return IMediaEngineForORTC::saveMediaTrace(filename);
    }

    //-------------------------------------------------------------------------
    bool ORTC::saveMediaTrace(String host, int port)
    {
      return IMediaEngineForORTC::saveMediaTrace(host, port);
    }

    //-------------------------------------------------------------------------
    bool ORTC::isMRPInstalled()
    {
#define TODO_NEED_THIS_MRP_INSTALLED_API_IMPLEMENTED 1
#define TODO_NEED_THIS_MRP_INSTALLED_API_IMPLEMENTED 2
//      return IRTPMediaEngineForORTC::isMRPInstalled();
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    void ORTC::overrideQueueDelegate(IMessageQueuePtr queue)
    {
      AutoRecursiveLock lock(*this);
      mDelegateQueue = queue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueDelegate() const
    {
      AutoRecursiveLock lock(*this);
      if (!mDelegateQueue) {
        mDelegateQueue = UseMessageQueueManager::getMessageQueueForGUIThread();
      }
      return mDelegateQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTC() const
    {
      AutoRecursiveLock lock(*this);
      class Once {
      public:
        Once() {
          zsLib::IMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_MAIN_THREAD_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_MAIN_THREAD_NAME)));
        }
      };
      static Once once;
      return UseMessageQueueManager::getThreadPoolQueue(ORTC_QUEUE_MAIN_THREAD_NAME);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTCPipeline() const
    {
      AutoRecursiveLock lock(*this);
      class Once {
      public:
        Once() {
          zsLib::IMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_PIPELINE_THREAD_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_PIPELINE_THREAD_NAME)));
        }
      };
      static Once once;
      return UseMessageQueueManager::getThreadPoolQueue(ORTC_QUEUE_PIPELINE_THREAD_NAME);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueBlockingMediaStartStopThread() const
    {
      AutoRecursiveLock lock(*this);
      if (!mBlockingMediaStartStopThread) {
        mBlockingMediaStartStopThread = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME);
      }
      return mBlockingMediaStartStopThread;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueMediaDevices() const
    {
      AutoRecursiveLock lock(*this);

      size_t index = mNextMediaQueueThread % ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS;

      if (!mMediaDeviceQueues[index]) {
        mMediaDeviceQueues[index] = UseMessageQueueManager::getMessageQueue((String(ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str());
      }

      ++mNextMediaQueueThread;
      return mMediaDeviceQueues[index];
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueRTP() const
    {
      AutoRecursiveLock lock(*this);

      size_t index = mNextRTPQueueThread % ORTC_QUEUE_TOTAL_RTP_THREADS;

      if (!mRTPQueues[index]) {
        mRTPQueues[index] = UseMessageQueueManager::getMessageQueue((String(ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str());
      }

      ++mNextRTPQueueThread;
      return mRTPQueues[index];
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueCertificateGeneration() const
    {
      AutoRecursiveLock lock(*this);
      if (!mCertificateGeneration) {
        mCertificateGeneration = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_CERTIFICATE_GENERATION_NAME);
      }
      return mCertificateGeneration;
    }

    //-------------------------------------------------------------------------
    Optional<Log::Level> ORTC::webrtcLogLevel() const
    {
      AutoRecursiveLock lock(*this);
      if (mWebRTCLogLevel.hasValue()) return mWebRTCLogLevel;
      return mDefaultWebRTCLogLevel;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ORTC::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ORTC::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    void ORTC::internalSetup()
    {
      installAllDefaults();
      ISettings::applyDefaults();

      UseMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME)));
      UseMessageQueueManager::registerMessageQueueThreadPriority(ORTC_QUEUE_CERTIFICATE_GENERATION_NAME, zsLib::threadPriorityFromString(ISettings::getString(ORTC_SETTING_ORTC_QUEUE_CERTIFICATE_GENERATION_NAME)));

      for (size_t index = 0; index < ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS; ++index) {
        UseMessageQueueManager::registerMessageQueueThreadPriority(
          (String(ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str(),
          zsLib::threadPriorityFromString(ISettings::getString((String(ORTC_SETTING_ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME) + string(index)).c_str()))
        );
      }
      for (size_t index = 0; index < ORTC_QUEUE_TOTAL_RTP_THREADS; ++index) {
        UseMessageQueueManager::registerMessageQueueThreadPriority(
          (String(ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str(),
          zsLib::threadPriorityFromString(ISettings::getString((String(ORTC_SETTING_ORTC_QUEUE_RTP_THREAD_NAME) + string(index)).c_str()))
        );
      }
    }

  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IORTC
  #pragma mark

  //---------------------------------------------------------------------------
  void IORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(defaultDelegateMessageQueue);
  }

  //---------------------------------------------------------------------------
#ifdef WINUWP
  void IORTC::setup(Windows::UI::Core::CoreDispatcher ^dispatcher)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(dispatcher);
  }
#endif //WINUWP

  //-------------------------------------------------------------------------
  Milliseconds IORTC::ntpServerTime()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return Milliseconds();
    return singleton->ntpServerTime();
  }

  //-------------------------------------------------------------------------
  void IORTC::ntpServerTime(const Milliseconds &value)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->ntpServerTime(value);
  }

  //-------------------------------------------------------------------------
  void IORTC::setDefaultLogLevel(Log::Level level)
  {
    auto singleton = internal::ORTC::singleton();
    if (singleton) {
      singleton->defaultWebrtcLogLevel(level);
    }

    UseServicesLogger::setLogLevel(level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setLogLevel(const char *componenet, Log::Level level)
  {
    String str(componenet);

    if (0 == str.compare("ortclib_webrtc")) {
      auto singleton = internal::ORTC::singleton();
      if (singleton) {
        singleton->webrtcLogLevel(level);
      }
    }

    UseServicesLogger::setLogLevel(componenet, level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setDefaultEventingLevel(Log::Level level)
  {
    UseServicesLogger::setEventingLevel(level);
  }

  //-------------------------------------------------------------------------
  void IORTC::setEventingLevel(const char *componenet, Log::Level level)
  {
    UseServicesLogger::setEventingLevel(componenet, level);
  }

  //-------------------------------------------------------------------------
  void IORTC::startMediaTracing()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->startMediaTracing();
  }

  //-------------------------------------------------------------------------
  void IORTC::stopMediaTracing()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->stopMediaTracing();
  }

  //-------------------------------------------------------------------------
  bool IORTC::isMediaTracing()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->isMediaTracing();
  }

  //-------------------------------------------------------------------------
  bool IORTC::saveMediaTrace(String filename)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->saveMediaTrace(filename);
  }

  //-------------------------------------------------------------------------
  bool IORTC::saveMediaTrace(String host, int port)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->saveMediaTrace(host, port);
  }

  //-------------------------------------------------------------------------
  bool IORTC::isMRPInstalled()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return false;
    return singleton->isMRPInstalled();
  }

}
