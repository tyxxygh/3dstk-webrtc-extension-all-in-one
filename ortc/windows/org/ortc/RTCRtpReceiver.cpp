#include "pch.h"

#include <org/ortc/RTCRtpReceiver.h>
#include <org/ortc/RTCDtlsTransport.h>
#include <org/ortc/RTCIceTransport.h>
#include <org/ortc/RTCSrtpSdesTransport.h>
#include <org/ortc/RtpTypes.h>
#include <org/ortc/helpers.h>
#include <org/ortc/Error.h>

#include <zsLib/SafeInt.h>

using namespace ortc;

using Platform::Collections::Vector;

namespace Org
{
  namespace Ortc
  {
    using Windows::Foundation::Collections::IVector;

    ZS_DECLARE_TYPEDEF_PTR(Internal::Helper, UseHelper)

    namespace Internal
    {
#pragma region RTCRtpReceiver delegates

      class RTCRtpReceiverDelegate : public IRTPReceiverDelegate
      {
      public:
        RTCRtpReceiverDelegate(RTCRtpReceiver^ owner) { _owner = owner; }

      private:
        RTCRtpReceiver^ _owner;
      };

#pragma endregion

#pragma region RTCRtpReceiver observers 

      class RTCRtpReceiverPromiseObserver : public zsLib::IPromiseResolutionDelegate
      {
      public:
        RTCRtpReceiverPromiseObserver(Concurrency::task_completion_event<void> tce) : mTce(tce) {}

        virtual void onPromiseResolved(PromisePtr promise) override
        {
          mTce.set();
        }

        virtual void onPromiseRejected(PromisePtr promise) override
        {
          auto reason = promise->reason<Any>();
          auto error = Error::CreateIfGeneric(reason);
          mTce.set_exception(error);
        }

      private:
        Concurrency::task_completion_event<void> mTce;
      };

#pragma endregion

    }
    
    Windows::Foundation::DateTime RTCRtpContributingSource::TimeStamp::get()
    {
      return UseHelper::ToCx(_timeStamp);
    }

    Platform::IBox<Platform::Boolean>^ RTCRtpContributingSource::VoiceActivityFlag::get()
    {
      return UseHelper::ToCx(_voiceActivityFlag);
    }

#pragma region RTCRtpReceiver

    RTCRtpReceiver::RTCRtpReceiver(IRTPReceiverPtr receiver) :
      _nativePointer(receiver),
      _nativeDelegatePointer(make_shared<Internal::RTCRtpReceiverDelegate>(this))
    {
      if (_nativePointer)
      {
        _nativeSubscriptionPointer = _nativePointer->subscribe(_nativeDelegatePointer);
      }

    }

