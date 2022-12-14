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

namespace ChatterBox.Background.AppService.Dto
{
    public sealed class FrameFormat
    {
        public uint ForegroundProcessId { get; set; }

        public uint Height { get; set; }
        public bool IsLocal { get; set; }

        public long SwapChainHandle { get; set; }

        public uint Width { get; set; }
    }
}