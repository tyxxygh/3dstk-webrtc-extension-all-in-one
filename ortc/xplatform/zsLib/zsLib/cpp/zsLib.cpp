/*

 Copyright (c) 2014, Robin Raymond
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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#include <zsLib/zsLib.h>

#ifndef ZSLIB_EVENTING_NOOP
#include <zsLib/internal/zsLib.events.h>
#else
#include <zsLib/eventing/noop.h>
#endif //ndef ZSLIB_EVENTING_NOOP

namespace zsLib {ZS_IMPLEMENT_SUBSYSTEM(zslib)}
namespace zsLib {ZS_IMPLEMENT_SUBSYSTEM(zslib_socket)}

ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(zsLib, Debug)
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(zsLib_socket, Debug)


namespace zsLib
{
  namespace internal
  {
    void initSubsystems()
    {
      ZS_GET_SUBSYSTEM_LOG_LEVEL(ZS_GET_OTHER_SUBSYSTEM(zsLib, zslib));
      ZS_GET_SUBSYSTEM_LOG_LEVEL(ZS_GET_OTHER_SUBSYSTEM(zsLib, zslib_socket));
    }
  } // namespace internal

  AutoInitializedPUID::AutoInitializedPUID()
  {
    mValue = createPUID();
  }

} // namespace zsLib

ZS_DECLARE_PROXY_IMPLEMENT(zsLib::IWakeDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(zsLib::IPromiseDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(zsLib::ISocketDelegate)
ZS_DECLARE_PROXY_IMPLEMENT(zsLib::ITimerDelegate)
