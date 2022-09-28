
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#pragma once

#include <webrtc/system_wrappers/include/critical_section_wrapper.h>
#include <webrtc/video_frame.h>

#include <MFidl.h>

namespace Org
{
  namespace Ortc
  {
    using Microsoft::WRL::ComPtr;

    /// <summary>
    /// Delegate used to notify about first video frame rendering.
    /// </summary>
    public delegate void FirstFrameRenderedEventHandler(double timestamp);

    public ref class FirstFrameRenderHelper sealed {
    public:
      /// <summary>
      /// Event fires when the first video frame renders.
      /// </summary>
      static event FirstFrameRenderedEventHandler^ FirstFrameRendered;
    internal:
      static void FireEvent(double timestamp);
    };

    struct SampleData {
      SampleData();
      ComPtr<IMFSample> sample;
      bool sizeHasChanged;
      SIZE size;
      bool rotationHasChanged;
      int rotation;
      LONGLONG renderTime;
    };

    class MediaSourceHelper { 
    public:
      MediaSourceHelper(bool isH264,
        std::function<HRESULT(webrtc::VideoFrame* frame, IMFSample** sample)> mkSample,
        std::function<void(int)> fpsCallback);
      ~MediaSourceHelper();

      void SetStartTimeNow();
      void QueueFrame(webrtc::VideoFrame* frame);
      std::unique_ptr<SampleData> DequeueFrame();
      bool HasFrames();

    private:
      std::unique_ptr<webrtc::CriticalSectionWrapper> _lock;
      std::list<webrtc::VideoFrame*> _frames;
      bool _isFirstFrame;
      LONGLONG _startTime;
      // One peculiarity, the timestamp of a sample should be slightly
      // in the future for Media Foundation to handle it properly.
      int _futureOffsetMs;
      // We keep the last sample time to catch cases where samples are
      // requested so quickly that the sample time doesn't change.
      // We then increment it slightly to prevent giving MF duplicate times.
      LONGLONG _lastSampleTime;
      // Stored to detect changes.
      SIZE _lastSize;
      // In degrees.  In practice it can only be 0, 90, 180 or 270.
      int _lastRotation;

      std::unique_ptr<SampleData> DequeueH264Frame();
      std::unique_ptr<SampleData> DequeueI420Frame();


      // Gets the next timestamp using the clock.
      // Guarantees no duplicate timestamps.
      LONGLONG GetNextSampleTimeHns(LONGLONG frameRenderTime);

      void CheckForAttributeChanges(webrtc::VideoFrame* frame, SampleData* data);

      std::function<HRESULT(webrtc::VideoFrame* frame, IMFSample** sample)> _mkSample;
      std::function<void(int)> _fpsCallback;

      // Called whenever a new sample is sent for rendering.
      void UpdateFrameRate();
      // State related to calculating FPS.
      int _frameCounter;
      int64_t _lastTimeFPSCalculated;

      // Are the frames H264 encoded.
      bool _isH264;
      int64_t _startTickTime;
    };

  } // namespace ortc
}  // namespace org
