/*
 
 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/types.h>

#include <set>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities
  #pragma mark
  
  interaction ICapabilities
  {
    ZS_DECLARE_STRUCT_PTR(CapabilityBoolean)
    ZS_DECLARE_STRUCT_PTR(CapabilityLong)
    ZS_DECLARE_STRUCT_PTR(CapabilityDouble)
    ZS_DECLARE_STRUCT_PTR(CapabilityString)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICapabilities::CapabilityBoolean
    #pragma mark

    struct CapabilityBoolean : public std::set<bool> {

      CapabilityBoolean() {}
      CapabilityBoolean(const CapabilityBoolean &op2) {*this = op2;}
      CapabilityBoolean(
                        ElementPtr elem,
                        const char *objectName = "bool"
                        );

      // {"objectName": {"objectValueName": [true,false]}}
      ElementPtr createElement(
                               const char *objectName,
                               const char *objectValueName = "bool"
                               ) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICapabilities::CapabilityLong
    #pragma mark

    struct CapabilityLong {
      LONG mMin {};
      LONG mMax {};

      CapabilityLong() {}
      CapabilityLong(LONG value) :          mMin(value), mMax(value) {}
      CapabilityLong(
                     LONG min,
                     LONG max
                     ) :                    mMin(min), mMax(max) {}
      CapabilityLong(const CapabilityLong &op2) {*this = op2;}

      // {"objectName": {"min": 0, "max": 0}}
      CapabilityLong(ElementPtr elem);

      // {"objectName": {"min": 0, "max": 0}}
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICapabilities::CapabilityDouble
    #pragma mark

    struct CapabilityDouble {
      double mMin {};
      double mMax {};

      CapabilityDouble() {}
      CapabilityDouble(double value) :      mMin(value), mMax(value) {}
      CapabilityDouble(
                       double min,
                       double max
                       ) :                  mMin(min), mMax(max) {}
      CapabilityDouble(const CapabilityDouble &op2) {*this = op2;}

      // {"objectName": {"min": 0.0, "max": 0.0}}
      CapabilityDouble(ElementPtr elem);

      // {"objectName": {"min": 0, "max": 0}}
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICapabilities::CapabilityString
    #pragma mark

    struct CapabilityString : public std::set<String> {

      CapabilityString() {}
      CapabilityString(const CapabilityString &op2) {*this = op2;}
      CapabilityString(
                       ElementPtr elem,
                       const char *objectName = "string"
                       );

      // {"objectName": {"objectValueName": ["a","b","c"]}}
      ElementPtr createElement(
                               const char *objectName,
                               const char *objectValueName = "string"
                               ) const;

      ElementPtr toDebug() const;
      String hash() const;
    };
  };
}
