/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef THIRD_PARTY_H264_WINUWP_UTILS_UTILS_H_
#define THIRD_PARTY_H264_WINUWP_UTILS_UTILS_H_

//#define ON_SUCCEEDED(act) SUCCEEDED(hr) && SUCCEEDED(hr = act)
#define ON_SUCCEEDED(act) if (SUCCEEDED(hr)) { hr = act; if (FAILED(hr)) { LOG(LS_WARNING) << "ERROR:" << #act; } }

#endif  // THIRD_PARTY_H264_WINUWP_UTILS_UTILS_H_
