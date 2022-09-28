/*

Copyright (c) 2016, Robin Raymond
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

#include <zsLib/eventing/tool/internal/zsLib_eventing_tool.h>

namespace zsLib { namespace eventing { namespace tool { ZS_IMPLEMENT_SUBSYSTEM(zslib_eventing_tool) } } }

namespace zsLib
{
  namespace eventing
  {
    namespace tool
    {
      namespace internal
      {
      }

      //-----------------------------------------------------------------------
      tool_ostream &output()
      {
        static tool_ostream gStream;
        return gStream;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StdOutputStream
      #pragma mark

      //-----------------------------------------------------------------------
      void StdOutputStream::output(const char *str) const
      {
        std::cout << str;
      }

      //-----------------------------------------------------------------------
      void StdOutputStream::output(const wchar_t *str) const
      {
        std::cout << str;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DebugOutputStream
      #pragma mark

      //---------------------------------------------------------------------
      void DebugOutputStream::output(const char *str) const
      {
#ifdef _WIN32
        OutputDebugStringA(str);
#endif //_WIN32
      }

      //---------------------------------------------------------------------
      void DebugOutputStream::output(const wchar_t *str) const
      {
#ifdef _WIN32
        OutputDebugStringW(str);
#endif //_WIN32
      }

    } // tool
  } // eventing
} // zsLib
