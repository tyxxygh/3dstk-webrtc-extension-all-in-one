@ECHO off

REM Copyright (c) 2017 The Chromium Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.

REM Run this script to add the current toolchain, as determined by
REM GYP_MSVS_VERSION, DEPOT_TOOLS_WIN_TOOLCHAIN and the hash in vs_toolchain.py,
REM to the path. Be aware of running this multiple times as too-long paths will
REM break things.
REM To get the toolchain for x64 targets pass /x64 to this batch file.

REM Execute whatever is printed by setenv.py.
FOR /f "usebackq tokens=*" %%a in (`python %~dp0setenv.py`) do %%a %1
