/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*
*/

#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_ENCODER_IMPL_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_ENCODER_IMPL_H_

#include <memory>
#include <vector>

#ifdef _WIN32
#else
#include <unistd.h>
#include <limits.h>
#endif

#include "webrtc/common_video/h264/h264_bitstream_parser.h"
#include "webrtc/modules/video_coding/codecs/h264/include/h264.h"
#include "webrtc/modules/video_coding/utility/quality_scaler.h"

#include "webrtc/modules/video_coding/utility/quality_scaler.h"
#include "webrtc/rtc_base/json.h"
#include "third_party/openh264/src/codec/api/svc/codec_app_def.h"

#include "third_party/nvpipe/nvEncodeAPI.h"
#include "third_party/nvpipe/dynlink_cuda.h"
#include "third_party/nvpipe/nvpipe.h"

class ISVCEncoder;

namespace webrtc {

	//static std::string ExePath(std::string fileName);

	class H264EncoderImpl : public H264Encoder {
	public:
		explicit H264EncoderImpl(const cricket::VideoCodec& codec);
		~H264EncoderImpl() override;

		// |max_payload_size| is ignored.
		// The following members of |codec_settings| are used. The rest are ignored.
		// - codecType (must be kVideoCodecH264)
		// - targetBitrate
		// - maxFramerate
		// - width
		// - height
		int32_t InitEncode(const VideoCodec* codec_settings,
			int32_t number_of_cores,
			size_t max_payload_size) override;
		int32_t Release() override;

		int32_t RegisterEncodeCompleteCallback(
			EncodedImageCallback* callback) override;
		int32_t SetRateAllocation(const BitrateAllocation& bitrate_allocation,
			uint32_t framerate) override;

		// The result of encoding - an EncodedImage and RTPFragmentationHeader - are
		// passed to the encode complete callback.
		int32_t Encode(const VideoFrame& frame,
			const CodecSpecificInfo* codec_specific_info,
			const std::vector<FrameType>* frame_types) override;

		const char* ImplementationName() const override;

		VideoEncoder::ScalingSettings GetScalingSettings() const override;
		static NVENCSTATUS CheckDeviceNVENCCapability();

		// Unsupported / Do nothing.
		int32_t SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;
		int32_t SetPeriodicKeyFrames(bool enable) override;

		// Exposed for testing.
		H264PacketizationMode PacketizationModeForTesting() const {
			return packetization_mode_;
		}

	private:
		bool IsInitialized() const;
		SEncParamExt CreateEncoderParams() const;
		void GetDefaultNvencodeConfig(EncodeConfig &nvEncodeConfig, Json::Value rootValue);

		webrtc::H264BitstreamParser h264_bitstream_parser_;
		// Reports statistics with histograms.
		void ReportInit();
		void ReportError();

		//Hw encoder
		EncodeConfig				m_encodeConfig;

		#ifdef _WIN32
		// Nv pipe
		HINSTANCE hGetProcIDDLL;
		#else
		void* m_hProcHandle = nullptr;
		#endif
		nvpipe* m_pNvPipeEncoder;
		uint8_t* pFrameBuffer;
		size_t bufferSize;
		nvpipe_create_encoder create_nvpipe_encoder;
		nvpipe_destroy destroy_nvpipe_encoder;
		nvpipe_encode encode_nvpipe;
		nvpipe_bitrate reconfigure_nvpipe;

		ISVCEncoder* encoder_;
		// Settings that are used by this encoder.
		int width_;
		int height_;
		float max_frame_rate_;
		uint32_t target_bps_;
		uint32_t max_bps_;
		VideoCodecMode mode_;
		// H.264 specifc parameters
		bool frame_dropping_on_;
		int key_frame_interval_;
		H264PacketizationMode packetization_mode_;

		size_t max_payload_size_;
		int32_t number_of_cores_;

		bool m_use_software_encoding;
		bool m_first_frame_sent;

		EncodedImage encoded_image_;
		std::unique_ptr<uint8_t[]> encoded_image_buffer_;
		EncodedImageCallback* encoded_image_callback_;
		int64_t last_prediction_timestamp_;

		bool has_reported_init_;
		bool has_reported_error_;
	};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CODING_CODECS_H264_H264_ENCODER_IMPL_H_
