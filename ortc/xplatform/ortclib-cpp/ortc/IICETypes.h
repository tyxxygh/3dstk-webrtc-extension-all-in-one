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

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes
  #pragma mark
  
  interaction IICETypes
  {
    ZS_DECLARE_STRUCT_PTR(GatherCandidate)
    ZS_DECLARE_STRUCT_PTR(Candidate)
    ZS_DECLARE_STRUCT_PTR(CandidateComplete)
    ZS_DECLARE_TYPEDEF_PTR(std::list<Candidate>, CandidateList)
    ZS_DECLARE_STRUCT_PTR(Parameters)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Roles
    #pragma mark

    enum Roles
    {
      Role_First,

      Role_Controlling    = Role_First,
      Role_Controlled,

      Role_Last           = Role_Controlled,
    };

    static const char *toString(Roles role);
    static Roles toRole(const char *role) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Components
    #pragma mark

    enum Components
    {
      Component_First = 1,

      Component_RTP = Component_First,
      Component_RTCP,

      Component_Last = Component_RTCP
    };

    static const char *toString(Components component);
    static Components toComponent(const char *component) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Protocols
    #pragma mark

    enum Protocols
    {
      Protocol_First,

      Protocol_UDP = Protocol_First,
      Protocol_TCP,

      Protocol_Last = Protocol_TCP
    };

    static const char *toString(Protocols protocol);
    static Protocols toProtocol(const char *protocol) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::CandidateTypes
    #pragma mark

    enum CandidateTypes
    {
      CandidateType_First,

      CandidateType_Host = CandidateType_First,
      CandidateType_Srflex,
      CandidateType_Prflx,
      CandidateType_Relay,

      CandidateType_Last = CandidateType_Relay
    };

    static const char *toString(CandidateTypes type);
    static CandidateTypes toCandidateType(const char *type) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::TCPCandidateTypes
    #pragma mark

    enum TCPCandidateTypes
    {
      TCPCandidateType_First,

      TCPCandidateType_Active = TCPCandidateType_First,
      TCPCandidateType_Passive,
      TCPCandidateType_SO,

      TCPCandidateType_Last = TCPCandidateType_SO
    };

    static const char *toString(TCPCandidateTypes type);
    static TCPCandidateTypes toTCPCandidateType(const char *type) throw (InvalidParameters);

    //-------------------------------------------------------------------------
    struct GatherCandidate
    {
      Components  mComponent {Component_RTP};

      static GatherCandidatePtr create(ElementPtr elem);

      virtual ElementPtr createElement(const char *objectName = NULL) const = 0;

      virtual ~GatherCandidate() {} // make polymorphic

    protected:
      GatherCandidate() {}
      GatherCandidate(const GatherCandidate &op2) :
        mComponent(op2.mComponent)
      {}
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Components
    #pragma mark

    struct Candidate : public GatherCandidate {
      String            mInterfaceType;
      String            mFoundation;
      DWORD             mPriority {};
      DWORD             mUnfreezePriority {};
      Protocols         mProtocol {Protocol_UDP};
      String            mIP;
      WORD              mPort {};
      CandidateTypes    mCandidateType {CandidateType_Host};
      TCPCandidateTypes mTCPType {TCPCandidateType_Active};
      String            mRelatedAddress;
      WORD              mRelatedPort {};

      static CandidatePtr convert(GatherCandidatePtr candidate);

      Candidate();
      Candidate(const Candidate &op2);
      Candidate(ElementPtr elem);
      ~Candidate();

      ElementPtr createElement(const char *objectName = "candidate") const override;

      ElementPtr toDebug() const;
      String hash(bool includePriorities = true) const;

      IPAddress ip() const;
      IPAddress relatedIP() const;
      String foundation(
                        const char *relatedServerURL = NULL,
                        const char *baseIP = NULL
                        ) const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Components
    #pragma mark

    struct CandidateComplete : public GatherCandidate {
      bool        mComplete {true};

      static CandidateCompletePtr convert(GatherCandidatePtr candidate);

      CandidateComplete() {}
      CandidateComplete(const CandidateComplete &op2) {(*this) = op2;}
      CandidateComplete(ElementPtr elem);

      ElementPtr createElement(const char *objectName = "candidateComplete") const override;

      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Parameters
    #pragma mark

    struct Parameters {
      bool mUseUnfreezePriority {false};
      String mUsernameFragment;
      String mPassword;
      bool mICELite {false};

      Parameters() {}
      Parameters(const Parameters &op2) {(*this) = op2;}
      Parameters(ElementPtr rootEl);

      static ParametersPtr create(ElementPtr rootEl) { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };
  };
}
