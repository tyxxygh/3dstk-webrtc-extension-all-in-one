#!/bin/bash
baseDir=$(pwd)
echo $baseDir

function collect()
{
	config=$1
	targetDir=dist/linux_x64_$config/lib
	mkdir -p $targetDir
	
	configDir=webrtc/xplatform/webrtc/out/linux_x64_$config
	cp -u $baseDir/$configDir/obj/webrtc/libwebrtc.a $targetDir
	cp -u $baseDir/$configDir/obj/third_party/protobuf/libprotobuf_full.a $targetDir
	cp -u $baseDir/$configDir/obj/webrtc/system_wrappers/libmetrics_default.a $targetDir
	cp -u $baseDir/$configDir/obj/webrtc/system_wrappers/libfield_trial_default.a $targetDir
	cp -u $baseDir/$configDir/obj/webrtc/common_video/libcommon_video.a $targetDir
	#cp -u $baseDir/$configDir/obj/third_party/boringssl/libboringssl_asm.a $targetDir
	cp -u $baseDir/$configDir/obj/third_party/boringssl/libboringssl.a $targetDir

	echo "Collect $config build result into $targetDir"
}

collect release
collect debug