    RTCRtpReceiver::RTCRtpReceiver(MediaStreamTrackKind kind, RTCDtlsTransport^ transport) :
      _nativeDelegatePointer(make_shared<Internal::RTCRtpReceiverDelegate>(this))
    {
      auto nativeTransport = RTCDtlsTransport::Convert(transport);

      try
      {
        _nativePointer = IRTPReceiver::create(_nativeDelegatePointer, UseHelper::Convert(kind), nativeTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    RTCRtpReceiver::RTCRtpReceiver(MediaStreamTrackKind kind, RTCSrtpSdesTransport^ transport) :
      _nativeDelegatePointer(make_shared<Internal::RTCRtpReceiverDelegate>(this))
    {
      auto nativeTransport = RTCSrtpSdesTransport::Convert(transport);

      try
      {
        _nativePointer = IRTPReceiver::create(_nativeDelegatePointer, UseHelper::Convert(kind), nativeTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    RTCRtpReceiver::RTCRtpReceiver(MediaStreamTrackKind kind, RTCDtlsTransport ^ transport, RTCDtlsTransport^ rtcpTransport) :
      _nativeDelegatePointer(make_shared<Internal::RTCRtpReceiverDelegate>(this))
    {
      auto nativeTransport = RTCDtlsTransport::Convert(transport);
      auto nativeRtcpTransport = RTCDtlsTransport::Convert(rtcpTransport);
      try
      {
        _nativePointer = IRTPReceiver::create(_nativeDelegatePointer, UseHelper::Convert(kind), nativeTransport, nativeRtcpTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    RTCRtpReceiver::RTCRtpReceiver(MediaStreamTrackKind kind, RTCSrtpSdesTransport ^ transport, RTCIceTransport^ rtcpTransport) :
      _nativeDelegatePointer(make_shared<Internal::RTCRtpReceiverDelegate>(this))
    {
      auto nativeTransport = RTCSrtpSdesTransport::Convert(transport);
      auto nativeRtcpTransport = RTCIceTransport::Convert(rtcpTransport);
      try
      {
        _nativePointer = IRTPReceiver::create(_nativeDelegatePointer, UseHelper::Convert(kind), nativeTransport, nativeRtcpTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    void RTCRtpReceiver::SetTransport(RTCDtlsTransport ^ transport)
    {
      auto nativeTransport = RTCDtlsTransport::Convert(transport);
      try
      {
        _nativePointer->setTransport(nativeTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    void RTCRtpReceiver::SetTransport(RTCSrtpSdesTransport ^ transport)
    {
      auto nativeTransport = RTCSrtpSdesTransport::Convert(transport);
      try
      {
        _nativePointer->setTransport(nativeTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    void RTCRtpReceiver::SetTransport(RTCDtlsTransport ^ transport, RTCDtlsTransport ^ rtcpTransport)
    {
      auto nativeTransport = RTCDtlsTransport::Convert(transport);
      auto nativeRtcpTransport = RTCDtlsTransport::Convert(rtcpTransport);
      try
      {
        _nativePointer->setTransport(nativeTransport, nativeRtcpTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    void RTCRtpReceiver::SetTransport(RTCSrtpSdesTransport ^ transport, RTCIceTransport ^ rtcpTransport)
    {
      auto nativeTransport = RTCSrtpSdesTransport::Convert(transport);
      auto nativeRtcpTransport = RTCIceTransport::Convert(rtcpTransport);
      try
      {
        _nativePointer->setTransport(nativeTransport, nativeRtcpTransport);
      }
      catch (const InvalidParameters &)
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }
    }

    RTCRtpCapabilities^ RTCRtpReceiver::GetCapabilities(Platform::String^ kind)
    {
      ORG_ORTC_THROW_INVALID_STATE_IF(kind == nullptr)

      String kindStr = UseHelper::FromCx(kind);

      if (0 == kindStr.compareNoCase(UseHelper::FromCx(MediaStreamTrackKind::Audio.ToString())))
        return Internal::ToCx(IRTPReceiver::getCapabilities(IMediaStreamTrackTypes::Kinds::Kind_Audio));

      if (0 == kindStr.compareNoCase(UseHelper::FromCx(MediaStreamTrackKind::Video.ToString())))
        return Internal::ToCx(IRTPReceiver::getCapabilities(IMediaStreamTrackTypes::Kinds::Kind_Video));

      ORG_ORTC_THROW_INVALID_PARAMETERS()
    }

    IAsyncAction^ RTCRtpReceiver::Receive(RTCRtpParameters^ parameters)
    {
      ORG_ORTC_THROW_INVALID_STATE_IF(!_nativePointer)
      ORG_ORTC_THROW_INVALID_PARAMETERS_IF(!parameters)

      PromisePtr promise;

      try
      {
        promise = _nativePointer->receive(*Internal::FromCx(parameters));
      }
      catch (const InvalidParameters & )
      {
        ORG_ORTC_THROW_INVALID_PARAMETERS()
      }
      catch (const InvalidStateError &e)
      {
        ORG_ORTC_THROW_INVALID_STATE(UseHelper::ToCx(e.what()))
      }

      IAsyncAction^ ret = Concurrency::create_async([promise]()
      {
        Concurrency::task_completion_event<void> tce;

        auto observer = make_shared<Internal::RTCRtpReceiverPromiseObserver>(tce);

        promise->then(observer);
        promise->background();
        auto tceTask = Concurrency::task<void>(tce);

        return tceTask.get();
      });

      return ret;
    }

    IVector<RTCRtpContributingSource^>^ Org::Ortc::RTCRtpReceiver::GetContributingSource()
    {
      auto ret = ref new Vector<RTCRtpContributingSource^>();

      if (_nativePointer)
      {
        auto contributingSource = _nativePointer->getContributingSources();
        for (IRTPReceiver::ContributingSourceList::iterator it = contributingSource.begin(); it != contributingSource.end(); ++it)
        {
          RTCRtpContributingSource^ source = ref new RTCRtpContributingSource();

          source->_source = it->mCSRC;
          source->_audioLevel = SafeInt<decltype(source->_audioLevel)>(it->mAudioLevel);
          source->_timeStamp = it->mTimestamp;
          source->_voiceActivityFlag = it->mVoiceActivityFlag;
        }
      }

      return ret;
    }

    void RTCRtpReceiver::Stop()
    {
      if (_nativePointer)
        _nativePointer->stop();
    }

    MediaStreamTrack^ RTCRtpReceiver::Track::get()
    {
      if (!_nativePointer) return nullptr;
      return MediaStreamTrack::Convert(_nativePointer->track());
    }

    Platform::Object^ RTCRtpReceiver::Transport::get()
    {
      if (!_nativePointer) return nullptr;
      auto nativeTransport = _nativePointer->transport();
      if (!nativeTransport) return nullptr;
      {
        auto dtlsTransport = IDTLSTransport::convert(nativeTransport);
        if (nullptr != dtlsTransport) {
          return RTCDtlsTransport::Convert(dtlsTransport);
        }
      }
      {
        auto srtpTransport = ISRTPSDESTransport::convert(nativeTransport);
        if (nullptr != srtpTransport) {
          return RTCSrtpSdesTransport::Convert(srtpTransport);
        }
      }
      return nullptr;
    }

    Platform::Object^ RTCRtpReceiver::RtcpTransport::get()
    {
      if (!_nativePointer) return nullptr;
      auto nativeTransport = _nativePointer->rtcpTransport();
      if (!nativeTransport) return nullptr;
      {
        auto dtlsTransport = IDTLSTransport::convert(nativeTransport);
        if (nullptr != dtlsTransport) {
          return RTCDtlsTransport::Convert(dtlsTransport);
        }
      }
      {
        auto iceTransport = IICETransport::convert(nativeTransport);
        if (nullptr != iceTransport) {
          return RTCIceTransport::Convert(iceTransport);
        }
      }
      return nullptr;
    }

#pragma endregion

  } // namespace ortc
} // namespace org
