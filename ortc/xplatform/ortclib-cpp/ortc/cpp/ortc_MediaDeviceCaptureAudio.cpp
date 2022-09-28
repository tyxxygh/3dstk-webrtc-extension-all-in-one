/*

 Copyright (c) 2017, Hookflash Inc. / Optical Tone Ltd.
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

#include <ortc/internal/ortc_MediaDeviceCaptureAudio.h>
#include <ortc/internal/ortc_MediaEngine.h>
//#include <ortc/internal/ortc_MediaStreamTrack.h>
//#include <ortc/internal/ortc_MediaDeviceCaptureAudio.h>
//#include <ortc/internal/ortc_MediaDeviceCaptureVideo.h>
//#include <ortc/internal/ortc_MediaDeviceRenderAudio.h>
////#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
////#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
////#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
////#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
//#include <ortc/internal/ortc_RTPPacket.h>
//#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#include <ortc/IStatsReport.h>
//
//#include <ortc/IHelper.h>
#include <ortc/services/IHTTP.h>
//
#include <zsLib/ISettings.h>
//#include <zsLib/Singleton.h>
//#include <zsLib/Log.h>
//#include <zsLib/XML.h>
//#include <zsLib/SafeInt.h>
//
//#include <cryptopp/sha.h>
//
//#include <webrtc/base/event_tracer.h>

#if 0
#include <limits>
#include <float.h>
#include <math.h>

#include <webrtc/rtc_base/timeutils.h>
#include <webrtc/call/rtc_event_log.h>
#include <webrtc/voice_engine/include/voe_codec.h>
#include <webrtc/voice_engine/include/voe_rtp_rtcp.h>
#include <webrtc/voice_engine/include/voe_network.h>
#include <webrtc/voice_engine/include/voe_hardware.h>
#include <webrtc/system_wrappers/include/cpu_info.h>
#include <webrtc/voice_engine/include/voe_audio_processing.h>
#include <webrtc/modules/video_capture/video_capture_factory.h>
#include <webrtc/modules/audio_coding/codecs/builtin_audio_decoder_factory.h>
#ifdef WINRT
#include <third_party/h264_winrt/h264_winrt_factory.h>
#endif
#endif //0

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_engine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaDeviceCaptureAudioSettingsDefaults);
    ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, UseStatsReport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudioSettingsDefaults
    #pragma mark

    class MediaDeviceCaptureAudioSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaDeviceCaptureAudioSettingsDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaDeviceCaptureAudioSettingsDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<MediaDeviceCaptureAudioSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaDeviceCaptureAudioSettingsDefaultsPtr create()
      {
        auto pThis(make_shared<MediaDeviceCaptureAudioSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
      }
      
    };

    //-------------------------------------------------------------------------
    void installMediaDeviceCaptureAudioSettingsDefaults()
    {
      MediaDeviceCaptureAudioSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceCaptureAudioForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaDeviceCaptureAudioForMediaEngine::ForMediaEnginePtr IMediaDeviceCaptureAudioForMediaEngine::create(
                                                                                                             UseMediaEnginePtr mediaEngine,
                                                                                                             const String &deviceID
                                                                                                             )
    {
      return internal::IMediaDeviceCaptureAudioFactory::singleton().create(mediaEngine, deviceID);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::MediaDeviceCaptureAudio(
                                                     const make_private &,
                                                     IMessageQueuePtr queue,
                                                     UseMediaEnginePtr mediaEngine,
                                                     const String &deviceID
                                                     ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mediaEngine_(mediaEngine),
      deviceID_(deviceID),
      subscribers_(make_shared<MediaSubscriberMap>())
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureAudioCreate, ol, MediaEngine, Start, puid, id, id_, string, deviceID, deviceID_);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::~MediaDeviceCaptureAudio()
    {
      if (isNoop()) return;

      thisWeak_.reset();

      cancel();
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioDestroy, ol, MediaEngine, Stop, puid, id, id_);
    }

    //-----------------------------------------------------------------------
    MediaDeviceCaptureAudioPtr MediaDeviceCaptureAudio::create(
                                                               UseMediaEnginePtr mediaEngine,
                                                               const String &deviceID
                                                               )
    {
      auto pThis(make_shared<MediaDeviceCaptureAudio>(make_private{}, IORTCForInternal::queueMediaDevices(), mediaEngine, deviceID));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudioPtr MediaDeviceCaptureAudio::convert(ForMediaEnginePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(MediaDeviceCaptureAudio, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => (for Media)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::notifyMediaStateChanged()
    {
      ZS_EVENTING_1(x, i, Debug, MediaDeviceCaptureAudioNotifyMediaStateChanged, ol, MediaEngine, Event, puid, id, id_);
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::notifyMediaFailure(
                                                     MediaPtr media,
                                                     WORD errorCode,
                                                     const char *inReason
                                                     )
    {
      ZS_EVENTING_2(x, e, Debug, MediaDeviceCaptureAudioNotifyMediaFailure, ol, MediaEngine, Event, puid, id, id_, puid, mediaId, media->getID());

      auto pThis = thisWeak_.lock();
      String reason(inReason);
      postClosure([pThis, media, errorCode, reason] {
        AutoRecursiveLock lock(*pThis);

        if (media != pThis->media_) return;

        pThis->setError(errorCode, reason);
        pThis->cancel();
      });
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => (for MediaSubscribers)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::notifySusbcriberGone()
    {
      ZS_EVENTING_1(x, i, Debug, MediaDeviceCaptureAudioNotifySubscriberGone, ol, MediaEngine, Event, puid, id, id_);

      recheckMode_ = true;
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => IMediaDeviceForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::isDeviceIdle()
    {
      AutoRecursiveLock lock(*this);

      if (isShutdown()) return true;

      if (pendingSubscribers_.size() > 0) return false;
      if (subscribers_->size() > 0) return false;

      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::shutdown()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::States MediaDeviceCaptureAudio::getState() const
    {
      AutoRecursiveLock lock(*this);
      return currentState_;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => IMediaDeviceCaptureForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::mediaDeviceCaptureSubscribe(
                                                              MediaDeviceCapturePromisePtr promise,
                                                              MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                              TrackConstraintsPtr constraints,
                                                              IMediaDeviceCaptureDelegatePtr delegate
                                                              )
    {    
      auto pThis = thisWeak_.lock();

      PendingSubscriber info;
      info.promise_ = promise;
      info.repaceExistingDeviceObjectID_ = repaceExistingDeviceObjectID;
      info.constraints_ = constraints;
      info.delegate_ = delegate;

      postClosure([pThis, info] {
        AutoRecursiveLock lock(*pThis);
        pThis->pendingSubscribers_.push_back(info);
        pThis->step();
      });
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => IMediaDeviceCaptureAudioForMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::onWake()
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureAudioOnWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioOnPromiseSettled, ol, MediaEngine, Event, puid, id, id_, puid, promiseId, promise->getID());

      AutoRecursiveLock lock(*this);

      if (deviceModesPromise_) {
        if (deviceModesPromise_->getID() == promise->getID()) {
          if (promise->isRejected()) {
            setError(promise);
            cancel();
            return;
          }
        }
      }

      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(IMediaDevice::State_ShuttingDown);

      deviceModesPromise_.reset();

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
        ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "graceful");

        // perform any graceful asynchronous shutdown processes needed and
        // re-attempt shutdown again later if needed.

        if (!stepShutdownPendingRequests()) return;
        if (!stepShutdownSubscribers()) return;
        if (!stepShutdownMedia()) return;
      }

      //.......................................................................
      // final cleanup (hard shutdown)

      setState(IMediaDevice::State_Shutdown);

      stepShutdownPendingRequests();
      stepShutdownSubscribers();
      stepShutdownMedia();

      auto engine = mediaEngine_.lock();
      if (engine) {
        engine->notifyDeviceIsIdleOrShutdownStateChanged();
      }

      // make sure to cleanup any final reference to self
      gracefulShutdownReference_.reset();

      mediaEngine_.reset();
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepShutdownPendingRequests()
    {
      ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "pending requests");

      for (auto iter_doNotUse = pendingSubscribers_.begin(); iter_doNotUse != pendingSubscribers_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto pending = (*current).promise_;

        pending->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Gone, "media engine is shutting down"));
        pendingSubscribers_.erase(current);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepShutdownSubscribers()
    {
      if (subscribers_->size() < 1) return true;

      ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "subscribers");

      for (auto iter_doNotUse = subscribers_->begin(); iter_doNotUse != subscribers_->end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto subscriber = (*current).second;

        subscriber->shutdown();
      }

      // empty the subscriber list
      subscribers_ = make_shared<MediaSubscriberMap>();

      if (media_) {
        media_->notifySubscribersChanged(subscribers_);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepShutdownMedia()
    {
      ZS_EVENTING_2(x, i, Debug, MediaDeviceCaptureAudioShutdownStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media");

      if (!media_) return true;

      media_->shutdown();
      bool shutdown = media_->isShutdown();
      if (shutdown) {
        media_.reset();
      }
      return shutdown;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::step()
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureAudioStep, ol, MediaEngine, Step, puid, id, id_);

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureAudioStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      if (!stepMediaReinitializationShutdown()) goto reinitializing;
      if (!stepDiscoverModes()) goto not_ready;
      if (!stepFigureOutMode()) goto not_ready;
      if (!stepWaitForMediaDevice()) goto not_ready;

      goto ready;

    not_ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "not ready");
        return;
      }

    reinitializing:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "reinitializing");
        setState(IMediaDevice::State_Reinitializing);
        return;
      }

    ready:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "ready");
        setState(IMediaDevice::State_Active);
        return;
      }
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepMediaReinitializationShutdown()
    {
      if (!media_) goto no_media_to_shutdown;

      if (media_->isShuttingDown()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "waiting for media shutdown to reinialize");
        return false;
      }

      if (media_->isShutdown()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media shutdown");
        media_.reset();
        return true;
      }

    no_media_to_shutdown:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no media to shutdown");
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepDiscoverModes()
    {
      if (!deviceModesPromise_) {
        deviceModesPromise_ = IMediaDevices::enumerateDefaultModes(deviceID_);

        if (!deviceModesPromise_) {
          setError(UseHTTP::HTTPStatusCode_ExpectationFailed, (String("unable to discover modes for an audio media device: ") + deviceID_).c_str());
          cancel();
          return false;
        }
        deviceModesPromise_->thenWeak(thisWeak_.lock());
      }

      if (!deviceModesPromise_->isSettled()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "waiting for device modes to be known");
        return false;
      }

      if (deviceModesPromise_->isRejected()) {
        setError(deviceModesPromise_);
        cancel();
        return false;
      }

      auto settingsList = deviceModesPromise_->value();

      if (!settingsList) {
        setError(UseHTTP::HTTPStatusCode_ExpectationFailed, (String("no modes found for audio media device: ") + deviceID_).c_str());
        cancel();
        return false;
      }

      ZS_EVENTING_3(x, i, Debug, MediaDeviceCaptureAudioStepModesFound, ol, MediaEngine, Step,
        puid, id, id_,
        string, deviceId, deviceID_,
        size_t, totalModes, settingsList->size());

      // copy modes to shared pointer list of settings
      for (auto iter = settingsList->begin(); iter != settingsList->end(); ++iter)
      {
        deviceModes_.push_back(make_shared<UseSettings>(*iter));
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepFigureOutMode()
    {
      {
        if (recheckMode_) goto mode_needed;
        if (!media_) goto mode_needed;
        if (pendingSubscribers_.size() > 0) goto mode_needed;
        return true;
      }

    mode_needed:
      {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "figure out mode");

        recheckMode_ = false;

        TrackConstraintsList allConstraints;

        for (auto iter = pendingSubscribers_.begin(); iter != pendingSubscribers_.end(); ++iter)
        {
          auto &info = (*iter);
          allConstraints.push_back(info.constraints_);
        }

        for (auto iter = subscribers_->begin(); iter != subscribers_->end(); ++iter) {
          auto &subscriber = (*iter).second;
          allConstraints.push_back(subscriber->getConstraints());
        }

        auto found = obtainBestMode(allConstraints, deviceModes_);

        bool changed = false;

        MediaSubscriberMapPtr replacement(make_shared<MediaSubscriberMap>());
        for (auto iter = subscribers_->begin(); iter != subscribers_->end(); ++iter) {
          auto &subscriber = (*iter).second;
          if (subscriber->isShutdown()) {
            changed = true;
            continue;
          }
          (*replacement)[(*iter).first] = subscriber;
        }

        if (found) {
          for (auto iter = pendingSubscribers_.begin(); iter != pendingSubscribers_.end(); ++iter) {
            auto &info = (*iter);
            auto subscriber = MediaSubscriber::create(getAssociatedMessageQueue(), thisWeak_.lock(), info.constraints_, info.delegate_);
            info.promise_->resolve(IMediaDeviceCapturePtr(subscriber));
            subscriber->notifyStateChanged(currentState_);
            changed = true;
            (*replacement)[subscriber->getID()] = subscriber;
          }
        } else {
          for (auto iter = pendingSubscribers_.begin(); iter != pendingSubscribers_.end(); ++iter) {
            auto &info = (*iter);
            info.promise_->reject(ErrorAny::create(UseHTTP::HTTPStatusCode_Conflict, "over constrained"));
          }
        }
        pendingSubscribers_.clear();

        if (changed) {
          subscribers_ = replacement;
          if (media_) {
            media_->notifySubscribersChanged(subscribers_);
          }
        }

        if (subscribers_->size() < 1) {
          found = UseSettingsPtr();

          auto engine = mediaEngine_.lock();
          if (engine) {
            engine->notifyDeviceIsIdleOrShutdownStateChanged();
          }
        }

        if (found) {
          if (found == requiredSettings_) {
            ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "optimal device mode has not changed");
            return true;
          }
        }

        if (isReady()) {
          setState(IMediaDevice::State_Reinitializing);
        }

        requiredSettings_ = found;
        if (media_) {
          media_->shutdown();
        }
      }
      return false;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::stepWaitForMediaDevice()
    {
      if (media_->isShutdown()) {
        media_.reset();
      }

      if (!media_) {
        if (!requiredSettings_) {
          ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "no media device mode required");
          return false;
        }
        media_ = Media::create(getAssociatedMessageQueue(), thisWeak_.lock(), deviceID_, requiredSettings_);
        media_->notifySubscribersChanged(subscribers_);
      }

      if (!media_->isReady()) {
        ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media device is not ready");
        return false;
      }

      ZS_EVENTING_2(x, i, Trace, MediaDeviceCaptureAudioStepMessage, ol, MediaEngine, Step, puid, id, id_, string, message, "media device is ready");
      return true;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::setState(States state)
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceCaptureAudioSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IMediaDevice::toString(state), string, oldState, IMediaDevice::toString(currentState_));

      currentState_ = state;

      for (auto iter = subscribers_->begin(); iter != subscribers_->end(); ++iter) {
        auto &subscriber = (*iter).second;
        subscriber->notifyStateChanged(currentState_);
      }

//      MediaEnginePtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onMediaEngineStateChanged(pThis, currentState_);
//      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::setError(PromisePtr promise)
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureAudioSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureAudioSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::Media
    #pragma mark

    //-------------------------------------------------------------------------
    const char *MediaDeviceCaptureAudio::Media::toString(MediaStates state)
    {
      switch (state)
      {
        case MediaState_Pending:        return "pending";
        case MediaState_Ready:          return "ready";
        case MediaState_ShuttingDown:   return "shutting down";
        case MediaState_Shutdown:       return "shutdown";
      }
      return "UNKNOWN";
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::Media::Media(
                                          const make_private &,
                                          IMessageQueuePtr queue,
                                          UseOuterPtr outer,
                                          const String &deviceID,
                                          UseSettingsPtr settings
                                          ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      outer_(outer),
      deviceID_(deviceID),
      settings_(settings),
      trace_(MediaChannelTrace::create(id_))
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureAudioMediaCreate, ol, MediaEngine, Start, puid, id, id_, string, deviceID, deviceID_);

      trace_->mKind = IMediaStreamTrackTypes::Kind_Audio;
      trace_->mSettings = settings;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::init()
    {
      IWakeDelegateProxy::create(thisWeak_.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::Media::~Media()
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureAudioMediaDestroy, ol, MediaEngine, Stop, puid, id, id_, string, deviceID, deviceID_);

      thisWeak_.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::MediaPtr MediaDeviceCaptureAudio::Media::create(
                                                                             IMessageQueuePtr queue,
                                                                             UseOuterPtr outer,
                                                                             const String &deviceID,
                                                                             UseSettingsPtr settings
                                                                             )
    {
      auto pThis(make_shared<Media>(make_private{}, queue, outer, deviceID, settings));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::Media => (for MediaDeviceCaptureAudio)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::shutdown()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioMediaShutdown, ol, MediaEngine, Close, puid, id, id_);

      auto pThis = thisWeak_.lock();
      postClosure([pThis] {
        AutoRecursiveLock lock(*pThis);
        pThis->cancel();
      });
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::Media::isReady() const
    {
      AutoRecursiveLock lock(*this);
      return MediaState_Ready == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::Media::isShuttingDown() const
    {
      AutoRecursiveLock lock(*this);
      return MediaState_ShuttingDown == currentState_;
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::Media::isShutdown() const
    {
      AutoRecursiveLock lock(*this);
      return MediaState_Shutdown == currentState_;
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::notifySubscribersChanged(MediaSubscriberMapPtr subscribers)
    {
      AutoRecursiveLock lock(*this);
      subscribers_ = subscribers;
    }

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::Media => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::onWake()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioMediaWake, ol, MediaEngine, Event, puid, id, id_);

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::Media => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::cancel()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioMediaCancel, ol, MediaEngine, Cancel, puid, id, id_);

      if (isShutdown()) return;

      setState(MediaState_ShuttingDown);

      if (!gracefulShutdownReference_) gracefulShutdownReference_ = thisWeak_.lock();

      if (gracefulShutdownReference_) {
#define TODO 1
#define TODO 2
      }

      setState(MediaState_Shutdown);

#define TODO 1
#define TODO 2

      auto outer = outer_.lock();
      if (outer) {
        outer->notifyMediaStateChanged();
      }

      gracefulShutdownReference_.reset();
      outer_.reset();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::step()
    {
      ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureAudioMediaStep, ol, MediaEngine, Step, puid, id, id_);

#define TODO 1
#define TODO 2
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_EVENTING_1(x, i, Trace, MediaDeviceCaptureAudioMediaStepForwardCancel, ol, MediaEngine, Step, puid, id, id_);
        cancel();
        return;
      }

      // if (!stepDoSomething()) goto return;

      goto ready;

    ready:
      {
        setState(MediaState_Ready);
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::setState(MediaStates state)
    {
      if (state == currentState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceCaptureAudioMediaSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, toString(state), string, oldState, toString(currentState_));

      currentState_ = state;

      if (MediaState_Ready == state) {
        auto outer = outer_.lock();
        if (outer) {
          outer->notifyMediaStateChanged();
        }
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::setError(PromisePtr promise)
    {
      if (!promise) return;

      auto reason = promise->reason<ErrorAny>();
      if (!reason) return;

      setError(reason->mErrorCode, reason->mReason);
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != lastError_) {
        ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureAudioMediaSetErrorIgnored, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);
        return;
      }

      lastError_ = errorCode;
      lastErrorReason_ = reason;

      ZS_EVENTING_3(x, e, Detail, MediaDeviceCaptureAudioMediaSetError, ol, MediaEngine, ErrorEvent, puid, id, id_, word, error, lastError_, string, reason, lastErrorReason_);

      auto pThis = thisWeak_.lock();
      if (pThis) {
        auto outer = outer_.lock();
        if (outer) {
          outer->notifyMediaFailure(pThis, lastError_, lastErrorReason_);
        }
      }
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::Media::internalNotifySubscribersOfFrame(AudioFramePtr frame)
    {
      MediaSubscriberMapPtr subscribers;

      ZS_EVENTING_1(x, e, Insane, MediaDeviceCaptureAudioMediaNotifySubscribersOfFrame, ol, MediaEngine, InternalEvent, puid, id, id_);

      {
        AutoRecursiveLock lock(*this);
        subscribers = subscribers_;
      }

      if (!subscribers) return;

      for (auto iter = subscribers->begin(); iter != subscribers->end(); ++iter)
      {
        auto &subscriber = (*iter).second;
        subscriber->notifyFrame(trace_, frame);
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber
    #pragma mark

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::MediaSubscriber::MediaSubscriber(
                                                              const make_private &,
                                                              IMessageQueuePtr queue,
                                                              UseOuterPtr outer,
                                                              TrackConstraintsPtr constraints,
                                                              IMediaDeviceCaptureDelegatePtr delegate
                                                              ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      outer_(outer),
      constraints_(constraints),
      notifyDelegate_(delegate_),
      delegate_(IMediaDeviceCaptureDelegateProxy::createWeak(delegate)),
      traceHelper_(id_)
    {
      ZS_EVENTING_2(x, i, Detail, MediaDeviceCaptureAudioMediaSubscriberCreate, ol, MediaEngine, Start, puid, id, id_, puid, outerId, outer->getID());
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::MediaSubscriber::init()
    {
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudio::MediaSubscriber::~MediaSubscriber()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioMediaSubscriberDestroy, ol, MediaEngine, Stop, puid, id, id_);

      thisWeak_.reset();
      cancel();
    }

    MediaDeviceCaptureAudio::MediaSubscriberPtr MediaDeviceCaptureAudio::MediaSubscriber::create(
                                                                                                 IMessageQueuePtr queue,
                                                                                                 UseOuterPtr outer,
                                                                                                 TrackConstraintsPtr constraints,
                                                                                                 IMediaDeviceCaptureDelegatePtr delegate
                                                                                                 )
    {
      auto pThis(make_shared<MediaSubscriber>(make_private{}, queue, outer, constraints, delegate));
      pThis->thisWeak_ = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber => (for MediaDeviceCaptureAudio)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::MediaSubscriber::shutdown()
    {
      ZS_EVENTING_1(x, i, Detail, MediaDeviceCaptureAudioMediaSubscriberShutdown, ol, MediaEngine, Close, puid, id, id_);

      AutoRecursiveLock lock(*this);
      outer_.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    bool MediaDeviceCaptureAudio::MediaSubscriber::isShutdown() const
    {
      return IMediaDevice::State_Shutdown == getState();
    }

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::MediaSubscriber::notifyStateChanged(States state)
    {
      auto pThis = thisWeak_.lock();

      postClosure([pThis, state] {
        AutoRecursiveLock lock(*pThis);
        pThis->setState(state);
      });
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber => (for Media)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::MediaSubscriber::notifyFrame(
                                                               ImmutableMediaChannelTracePtr trace,
                                                               AudioFramePtr frame
                                                               )
    {
      IMediaDeviceCaptureDelegatePtr delegate;

      {
        AutoRecursiveLock lock(*this);
        delegate = notifyDelegate_.lock();
      }

      if (!delegate) return;
      delegate->notifyMediaDeviceCaptureAudioFrame(traceHelper_.trace(trace), frame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber => IMediaDevice
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::MediaSubscriber::cancel()
    {
      AutoRecursiveLock lock(*this);

      setState(IMediaDevice::State_ShuttingDown);

      setState(IMediaDevice::State_Shutdown);

      auto outer = outer_.lock();
      if (outer) {
        outer->notifySusbcriberGone();
      }
      outer_.reset();

      delegate_.reset();
      notifyDelegate_.reset();
    }

    //-------------------------------------------------------------------------
    IMediaDevice::States MediaDeviceCaptureAudio::MediaSubscriber::getState() const
    {
      AutoRecursiveLock lock(*this);
      return lastReportedState_;
    }

    //---------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber => IMediaDeviceCapture
    #pragma mark

    //---------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber => IMediaDeviceCaptureAudio
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceCaptureAudio::MediaSubscriber => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaDeviceCaptureAudio::MediaSubscriber::setState(States state)
    {
      if (state == lastReportedState_) return;
      if (IMediaDevice::State_Shutdown == lastReportedState_) return;

      ZS_EVENTING_3(x, i, Detail, MediaDeviceCaptureAudioMediaSubscriberSetState, ol, MediaEngine, StateEvent, puid, id, id_, string, newState, IMediaDevice::toString(state), string, oldState, IMediaDevice::toString(lastReportedState_));

      lastReportedState_ = state;

      auto pThis = thisWeak_.lock();
      if (!pThis) return;
      if (!delegate_) return;

      try {
        delegate_->onMediaDeviceCaptureStateChanged(pThis, state);
      } catch (const IMediaDeviceCaptureDelegateProxy::Exceptions::DelegateGone &) {
        delegate_.reset();
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceCaptureAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaDeviceCaptureAudioFactory &IMediaDeviceCaptureAudioFactory::singleton()
    {
      return MediaDeviceCaptureAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    MediaDeviceCaptureAudioPtr IMediaDeviceCaptureAudioFactory::create(
                                                                       UseMediaEnginePtr mediaEngine,
                                                                       const String &deviceID
                                                                       )
    {
      if (this) {}
      return internal::MediaDeviceCaptureAudio::create(mediaEngine, deviceID);
    }

  } // internal namespace
}
