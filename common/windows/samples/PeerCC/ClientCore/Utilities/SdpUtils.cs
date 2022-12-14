//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
#if ORTCLIB
using Org.Ortc;
using CodecInfo= Org.Ortc.RTCRtpCodecCapability;
#else
using Org.WebRtc;
#endif

namespace PeerConnectionClient.Utilities
{
    /// <summary>
    /// Utility class to organize SDP negotiation.
    /// </summary>
    class SdpUtils
    {
        /// <summary>
        /// Forces the SDP to use the selected audio and video codecs.
        /// </summary>
        /// <param name="sdp">Session description.</param>
        /// <param name="audioCodec">Audio codec.</param>
        /// <param name="videoCodec">Video codec.</param>
        /// <returns>True if succeeds to force to use the selected audio/video codecs.</returns>
        public static bool SelectCodecs(ref string sdp, CodecInfo audioCodec, CodecInfo videoCodec)
        {
            string audioCodecName = audioCodec?.Name ?? null;
            string videoCodecName = videoCodec?.Name ?? null;

            if(audioCodecName != null)
            {
                MaybePreferCodec(ref sdp, "audio", "receive", audioCodecName);
                MaybePreferCodec(ref sdp, "audio", "send", audioCodecName);
            }

            if(videoCodecName != null)
            {
                MaybePreferCodec(ref sdp, "video", "receive", videoCodecName);
                MaybePreferCodec(ref sdp, "video", "send", videoCodecName);
            }

            return true;
        }

        // Sets |codec| as the default |type| codec if it's present.
        // The format of |codec| is 'NAME/RATE', e.g. 'opus/48000'.
        private static bool MaybePreferCodec(ref string sdp, string type, string dir, string codec)
        {
            string  str = type + " " + dir + " codec";

            string[] sdpLines = sdp.Split(new string[] { "\r\n"}, StringSplitOptions.None);

            // Search for m line.
            var mLineIndex = FindLine(sdpLines, "m=", type);
            if (mLineIndex == -1)
            {
                return false;
            }

            // If the codec is available, set it as the default in m line.
            string payload = null;
            // Iterate through rtpmap enumerations to find all matching codec entries
            for (int i = sdpLines.Length - 1; i >= 0; i--)
            {
                // Finds first match in rtpmap
                int index = FindLineInRange(sdpLines, i, 0, "a=rtpmap", codec, 1);
                if (index != -1)
                {
                    // Skip all of the entries between i and index match
                    i = index;
                    payload = GetCodecPayloadTypeFromLine(sdpLines[index]);
                    if (payload != null)
                    {
                        // Move codec to top
                        sdpLines[mLineIndex] = SetDefaultCodec(sdpLines[mLineIndex], payload);
                    }
                }
                else
                {
                    // No match means we can break the loop
                    break;
                }
            }
            
            sdp = string.Join("\r\n",sdpLines);
            return true;
        }

        // Find the line in sdpLines that starts with |prefix|, and, if specified,
        // contains |substr| (case-insensitive search).
        private static int FindLine(string[] sdpLines, string prefix, string substr)
        {
            return FindLineInRange(sdpLines, 0, -1, prefix, substr);
        }

        // Find the line in sdpLines[startLine...endLine - 1] that starts with |prefix|
        // and, if specified, contains |substr| (case-insensitive search).
        private static int FindLineInRange(string [] sdpLines, int startLine, int endLine, string prefix, string substr, int direction = 0) {
            if (direction != 0)
            {
                direction = 1;
            }
            
            if (direction == 0)
            {
                // Search beginning to end
                int realEndLine = endLine != -1 ? endLine : sdpLines.Length;
                for (int i = startLine; i < realEndLine; i++)
                {
                    if (sdpLines[i].StartsWith(prefix))
                    {
                        if (substr == "" || sdpLines[i].ToLower().Contains(substr.ToLower()))
                        {
                            return i;
                        }
                    }
                }
            }
            else
            {
                // Search end to beginning
                var realStartLine = startLine != -1 ? startLine : sdpLines.Length - 1;
                for (var j = realStartLine; j >= 0; --j)
                {
                    if (sdpLines[j].StartsWith(prefix))
                    {
                        if (substr == "" || sdpLines[j].ToLower().Contains(substr.ToLower()))
                        {
                            return j;
                        }
                    }
                }
            }
            return -1;
        }

        // Gets the codec payload type from an a=rtpmap:X line.
        private static string GetCodecPayloadTypeFromLine(string sdpLine)
        {
            Regex pattern = new Regex("a=rtpmap:(\\d+) [a-zA-Z0-9-]+\\/\\d+");
            Match result = pattern.Match(sdpLine);
            return (result.Success) ? result.Groups[1].Value : null;
        }

        // Returns a new m= line with the specified codec as the first one.
        private static string SetDefaultCodec(string mLine, string payload)
        {
            List<string> elements = new List<string>(mLine.Split(' '));

            // Just copy the first three parameters; codec order starts on fourth.
            List<string> newLine = elements.GetRange(0, 3);

            // Put target payload first and copy in the rest.
            newLine.Add(payload);
            elements.GetRange(3, elements.Count-3).ForEach(
            delegate (String element)
            {
                if(!element.Equals(payload))
                {
                    newLine.Add(element);
                }
            });

            return String.Join(" ", newLine);
        }
    }
}
