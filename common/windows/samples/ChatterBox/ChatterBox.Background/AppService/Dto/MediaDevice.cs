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

namespace ChatterBox.Background.AppService.Dto
{
    public sealed class MediaDevice
    {
        [DataMember]
        public string Id { get; set; }

        [DataMember]
        public bool IsPreferred { get; set; }

        [DataMember]
        public string Name { get; set; }

        [DataMember]
        public MediaDeviceLocation Location { get; set;}
    }
}