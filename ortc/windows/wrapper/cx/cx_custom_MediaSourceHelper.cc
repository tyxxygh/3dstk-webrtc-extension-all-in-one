// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.


#include "cx_custom_MediaSourceHelper.h"

#include <webrtc/rtc_base/timing.h>
#include <webrtc/rtc_base/timeutils.h>

#include <MFapi.h>

namespace Org
{
  namespace Ortc
  {

    // Helper functions defined below.
    bool IsSampleIDR(IMFSample* sample);
    bool DropFramesToIDR(std::list<webrtc::VideoFrame*>& frames);


    SampleData::SampleData()
      : sizeHasChanged(false)
      , size({ -1, -1 })
      , rotationHasChanged(false)
      , rotation(-1) {
    }

    MediaSourceHelper::MediaSourceHelper(bool isH264,
      std::function<HRESULT(webrtc::VideoFrame* frame, IMFSample** sample)> mkSample,
      std::function<void(int)> fpsCallback)
      : _mkSample(mkSample)
      , _fpsCallback(fpsCallback)
      , _isFirstFrame(true)
      , _futureOffsetMs(45)
      , _lastSampleTime(0)
      , _lastSize({ 0, 0 })
      , _lastRotation(-1)
      , _isH264(isH264)
      , _frameCounter(0)
      , _startTime(0)
      , _lastTimeFPSCalculated(webrtc::TickTime::Now())
      , _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection()) {

    }
    MediaSourceHelper::~MediaSourceHelper() {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      // Clear the buffered frames.
      while (!_frames.empty()) {
        rtc::scoped_ptr<webrtc::VideoFrame> frame(_frames.front());
        _frames.pop_front();
      }
    }

    void MediaSourceHelper::QueueFrame(webrtc::VideoFrame* frame) {
      webrtc::CriticalSectionScoped csLock(_lock.get());

      if (_isH264) {
        // For H264 we keep all frames since they are encoded.
        _frames.push_back(frame);
      }
      else {
        // For I420 frame, keep only the latest.
        for (auto oldFrame : _frames) {
          delete oldFrame;
        }
        _frames.clear();
        _frames.push_back(frame);
      }
    }

    rtc::scoped_ptr<SampleData> MediaSourceHelper::DequeueFrame() {
      webrtc::CriticalSectionScoped csLock(_lock.get());

      if (_frames.size() == 0) {
        return nullptr;
      }

      rtc::scoped_ptr<SampleData> data;
      if (_isH264) {
        data = DequeueH264Frame();
      }
      else {
        data = DequeueI420Frame();
      }

      // Set the timestamp property
      if (_isFirstFrame) {
        _isFirstFrame = false;
        FirstFrameRenderHelper::FireEvent(
          rtc::Timing::WallTimeNow() * rtc::kNumMillisecsPerSec);
        LONGLONG frameTime = GetNextSampleTimeHns(data->renderTime);
        data->sample->SetSampleTime(frameTime);
      } else {

        LONGLONG frameTime = GetNextSampleTimeHns(data->renderTime);

        data->sample->SetSampleTime(frameTime);

        // Set the duration property
        if (_isH264) {
          data->sample->SetSampleDuration(frameTime - _lastSampleTime);
        }
        else {
          LONGLONG duration = (LONGLONG)((1.0 / 30) * 1000 * 1000 * 10);
          data->sample->SetSampleDuration(duration);
        }
        _lastSampleTime = frameTime;
      }

      UpdateFrameRate();

      return data;
    }

    bool MediaSourceHelper::HasFrames() {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      return _frames.size() > 0;
    }

    // === Private functions below ===

    rtc::scoped_ptr<SampleData> MediaSourceHelper::DequeueH264Frame() {

      if (_frames.size() > 15)
        DropFramesToIDR(_frames);

      rtc::scoped_ptr<webrtc::VideoFrame> frame(_frames.front());
      _frames.pop_front();

      rtc::scoped_ptr<SampleData> data(new SampleData);

      // Get the IMFSample in the frame.
      {
        IMFSample* tmp = (IMFSample*)frame->video_frame_buffer()->native_handle();
        if (tmp != nullptr) {
          tmp->AddRef();
          data->sample.Attach(tmp);
          data->renderTime = frame->timestamp();

          ComPtr<IMFAttributes> sampleAttributes;
          data->sample.As(&sampleAttributes);
          if (IsSampleIDR(tmp)) {
            sampleAttributes->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
          }
        }
      }

      CheckForAttributeChanges(frame.get(), data.get());
      return data;
    }

    rtc::scoped_ptr<SampleData> MediaSourceHelper::DequeueI420Frame() {
      rtc::scoped_ptr<webrtc::VideoFrame> frame(_frames.front());
      _frames.pop_front();

      rtc::scoped_ptr<SampleData> data(new SampleData);

      if (FAILED(_mkSample(frame.get(), &data->sample))) {
        return nullptr;
      }

      ComPtr<IMFAttributes> sampleAttributes;
      data->sample.As(&sampleAttributes);
      sampleAttributes->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
      sampleAttributes->SetUINT32(MFSampleExtension_Discontinuity, TRUE);

      CheckForAttributeChanges(frame.get(), data.get());
      return data;
    }

