/*

 Copyright (c) 2014, Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#pragma once

#include <ortc/services/types.h>

#include <zsLib/Log.h>

namespace ortc
{
  namespace services
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ILogger
    #pragma mark

    interaction ILogger
    {
      static void installStdOutLogger(bool colorizeOutput);
      static void installFileLogger(const char *fileName, bool colorizeOutput);
      static void installTelnetLogger(
                                      WORD listenPort,
                                      Seconds maxSecondsWaitForSocketToBeAvailable,
                                      bool colorizeOutput
                                      );
      static void installOutgoingTelnetLogger(
                                              const char *serverHostWithPort,
                                              bool colorizeOutput,
                                              const char *sendStringUponConnection
                                              );
      static void installDebuggerLogger(bool colorizeOutput = false);

      static bool isTelnetLoggerListening();
      static bool isTelnetLoggerConnected();
      static bool isOutgoingTelnetLoggerConnected();

      static void uninstallStdOutLogger();
      static void uninstallFileLogger();
      static void uninstallTelnetLogger();
      static void uninstallOutgoingTelnetLogger();
      static void uninstallDebuggerLogger();

      static void setLogLevel(Log::Level logLevel);
      static void setLogLevel(const char *component, Log::Level logLevel);

      static void installEventingListener(
        WORD listenPort,
        const char *sharedSecret,
        Seconds maxSecondsWaitForSocketToBeAvailable
      );
      static void uninstallEventingListener();

      static void connectToEventingServer(
        const char *serverAddress,
        const char *sharedSecret
      );
      static void disconnectEventingServer();

      static void setDefaultEventingLevel(Log::Level logLevel);
      static void setDefaultEventingLevel(const char *component, Log::Level logLevel);
      static void setEventingLevel(Log::Level logLevel);
      static void setEventingLevel(const char *component, Log::Level logLevel);
    };
  }
}
