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

using System.Runtime.Serialization;

namespace ChatterBox.Background.Signaling.Dto
{
    public sealed class DtoIceCandidate
    {
        [DataMember]
        public string Candidate { get; set; }

        [DataMember]
        public string SdpMid { get; set; }

        [DataMember]
        public ushort SdpMLineIndex { get; set; }
    }
}