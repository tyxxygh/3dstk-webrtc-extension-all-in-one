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
using ChatterBox.Communication.Messages.Interfaces;

namespace ChatterBox.Communication.Messages.Peers
{
    public sealed class PeerUpdate : IMessage
    {
        public PeerData PeerData { get; set; }
        public string Id { get; set; } = Guid.NewGuid().ToString();
        public DateTimeOffset SentDateTimeUtc { get; set; }
    }
}