    // Guid to cache the IDR check result in the sample attributes.
    const GUID GUID_IS_IDR = { 0x588e319a, 0x218c, 0x4d0d,{ 0x99, 0x6e, 0x77, 0x96, 0xb1, 0x46, 0x3e, 0x7e } };

    bool IsSampleIDR(IMFSample* sample) {
      ComPtr<IMFAttributes> sampleAttributes;
      sample->QueryInterface<IMFAttributes>(&sampleAttributes);

      UINT32 isIdr;
      if (SUCCEEDED(sampleAttributes->GetUINT32(GUID_IS_IDR, &isIdr))) {
        return isIdr > 0;
      }

      ComPtr<IMFMediaBuffer> pBuffer;
      sample->GetBufferByIndex(0, &pBuffer);
      BYTE* pBytes;
      DWORD maxLength, curLength;
      if (FAILED(pBuffer->Lock(&pBytes, &maxLength, &curLength))) {
        return false;
      }

      // Search for the beginnings of nal units.
      for (DWORD i = 0; i < curLength - 5; ++i) {
        BYTE* ptr = pBytes + i;
        int prefixLengthFound = 0;
        if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01) {
          prefixLengthFound = 4;
        }
        else if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01) {
          prefixLengthFound = 3;
        }

        if (prefixLengthFound > 0 && (ptr[prefixLengthFound] & 0x1f) == 0x05) {
          // Found IDR NAL unit
          pBuffer->Unlock();
          sampleAttributes->SetUINT32(GUID_IS_IDR, 1);  // Cache result
          return true;
        }
      }
      pBuffer->Unlock();
      sampleAttributes->SetUINT32(GUID_IS_IDR, 0);  // Cache result
      return false;
    }

    bool DropFramesToIDR(std::list<webrtc::VideoFrame*>& frames) {
      webrtc::VideoFrame* idrFrame = nullptr;
      // Go through the frames in reverse order (from newest to oldest) and look
      // for an IDR frame.
      for (auto it = frames.rbegin(); it != frames.rend(); ++it) {
        IMFSample* pSample = (IMFSample*)(*it)->video_frame_buffer()->native_handle();
        if (pSample == nullptr) {
          continue;  // I don't expect this will ever happen.
        }

        if (IsSampleIDR(pSample)) {
          idrFrame = *it;
          break;
        }
      }

      // If we have an IDR frame, drop all older frames.
      if (idrFrame != nullptr) {
        OutputDebugString(L"IDR found, dropping all other samples.\r\n");
        while (!frames.empty()) {
          if (frames.front() == idrFrame) {
            break;
          }
          auto frame = frames.front();
          frames.pop_front();
          delete frame;
        }
      }
      return idrFrame != nullptr;
    }

    void MediaSourceHelper::SetStartTimeNow() {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      _startTickTime = webrtc::TickTime::Now();
      if (_isH264) {
        if (!DropFramesToIDR(_frames)) {
          // Flush all frames then.
          while (!_frames.empty()) {
            rtc::scoped_ptr<webrtc::VideoFrame> frame(_frames.front());
            _frames.pop_front();
          }
        }
      }
    }

#define USE_WALL_CLOCK
    LONGLONG MediaSourceHelper::GetNextSampleTimeHns(LONGLONG frameRenderTime) {

      if (_isH264) {
#ifdef USE_WALL_CLOCK
        if (_startTickTime.Ticks() == 0) {
          _startTickTime = webrtc::TickTime::Now();
          return 0;
        }
        LONGLONG frameTime = ((webrtc::TickTime::Now() - _startTickTime).Milliseconds() + _futureOffsetMs) * 1000 * 10;
#else
        if (_startTime == 0) {

          _startTime = frameRenderTime;
          // Return zero here so the first frame starts at zero.
          // Only follow-up samples get an future offset.
          return 0;
        }

        LONGLONG frameTime = (frameRenderTime - _startTime) / 100 + (_futureOffsetMs * 1000 * 10);
#endif

        return frameTime;
      }
      else {
        // Non-encoded samples seem to work best with a zero timestamp.
        return 0;
      }
    }

    void MediaSourceHelper::CheckForAttributeChanges(webrtc::VideoFrame* frame, SampleData* data) {
      SIZE currentSize = { (LONG)frame->width(), (LONG)frame->height() };
      if (_lastSize.cx != currentSize.cx || _lastSize.cy != currentSize.cy) {
        data->sizeHasChanged = true;
        data->size = currentSize;
        _lastSize = currentSize;
      }

      // Update rotation property
      int currentRotation = frame->rotation();

      // If the rotation has changed
      if (_lastRotation == -1 || _lastRotation != currentRotation) {
        data->rotationHasChanged = true;
        data->rotation = currentRotation;
        _lastRotation = currentRotation;
      }

    }

    // Called whenever a new sample is sent for rendering.
    void MediaSourceHelper::UpdateFrameRate() {
      // Do FPS calculation and notification.
      _frameCounter++;
      // If we have about a second worth of frames
      webrtc::TickTime now = webrtc::TickTime::Now();
      if ((now - _lastTimeFPSCalculated).Milliseconds() > 1000) {
        _fpsCallback(_frameCounter);
        _frameCounter = 0;
        _lastTimeFPSCalculated = now;
      }
    }

    void FirstFrameRenderHelper::FireEvent(double timestamp) {
      FirstFrameRendered(timestamp);
    }
  } // namespace ortc
}  // namespace org
