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

#pragma once

#ifdef __cplusplus_winrt
#include <Windows.h>
#endif //__cplusplus_winrt

#include <zsLib/types.h>


namespace zsLib
{
  //---------------------------------------------------------------------------
  PUID createPUID();
  UUID createUUID();

  //---------------------------------------------------------------------------
  void debugSetCurrentThreadName(const char *name);

  // see: http://stackoverflow.com/questions/8357240/how-to-automatically-convert-strongly-typed-enum-into-int
  template <typename E>
  constexpr typename std::underlying_type<E>::type to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
  }

  //---------------------------------------------------------------------------
  Time now();

  template <typename duration_type>
  inline duration_type timeSinceEpoch(Time time)
  {
    if (Time() == time) return duration_type();
    return std::chrono::duration_cast<duration_type>(time.time_since_epoch());
  }

  template <typename duration_type>
  inline Time timeSinceEpoch(duration_type duration)
  {
    if (decltype(duration)() == duration) return Time();
    return Time(duration);
  }

  inline Days toDays(const Days &v) {return v;}
  inline Days toDays(const Hours &v) {return std::chrono::duration_cast<Days>(v);}
  inline Days toDays(const Minutes &v) {return std::chrono::duration_cast<Days>(v);}
  inline Days toDays(const Seconds &v) {return std::chrono::duration_cast<Days>(v);}
  inline Days toDays(const Milliseconds &v) {return std::chrono::duration_cast<Days>(v);}
  inline Days toDays(const Microseconds &v) {return std::chrono::duration_cast<Days>(v);}
  inline Days toDays(const Nanoseconds &v) {return std::chrono::duration_cast<Days>(v);}
  inline Days toDays(const Time &v) { return Days(std::chrono::time_point_cast<Days>(v).time_since_epoch()); }

  inline Hours toHours(const Days &v) {return std::chrono::duration_cast<Hours>(v);}
  inline Hours toHours(const Hours &v) {return v;}
  inline Hours toHours(const Minutes &v) {return std::chrono::duration_cast<Hours>(v);}
  inline Hours toHours(const Seconds &v) {return std::chrono::duration_cast<Hours>(v);}
  inline Hours toHours(const Milliseconds &v) {return std::chrono::duration_cast<Hours>(v);}
  inline Hours toHours(const Microseconds &v) {return std::chrono::duration_cast<Hours>(v);}
  inline Hours toHours(const Nanoseconds &v) {return std::chrono::duration_cast<Hours>(v);}
  inline Hours toHours(const Time &v) { return Hours(std::chrono::time_point_cast<Hours>(v).time_since_epoch()); }

  inline Minutes toMinutes(const Days &v) {return std::chrono::duration_cast<Minutes>(v);}
  inline Minutes toMinutes(const Hours &v) {return std::chrono::duration_cast<Minutes>(v);}
  inline Minutes toMinutes(const Minutes &v) {return v;}
  inline Minutes toMinutes(const Seconds &v) {return std::chrono::duration_cast<Minutes>(v);}
  inline Minutes toMinutes(const Milliseconds &v) {return std::chrono::duration_cast<Minutes>(v);}
  inline Minutes toMinutes(const Microseconds &v) {return std::chrono::duration_cast<Minutes>(v);}
  inline Minutes toMinutes(const Nanoseconds &v) {return std::chrono::duration_cast<Minutes>(v);}
  inline Minutes toMinutes(const Time &v) { return Minutes(std::chrono::time_point_cast<Minutes>(v).time_since_epoch()); }

  inline Seconds toSeconds(const Days &v) {return std::chrono::duration_cast<Seconds>(v);}
  inline Seconds toSeconds(const Hours &v) {return std::chrono::duration_cast<Seconds>(v);}
  inline Seconds toSeconds(const Minutes &v) {return std::chrono::duration_cast<Seconds>(v);}
  inline Seconds toSeconds(const Seconds &v) {return v;}
  inline Seconds toSeconds(const Milliseconds &v) {return std::chrono::duration_cast<Seconds>(v);}
  inline Seconds toSeconds(const Microseconds &v) {return std::chrono::duration_cast<Seconds>(v);}
  inline Seconds toSeconds(const Nanoseconds &v) {return std::chrono::duration_cast<Seconds>(v);}
  inline Seconds toSeconds(const Time &v) { return Seconds(std::chrono::time_point_cast<Seconds>(v).time_since_epoch()); }

  inline Milliseconds toMilliseconds(const Days &v) {return std::chrono::duration_cast<Milliseconds>(v);}
  inline Milliseconds toMilliseconds(const Hours &v) {return std::chrono::duration_cast<Milliseconds>(v);}
  inline Milliseconds toMilliseconds(const Minutes &v) {return std::chrono::duration_cast<Milliseconds>(v);}
  inline Milliseconds toMilliseconds(const Seconds &v) {return std::chrono::duration_cast<Milliseconds>(v);}
  inline Milliseconds toMilliseconds(const Milliseconds &v) {return v;}
  inline Milliseconds toMilliseconds(const Microseconds &v) {return std::chrono::duration_cast<Milliseconds>(v);}
  inline Milliseconds toMilliseconds(const Nanoseconds &v) {return std::chrono::duration_cast<Milliseconds>(v);}
  inline Milliseconds toMilliseconds(const Time &v) { return Milliseconds(std::chrono::time_point_cast<Milliseconds>(v).time_since_epoch()); }

  inline Microseconds toMicroseconds(const Days &v) {return std::chrono::duration_cast<Microseconds>(v);}
  inline Microseconds toMicroseconds(const Hours &v) {return std::chrono::duration_cast<Microseconds>(v);}
  inline Microseconds toMicroseconds(const Minutes &v) {return std::chrono::duration_cast<Microseconds>(v);}
  inline Microseconds toMicroseconds(const Seconds &v) {return std::chrono::duration_cast<Microseconds>(v);}
  inline Microseconds toMicroseconds(const Milliseconds &v) {return std::chrono::duration_cast<Microseconds>(v);}
  inline Microseconds toMicroseconds(const Microseconds &v) {return v;}
  inline Microseconds toMicroseconds(const Nanoseconds &v) {return std::chrono::duration_cast<Microseconds>(v);}
  inline Microseconds toMicroseconds(const Time &v) { return Microseconds(std::chrono::time_point_cast<Microseconds>(v).time_since_epoch()); }

  inline Nanoseconds toNanoseconds(const Days &v) {return std::chrono::duration_cast<Nanoseconds>(v);}
  inline Nanoseconds toNanoseconds(const Hours &v) {return std::chrono::duration_cast<Nanoseconds>(v);}
  inline Nanoseconds toNanoseconds(const Minutes &v) {return std::chrono::duration_cast<Nanoseconds>(v);}
  inline Nanoseconds toNanoseconds(const Seconds &v) {return std::chrono::duration_cast<Nanoseconds>(v);}
  inline Nanoseconds toNanoseconds(const Milliseconds &v) {return std::chrono::duration_cast<Nanoseconds>(v);}
  inline Nanoseconds toNanoseconds(const Microseconds &v) {return std::chrono::duration_cast<Nanoseconds>(v);}
  inline Nanoseconds toNanoseconds(const Nanoseconds &v) {return v;}
  inline Nanoseconds toNanoseconds(const Time &v) { return Nanoseconds(std::chrono::time_point_cast<Nanoseconds>(v).time_since_epoch()); }

} // namespace zsLib
