#include "pch.h"

#include <org/ortc/adapter/MediaStream.h>
#include <org/ortc/MediaStreamTrack.h>
#include <org/ortc/Error.h>
#include <org/ortc/helpers.h>
//#include <org/ortc//RTCIceGatherer.h>
//#include <org/ortc//RTCDtlsTransport.h>
//#include <org/ortc//RTCSrtpSdesTransport.h>
//#include <org/ortc//RTPTypes.h>
//#include <org/ortc//RTCSctpTransport.h>
//
#include <ortc/internal/ortc_Helper.h>
//#include <ortc/adapter/IHelper.h>
//#include <zsLib/SafeInt.h>

using Platform::Collections::Vector;

using namespace ortc;

namespace Org
{
  namespace Ortc
  {
    namespace Adapter
    {
      ZS_DECLARE_TYPEDEF_PTR(Org::Ortc::Internal::Helper, UseHelper);
      ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IMediaStreamTypes, IMediaStreamTypes);
      //ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IHelper, UseAdapterHelper);
      ZS_DECLARE_TYPEDEF_PTR(::ortc::adapter::IMediaStreamDelegate, IMediaStreamDelegate);

      using std::make_shared;

      namespace Internal
      {

#pragma region MediaStreamDelegate

        class MediaStreamDelegate : public IMediaStreamDelegate
        {
        public:
          MediaStreamDelegate(MediaStream^ owner) { _owner = owner; }

          virtual void onMediaStreamAddTrack(
            IMediaStreamPtr stream,
            IMediaStreamTrackPtr track
            ) override
          {
            if (!_owner) return;
            _owner->OnAddTrack(MediaStreamTrack::Convert(track));
          }
          virtual void onMediaStreamRemoveTrack(
            IMediaStreamPtr stream,
            IMediaStreamTrackPtr track
            ) override
          {
            if (!_owner) return;
            _owner->OnRemoveTrack(MediaStreamTrack::Convert(track));
          }

        private:
          MediaStream^ _owner;
        };

#pragma endregion

      }

#pragma region Mediastream

      MediaStream::MediaStream(IMediaStreamPtr nativeStream) :
        _nativePointer(nativeStream),
        _nativeDelegatePointer(make_shared<Internal::MediaStreamDelegate>(this))
      {
        if (_nativePointer)
        {
          _nativeSubscriptionPointer = _nativePointer->subscribe(_nativeDelegatePointer);
        }
      }

      MediaStream::MediaStream() :
        _nativeDelegatePointer(make_shared<Internal::MediaStreamDelegate>(this))
      {
        _nativePointer = IMediaStream::create(_nativeDelegatePointer);
      }

      MediaStream::MediaStream(MediaStream^ stream)
      {
        _nativePointer = IMediaStream::create(_nativeDelegatePointer, Convert(stream));
      }

      MediaStream::MediaStream(IVector<MediaStreamTrack^>^ tracks)
      {
        IMediaStreamTypes::MediaStreamTrackList nativeTracks;
        for (MediaStreamTrack^ track : tracks)
        {
          auto nativeTrack = MediaStreamTrack::Convert(track);
          nativeTracks.push_back(nativeTrack);
        }
        _nativePointer = IMediaStream::create(_nativeDelegatePointer, nativeTracks);
      }

      Platform::String^ MediaStream::Id::get()
      {
        ORG_ORTC_THROW_INVALID_STATE_IF(!_nativePointer);
        return UseHelper::ToCx(_nativePointer->id());
      }

      Platform::Boolean MediaStream::Active::get()
      {
        if (!_nativePointer) return false;
        return _nativePointer->active();
      }

      IVector<MediaStreamTrack^>^ MediaStream::GetAudioTracks()
      {
        auto result = ref new Vector<MediaStreamTrack^>();
        if (!_nativePointer) return result;

        auto nativeTracks = _nativePointer->getAudioTracks();
        for (auto iter = nativeTracks->begin(); iter != nativeTracks->end(); ++iter)
        {
          auto nativeTrack = (*iter);
          result->Append(MediaStreamTrack::Convert(nativeTrack));
        }
        return result;
      }

      IVector<MediaStreamTrack^>^ MediaStream::GetVideoTracks()
      {
        auto result = ref new Vector<MediaStreamTrack^>();
        if (!_nativePointer) return result;

        auto nativeTracks = _nativePointer->getVideoTracks();
        for (auto iter = nativeTracks->begin(); iter != nativeTracks->end(); ++iter)
        {
          auto nativeTrack = (*iter);
          result->Append(MediaStreamTrack::Convert(nativeTrack));
        }
        return result;
      }

      IVector<MediaStreamTrack^>^ MediaStream::GetTracks()
      {
        auto result = ref new Vector<MediaStreamTrack^>();
        if (!_nativePointer) return result;

        auto nativeTracks = _nativePointer->getTracks();
        for (auto iter = nativeTracks->begin(); iter != nativeTracks->end(); ++iter)
        {
          auto nativeTrack = (*iter);
          result->Append(MediaStreamTrack::Convert(nativeTrack));
        }
        return result;
      }

      MediaStreamTrack^ MediaStream::GetTrackById(Platform::String^ trackId)
      {
        if (!_nativePointer) return nullptr;
        return MediaStreamTrack::Convert(_nativePointer->getTrackByID(UseHelper::FromCx(trackId).c_str()));
      }

      void MediaStream::AddTrack(MediaStreamTrack^ track)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS_IF(!track)
        ORG_ORTC_THROW_INVALID_STATE_IF(!_nativePointer);
        try
        {
          _nativePointer->addTrack(MediaStreamTrack::Convert(track));
        }
        catch (const InvalidParameters &)
        {
          ORG_ORTC_THROW_INVALID_PARAMETERS()
        }
      }

      void MediaStream::RemoveTrack(MediaStreamTrack^ track)
      {
        if (!track) return;
        if (!_nativePointer) return;
        _nativePointer->removeTrack(MediaStreamTrack::Convert(track));
      }

      MediaStream^ MediaStream::Clone()
      {
        if (!_nativePointer) return nullptr;
        auto nativeResult = _nativePointer->clone();
        return MediaStream::Convert(nativeResult);
      }

#pragma endregion

    } // namespace adapter
  } // namespace ortc
} // namespace org
