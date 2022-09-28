/*

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

#pragma once

#include <ortc/internal/types.h>

#include <ortc/internal/ortc_IMediaDeviceRender.h>
#include <ortc/internal/ortc_MediaChannelTraceHelper.h>
#include <ortc/IMediaDevices.h>

#include <zsLib/IWakeDelegate.h>

namespace ortc
{
  namespace internal
  {

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceRenderAudio (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaDeviceRenderAudio
    #pragma mark
    
    class MediaDeviceRenderAudio : public Any,
                                   public Noop,
                                   public MessageQueueAssociator,
                                   public SharedRecursiveLock,
                                   public IMediaDeviceRenderAudioForMediaEngine,
                                   public zsLib::IWakeDelegate,
                                   public zsLib::IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaDeviceRenderAudioForMediaEngine;

      ZS_DECLARE_CLASS_PTR(Media);
      ZS_DECLARE_CLASS_PTR(MediaSubscriber);

      friend class Media;
      friend class MediaSubscriber;

      typedef PUID ObjectID;
      typedef std::map<ObjectID, MediaSubscriberPtr> MediaSubscriberMap;
      ZS_DECLARE_PTR(MediaSubscriberMap);

      typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, PromiseWithSettingsList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::States, States);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, UsePromiseWithSettingsList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Settings, UseSettings);
      ZS_DECLARE_TYPEDEF_PTR(std::list<UseSettingsPtr>, SettingsList);

      ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamTrackTypes::TrackConstraintsPtr>, TrackConstraintsList);

      struct PendingSubscriber
      {
        MediaDeviceRenderPromisePtr promise_;
        MediaDeviceObjectID repaceExistingDeviceObjectID_ {};
        TrackConstraintsPtr constraints_;
        IMediaDeviceRenderDelegatePtr delegate_;
      };

      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingSubscriber>, PendingSubscriberList);

    public:
      MediaDeviceRenderAudio(
                              const make_private &,
                              IMessageQueuePtr queue,
                              UseMediaEnginePtr mediaEngine,
                              const String &deviceID
                              );

    protected:
      MediaDeviceRenderAudio(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~MediaDeviceRenderAudio();

      static MediaDeviceRenderAudioPtr create(
                                               UseMediaEnginePtr mediaEngine,
                                               const String &deviceID
                                               );

      static MediaDeviceRenderAudioPtr convert(ForMediaEnginePtr object);


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => (for Media)
      #pragma mark

      void notifyMediaStateChanged();
      void notifyMediaFailure(
                              MediaPtr media,
                              WORD errorCode,
                              const char *reason
                              );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => (for MediaSubscribers)
      #pragma mark

      PUID getID() const { return id_; }
      void notifySusbcriberGone();
      void notifyAudioFrame(
                            ImmutableMediaChannelTracePtr trace,
                            AudioFramePtr frame
                            );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => IMediaDeviceForMediaEngine
      #pragma mark

      bool isDeviceIdle() override;
      void shutdown() override;

      States getState() const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => IMediaDeviceRenderForMediaEngine
      #pragma mark

      void mediaDeviceRenderSubscribe(
                                      MediaDeviceRenderPromisePtr promise,
                                      MediaDeviceObjectID repaceExistingDeviceObjectID,
                                      TrackConstraintsPtr constraints,
                                      IMediaDeviceRenderDelegatePtr delegate
                                      ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => IMediaDeviceRenderAudioForMediaEngine
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => IWakeDelegate
      #pragma mark

      void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => IPromiseSettledDelegate
      #pragma mark

      void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => (internal)
      #pragma mark

      bool isPending() const { return IMediaDevice::State_Pending == currentState_; }
      bool isReady() const { return IMediaDevice::State_Active == currentState_; }
      bool isReinitializing() const { return IMediaDevice::State_Reinitializing == currentState_; }
      bool isShuttingDown() const { return IMediaDevice::State_ShuttingDown == currentState_; }
      bool isShutdown() const { return IMediaDevice::State_Shutdown == currentState_; }

      void cancel();

      bool stepShutdownPendingRequests();
      bool stepShutdownSubscribers();
      bool stepShutdownMedia();

      void step();
      bool stepMediaReinitializationShutdown();
      bool stepDiscoverModes();
      bool stepFigureOutMode();
      bool stepWaitForMediaDevice();

      void setState(States state);
      void setError(PromisePtr promise);
      void setError(WORD error, const char *inReason);

    public:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio::Media
      #pragma mark

      class Media : public MessageQueueAssociator,
                    public SharedRecursiveLock,
                    public zsLib::IWakeDelegate
      {
      protected:
        struct make_private {};

        enum MediaStates
        {
          MediaState_Pending,
          MediaState_Ready,
          MediaState_ShuttingDown,
          MediaState_Shutdown,
        };

        static const char *toString(MediaStates state);

      public:
        ZS_DECLARE_TYPEDEF_PTR(MediaDeviceRenderAudio, UseOuter);

      public:
        Media(
              const make_private &,
              IMessageQueuePtr queue,
              UseOuterPtr outer,
              const String &deviceID,
              UseSettingsPtr settings
              );

      protected:
        void init();

      public:
        virtual ~Media();

        static MediaPtr create(
                               IMessageQueuePtr queue,
                               UseOuterPtr outer,
                               const String &deviceID,
                               UseSettingsPtr settings
                               );

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::Media => (for MediaDeviceRenderAudio)
        #pragma mark

        MediaChannelID getID() const { return id_; }

        void shutdown();
        bool isReady() const;
        bool isShuttingDown() const;
        bool isShutdown() const;

        void notifySubscribersChanged(MediaSubscriberMapPtr subscribers);

        void notifyAudioFrame(
                              ImmutableMediaChannelTracePtr trace,
                              AudioFramePtr frame
                              );

        //-----------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::Media => IWakeDelegate
        #pragma mark

        virtual void onWake() override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::Media => (internal)
        #pragma mark

        void cancel();
        void step();

        void setState(MediaStates state);
        void setError(PromisePtr promise);
        void setError(WORD errorCode, const char *inReason);

      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::Media => (data)
        #pragma mark

        AutoPUID id_;
        MediaWeakPtr thisWeak_;

        MediaPtr gracefulShutdownReference_;

        UseOuterWeakPtr outer_;
        MediaSubscriberMapPtr subscribers_;
        String deviceID_;
        UseSettingsPtr settings_;

        MediaStates currentState_ {MediaState_Pending};
        WORD lastError_ {};
        String lastErrorReason_;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio::MediaSubscriber
      #pragma mark

      class MediaSubscriber : public MessageQueueAssociator,
                              public SharedRecursiveLock,
                              public IMediaDeviceRenderAudio
      {
      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(MediaDeviceRenderAudio, UseOuter);

      public:
        MediaSubscriber(
                        const make_private &,
                        IMessageQueuePtr queue,
                        UseOuterPtr outer,
                        TrackConstraintsPtr constraints,
                        IMediaDeviceRenderDelegatePtr delegate
                        );

      protected:
        void init();

      public:
        virtual ~MediaSubscriber();

        static MediaSubscriberPtr create(
                                         IMessageQueuePtr queue,
                                         UseOuterPtr outer,
                                         TrackConstraintsPtr constraints,
                                         IMediaDeviceRenderDelegatePtr delegate
                                         );

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => (for MediaDeviceRenderAudio)
        #pragma mark

        virtual MediaDeviceObjectID getID() const override { return id_; }
        void shutdown();
        bool isShutdown() const;
        void notifyStateChanged(States state);
        TrackConstraintsPtr getConstraints() const { return constraints_; }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => (for Media)
        #pragma mark

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => IMediaDevice
        #pragma mark

        // (duplicate) virtual MediaDeviceObjectID getID() const override { return id_; }
        virtual void cancel() override;

        virtual States getState() const override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => IMediaDeviceRender
        #pragma mark

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => IMediaDeviceRenderAudio
        #pragma mark

        virtual void notifyAudioFrame(
                                      ImmutableMediaChannelTracePtr trace,
                                      AudioFramePtr frame
                                      ) override;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => (internal)
        #pragma mark

        void setState(States state);

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaDeviceRenderAudio::MediaSubscriber => (data)
        #pragma mark

        AutoPUID id_;
        MediaSubscriberWeakPtr thisWeak_;

        UseOuterWeakPtr outer_;
        TrackConstraintsPtr constraints_;

        IMediaDeviceRenderDelegateWeakPtr notifyDelegate_;
        IMediaDeviceRenderDelegatePtr delegate_;

        States lastReportedState_;
        MediaChannelTraceHelper traceHelper_;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaDeviceRenderAudio => (data)
      #pragma mark

      AutoPUID id_;
      MediaDeviceRenderAudioWeakPtr thisWeak_;

      MediaDeviceRenderAudioPtr gracefulShutdownReference_;

      UseMediaEngineWeakPtr mediaEngine_;
      String deviceID_;

      States currentState_ {IMediaDevice::State_Pending};
      WORD lastError_ {};
      String lastErrorReason_;

      PromiseWithSettingsListPtr deviceModesPromise_;
      SettingsList deviceModes_;

      PendingSubscriberList pendingSubscribers_;
      MediaSubscriberMapPtr subscribers_;

      std::atomic<bool> recheckMode_ {};
      UseSettingsPtr requiredSettings_;
      MediaPtr media_;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDeviceRenderAudioFactory
    #pragma mark

    interaction IMediaDeviceRenderAudioFactory
    {
      static IMediaDeviceRenderAudioFactory &singleton();

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceRenderAudio, UseMediaEngine);

      virtual MediaDeviceRenderAudioPtr create(
                                                UseMediaEnginePtr mediaEngine,
                                                const String &deviceID
                                                );
    };

    class MediaDeviceRenderAudioFactory : public IFactory<IMediaDeviceRenderAudioFactory> {};
  }
}
