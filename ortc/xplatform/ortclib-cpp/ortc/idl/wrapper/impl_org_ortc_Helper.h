
#pragma once

#include "types.h"
#include "generated/org_ortc_Json.h"

#include <ortc/IMediaStreamTrack.h>
#include <ortc/IStatsReport.h>
#include <ortc/IICETypes.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IMediaDevices.h>
#include <ortc/adapter/IPeerConnection.h>

#include <zsLib/Log.h>
#include <zsLib/SafeInt.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Helper
        {
          typedef PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > PromiseWithStatsReport;
          ZS_DECLARE_PTR(PromiseWithStatsReport);

          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes, IMediaStreamTrackTypes);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsProvider, IStatsProvider);

          typedef list< ::ortc::IMediaStreamTrackPtr > MediaStreamTrackList;
          ZS_DECLARE_PTR(MediaStreamTrackList);

          typedef list< wrapper::org::ortc::MediaStreamTrackPtr > WrapperMediaStreamTrackList;
          ZS_DECLARE_PTR(WrapperMediaStreamTrackList);

          static zsLib::IMessageQueuePtr getGuiQueue();

          static wrapper::org::ortc::log::Level toWrapper(zsLib::Log::Level level);
          static zsLib::Log::Level toNative(wrapper::org::ortc::log::Level level);

          static const char *toNative(wrapper::org::ortc::log::Component component);

          static wrapper::org::ortc::MediaStreamTrackKind toWrapper(IMediaStreamTrackTypes::Kinds kind);
          static IMediaStreamTrackTypes::Kinds toNative(wrapper::org::ortc::MediaStreamTrackKind kind);

          static wrapper::org::ortc::MediaStreamTrackState toWrapper(IMediaStreamTrackTypes::States state);

          static ::ortc::IMediaDevicesTypes::DeviceKinds toNative(wrapper::org::ortc::MediaDeviceKind kind);
          static wrapper::org::ortc::MediaDeviceKind toWrapper(::ortc::IMediaDevicesTypes::DeviceKinds kind);

          static ::ortc::IStatsReportTypes::StatsTypes toNative(wrapper::org::ortc::RTCStatsType type);
          static wrapper::org::ortc::RTCStatsType toWrapper(::ortc::IStatsReportTypes::StatsTypes type);

          static wrapper::org::ortc::RTCSctpTransportState toWrapper(::ortc::ISCTPTransportTypes::States state);
          static wrapper::org::ortc::RTCDataChannelState toWrapper(::ortc::IDataChannelTypes::States state);

          static wrapper::org::ortc::RTCIceCandidatePairState toWrapper(::ortc::IStatsReportTypes::StatsICECandidatePairStates state);

          static wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingModes mode);
          static ::ortc::adapter::IPeerConnectionTypes::SignalingModes toNative(wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode mode);

          static wrapper::org::ortc::adapter::RTCRtcpMuxPolicy toWrapper(::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicies policy);
          static ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicies toNative(wrapper::org::ortc::adapter::RTCRtcpMuxPolicy policy);

          static wrapper::org::ortc::adapter::RTCBundlePolicy toWrapper(::ortc::adapter::IPeerConnectionTypes::BundlePolicies policy);
          static ::ortc::adapter::IPeerConnectionTypes::BundlePolicies toNative(wrapper::org::ortc::adapter::RTCBundlePolicy policy);

          static wrapper::org::ortc::adapter::RTCSignalingState toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingStates state);
          static wrapper::org::ortc::adapter::RTCIceConnectionState toPeerConnectionWrapper(::ortc::IICETransportTypes::States state);
          static wrapper::org::ortc::adapter::RTCPeerConnectionState toWrapper(::ortc::adapter::IPeerConnectionTypes::PeerConnectionStates state);

          static wrapper::org::ortc::RTCIceGatherFilterPolicy toWrapper(::ortc::IICEGathererTypes::FilterPolicies policy);
          static ::ortc::IICEGathererTypes::FilterPolicies toNative(wrapper::org::ortc::RTCIceGatherFilterPolicy policy);

          static wrapper::org::ortc::RTCIceCredentialType toWrapper(::ortc::IICEGathererTypes::CredentialTypes policy);
          static ::ortc::IICEGathererTypes::CredentialTypes toNative(wrapper::org::ortc::RTCIceCredentialType policy);

          static wrapper::org::ortc::RTCIceGathererState toWrapper(::ortc::IICEGathererTypes::States state);
          static ::ortc::IICEGathererTypes::States toNative(wrapper::org::ortc::RTCIceGathererState state);

          static wrapper::org::ortc::RTCIceComponent toWrapper(::ortc::IICETypes::Components component);
          static wrapper::org::ortc::RTCIceProtocol toWrapper(::ortc::IICETypes::Protocols protocol);
          static wrapper::org::ortc::RTCIceCandidateType toWrapper(::ortc::IICETypes::CandidateTypes type);
          static wrapper::org::ortc::RTCIceTcpCandidateType toWrapper(::ortc::IICETypes::TCPCandidateTypes type);

          static wrapper::org::ortc::RTCIceTransportState toWrapper(::ortc::IICETransportTypes::States state);
          static ::ortc::IICETransportTypes::States toNative(wrapper::org::ortc::RTCIceTransportState state);

          static wrapper::org::ortc::RTCIceRole toWrapper(::ortc::IICETypes::Roles role);
          static ::ortc::IICETypes::Roles toNative(wrapper::org::ortc::RTCIceRole role);

          static wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signals signal);
          static ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signals toNative(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal signal);

          static wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Applications application);
          static ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Applications toNative(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication application);

          static wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP toWrapper(::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToPs top);
          static ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToPs toNative(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP top);

          static wrapper::org::ortc::RTCRtpDegradationPreference toWrapper(::ortc::IRTPTypes::DegradationPreferences preference);
          static ::ortc::IRTPTypes::DegradationPreferences toNative(wrapper::org::ortc::RTCRtpDegradationPreference preference);

          static wrapper::org::ortc::RTCRtpPriorityType toWrapper(::ortc::IRTPTypes::PriorityTypes type);
          static ::ortc::IRTPTypes::PriorityTypes toNative(wrapper::org::ortc::RTCRtpPriorityType type);

          static wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType toWrapper(::ortc::adapter::ISessionDescriptionTypes::SignalingTypes type);
          static ::ortc::adapter::ISessionDescriptionTypes::SignalingTypes toNative(wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type);

          static wrapper::org::ortc::RTCDtlsRole toWrapper(::ortc::IDTLSTransportTypes::Roles role);
          static ::ortc::IDTLSTransportTypes::Roles toNative(wrapper::org::ortc::RTCDtlsRole role);

          static wrapper::org::ortc::RTCDtlsTransportState toWrapper(::ortc::IDTLSTransportTypes::States state);
          static ::ortc::IDTLSTransportTypes::States toNative(wrapper::org::ortc::RTCDtlsTransportState state);

          static wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection toWrapper(::ortc::adapter::ISessionDescriptionTypes::MediaDirections direction);
          static ::ortc::adapter::ISessionDescriptionTypes::MediaDirections toNative(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection direction);

          static WrapperMediaStreamTrackListPtr toWrapper(MediaStreamTrackListPtr tracks);
          static MediaStreamTrackListPtr toNative(WrapperMediaStreamTrackListPtr tracks);

          static PromisePtr toWrapper(PromisePtr promise);
          static void reject(
                             PromisePtr nativePromise,
                             PromisePtr wrapperPromise
                             );

          static PromiseWithStatsReportPtr getStats(IStatsProviderPtr native, wrapper::org::ortc::RTCStatsTypeSetPtr statTypes);

          template <typename optionalType1, typename optionalType2>
          static void optionalSafeIntConvert(const optionalType1 &inputType, optionalType2 &outputType)
          {
            outputType = inputType.hasValue() ? optionalType2(SafeInt<typename optionalType2::UseType>(inputType.value())) : optionalType2();
          }

        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

