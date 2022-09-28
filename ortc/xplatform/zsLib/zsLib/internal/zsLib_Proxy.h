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

#ifndef ZSLIB_INTERNAL_PROXY_H_a1792950ebd2df4b616a6b341965c42d
#define ZSLIB_INTERNAL_PROXY_H_a1792950ebd2df4b616a6b341965c42d

#include <zsLib/types.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Log.h>
#include <zsLib/internal/zsLib_ProxyPack.h>

#define ZS_LIB_NO_CONST
#define ZS_LIB_CONST const

#define ZS_INTERNAL_DECLARE_STUB_PTR(xStub) typedef std::unique_ptr< xStub > xStub##UniPtr;

namespace zsLib
{
  template <typename XINTERFACE>
  class Proxy;

  ZS_DECLARE_FORWARD_SUBSYSTEM(zslib)

  namespace internal
  {
    void proxyCountIncrement(int line, const char *fileName);
    void proxyCountDecrement(int line, const char *fileName);

    template <typename XINTERFACE, bool XDELEGATEMUSTHAVEQUEUE>
    class Proxy : public XINTERFACE
    {
    public:
      ZS_DECLARE_TYPEDEF_PTR(XINTERFACE, Delegate)

    public:
      Proxy(IMessageQueuePtr queue, DelegatePtr delegate, int line, const char *fileName) : mQueue(queue), mDelegate(delegate), mLine(line), mFileName(fileName), mNoop(false), mIgnoreMethodCall(false) {proxyCountIncrement(mLine, mFileName);}
      Proxy(IMessageQueuePtr queue, DelegateWeakPtr delegateWeakPtr, int line, const char *fileName) : mQueue(queue), mWeakDelegate(delegateWeakPtr), mLine(line), mFileName(fileName), mNoop(false), mIgnoreMethodCall(false) {proxyCountIncrement(mLine, mFileName);}
      Proxy(IMessageQueuePtr queue, bool throwsDelegateGone, int line, const char *fileName) : mQueue(queue), mLine(line), mFileName(fileName), mNoop(true), mIgnoreMethodCall(!throwsDelegateGone) {proxyCountIncrement(mLine, mFileName);}
      ~Proxy() {proxyCountDecrement(mLine, mFileName);}

      DelegatePtr getDelegate() const
      {
        if (mDelegate) {
          return mDelegate;
        }
        DelegatePtr result = mWeakDelegate.lock();
        if (!result) {throwDelegateGone();}
        return result;
      }

      DelegatePtr getDelegate(bool throwDelegateGone) const
      {
        if (throwDelegateGone) return getDelegate();
        if (mDelegate) {
          return mDelegate;
        }
        return mWeakDelegate.lock();
      }

      IMessageQueuePtr getQueue() const
      {
        return mQueue;
      }

      bool isNoop() const {return mNoop;}

      bool ignoreMethodCall() const {return mIgnoreMethodCall;}

    protected:
      static const char *getInterfaceName() {return typeid(XINTERFACE).name();}

      virtual void throwDelegateGone() const = 0;

    protected:
      IMessageQueuePtr mQueue;
      DelegatePtr mDelegate;
      DelegateWeakPtr mWeakDelegate;
      int mLine;
      const char *mFileName;
      bool mNoop;
      bool mIgnoreMethodCall;
    };
  }
}

#define ZS_INTERNAL_DECLARE_INTERACTION_PROXY(xInteractionName)                                               \
  interaction xInteractionName;                                                                               \
  typedef std::shared_ptr<xInteractionName> xInteractionName##Ptr;                                            \
  typedef std::weak_ptr<xInteractionName> xInteractionName##WeakPtr;                                          \
  typedef zsLib::Proxy<xInteractionName> xInteractionName##Proxy;

#define ZS_INTERNAL_DECLARE_TYPEDEF_PROXY(xOriginalType, xNewTypeName)                                        \
  typedef xOriginalType xNewTypeName;                                                                         \
  typedef std::shared_ptr<xNewTypeName> xNewTypeName##Ptr;                                                    \
  typedef std::weak_ptr<xNewTypeName> xNewTypeName##WeakPtr;                                                  \
  typedef zsLib::Proxy<xNewTypeName> xNewTypeName##Proxy;

#define ZS_INTERNAL_DECLARE_USING_PROXY(xNamespace, xExistingType)                                            \
  using xNamespace::xExistingType;                                                                            \
  using xNamespace::xExistingType##Ptr;                                                                       \
  using xNamespace::xExistingType##WeakPtr;                                                                   \
  using xNamespace::xExistingType##Proxy;


#define ZS_INTERNAL_DECLARE_PROXY_TYPEDEF(xOriginalType, xTypeAlias)                                          \
    typedef xOriginalType xTypeAlias;

#define ZS_INTERNAL_DECLARE_PROXY_IMPLEMENT(xInterface)                                                       \
  namespace zsLib                                                                                             \
  {                                                                                                           \
    void declareProxyInterface(const xInterface &)                                                            \
    {                                                                                                         \
      typedef std::shared_ptr<xInterface> DelegatePtr;                                                        \
      zsLib::Proxy<xInterface>::create(DelegatePtr());                                                        \
      zsLib::Proxy<xInterface>::create(zsLib::IMessageQueuePtr(), DelegatePtr());                             \
      zsLib::Proxy<xInterface>::createUsingQueue(zsLib::IMessageQueuePtr(), DelegatePtr());                   \
      zsLib::Proxy<xInterface>::createWeak(DelegatePtr());                                                    \
      zsLib::Proxy<xInterface>::createWeak(zsLib::IMessageQueuePtr(), DelegatePtr());                         \
      zsLib::Proxy<xInterface>::createWeakUsingQueue(zsLib::IMessageQueuePtr(), DelegatePtr());               \
      zsLib::Proxy<xInterface>::createNoop(zsLib::IMessageQueuePtr());                                        \
      zsLib::Proxy<xInterface>::isProxy(DelegatePtr());                                                       \
      zsLib::Proxy<xInterface>::original(DelegatePtr());                                                      \
      zsLib::Proxy<xInterface>::getAssociatedMessageQueue(DelegatePtr());                                     \
      zsLib::Proxy<xInterface>::throwMissingMessageQueue();                                                   \
    }                                                                                                         \
  }

#ifndef ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#define ZS_INTERNAL_DECLARE_PROXY_BEGIN(xInterface, xDelegateMustHaveQueue)                                   \
namespace zsLib                                                                                               \
{                                                                                                             \
  template<>                                                                                                  \
  class Proxy<xInterface> : public internal::Proxy<xInterface, xDelegateMustHaveQueue>                        \
  {                                                                                                           \
  public:                                                                                                     \
    struct Exceptions                                                                                         \
    {                                                                                                         \
      ZS_DECLARE_CUSTOM_EXCEPTION_ALT_BASE(DelegateGone, ProxyBase::Exceptions::DelegateGone)                 \
      ZS_DECLARE_CUSTOM_EXCEPTION_ALT_BASE(MissingDelegateMessageQueue, ProxyBase::Exceptions::MissingDelegateMessageQueue) \
    };                                                                                                        \
    ZS_DECLARE_TYPEDEF_PTR(xInterface, Delegate)                                                              \
    ZS_DECLARE_TYPEDEF_PTR(Proxy<xInterface>, ProxyType)                                                      \
                                                                                                              \
  public:                                                                                                     \
    Proxy(IMessageQueuePtr queue, DelegatePtr delegate, int line, const char *fileName);                      \
    Proxy(IMessageQueuePtr queue, DelegateWeakPtr delegate, int line, const char *fileName);                  \
    Proxy(IMessageQueuePtr queue, bool throwsDelegateGone, int line, const char *fileName);                   \
                                                                                                              \
  public:                                                                                                     \
    static DelegatePtr create(DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static DelegatePtr create(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static DelegatePtr createUsingQueue(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static DelegatePtr createWeak(DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static DelegatePtr createWeak(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static DelegatePtr createWeakUsingQueue(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static DelegatePtr createNoop(IMessageQueuePtr queue, bool throwsDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__); \
                                                                                                              \
    static bool isProxy(DelegatePtr delegate);                                                                \
                                                                                                              \
    static DelegatePtr original(DelegatePtr delegate, bool throwDelegateGone = false);                        \
                                                                                                              \
    static IMessageQueuePtr getAssociatedMessageQueue(DelegatePtr delegate);                                  \
                                                                                                              \
    void throwDelegateGone() const override;                                                                  \
                                                                                                              \
    static void throwMissingMessageQueue();                                                                   \

#define ZS_INTERNAL_DECLARE_PROXY_END()                                                                       \
  };                                                                                                          \
}

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_0(xConst, xMethod)                                              \
    void xMethod() xConst override;                                                                           \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_1(xConst, xMethod,t1)                                           \
    void xMethod(t1 v1) xConst override;                                                                      \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_2(xConst, xMethod,t1,t2)                                        \
    void xMethod(t1 v1, t2 v2) xConst override;                                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_3(xConst, xMethod,t1,t2,t3)                                     \
    void xMethod(t1 v1,t2 v2,t3 v3) xConst override;                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_4(xConst, xMethod,t1,t2,t3,t4)                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4) xConst override;                                                    \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_5(xConst, xMethod,t1,t2,t3,t4,t5)                               \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) xConst override;                                              \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_6(xConst, xMethod,t1,t2,t3,t4,t5,t6)                            \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) xConst override;                                        \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_7(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7)                         \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) xConst override;                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_8(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8)                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) xConst override;                            \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_9(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9)                   \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) xConst override;                      \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_10(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10)              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) xConst override;              \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_11(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_12(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12)                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) xConst override;            \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_13(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13)                                                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) xConst override;                                                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_14(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14)                                                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) xConst override;                                                                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_15(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15)                                                                                              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) xConst override;                                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_16(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16)                                                                                          \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) xConst override;                                                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_17(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17)                                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) xConst override;                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_18(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18)                                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) xConst override;                                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_19(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19)                                                                              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) xConst override;                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_20(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20)                                                                          \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) xConst override;                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_21(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21)                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) xConst override;                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_22(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22)                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) xConst override;                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_23(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23)                                                              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_24(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24)                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_25(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24,t25)                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) xConst override;          \


#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_0(xConst, xMethod,r1)                                  \
    r1 xMethod() xConst override;                                                                           \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_1(xConst, xMethod,r1,t1)                               \
    r1 xMethod(t1 v1) xConst override;                                                                      \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_2(xConst, xMethod,r1,t1,t2)                            \
    r1 xMethod(t1 v1, t2 v2) xConst override;                                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_3(xConst, xMethod,r1,t1,t2,t3)                         \
    r1 xMethod(t1 v1,t2 v2,t3 v3) xConst override;                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_4(xConst, xMethod,r1,t1,t2,t3,t4)                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4) xConst override;                                                    \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_5(xConst, xMethod,r1,t1,t2,t3,t4,t5)                   \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) xConst override;                                              \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_6(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6)                \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) xConst override;                                        \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_7(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7)             \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) xConst override;                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_8(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8)          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) xConst override;                            \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_9(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9)       \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) xConst override;                      \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_10(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10)  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) xConst override;              \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_11(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) xConst override;              \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_12(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12)      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_13(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13)                                                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) xConst override;                                                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_14(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14)                                                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) xConst override;                                                                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_15(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15)                                                                                  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) xConst override;                                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_16(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16)                                                                              \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) xConst override;                                                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_17(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17)                                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) xConst override;                                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_18(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18)                                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) xConst override;                                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_19(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19)                                                                  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) xConst override;                                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_20(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20)                                                              \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) xConst override;                                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_21(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21)                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) xConst override;                          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_22(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22)                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) xConst override;                  \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_23(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23)                                                  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_24(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24)                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_25(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24,t25)                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) xConst override;          \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_0(xMethod)                                                                                                 \
    class Stub_0_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
    public:                                                                                                                                         \
      Stub_0_##xMethod(DelegatePtr delegate);                                                                                                       \
      ~Stub_0_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod() override;                                                                                                                        \


#define ZS_INTERNAL_DECLARE_PROXY_METHOD_1(xMethod,t1)                                                                                              \
    class Stub_1_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1;                                                                                                                                        \
    public:                                                                                                                                         \
      Stub_1_##xMethod(DelegatePtr delegate,t1 v1);                                                                                                 \
      ~Stub_1_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1) override;                                                                                                                   \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_2(xMethod,t1,t2)                                                                                           \
    class Stub_2_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2;                                                                                                                                 \
    public:                                                                                                                                         \
      Stub_2_##xMethod(DelegatePtr delegate,t1 v1,t2 v2);                                                                                           \
      ~Stub_2_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2) override;                                                                                                             \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_3(xMethod,t1,t2,t3)                                                                                        \
    class Stub_3_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3;                                                                                                                          \
    public:                                                                                                                                         \
      Stub_3_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3);                                                                                     \
      ~Stub_3_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3) override;                                                                                                       \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_4(xMethod,t1,t2,t3,t4)                                                                                     \
    class Stub_4_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4;                                                                                                                   \
    public:                                                                                                                                         \
      Stub_4_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4);                                                                               \
      ~Stub_4_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4) override;                                                                                                 \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_5(xMethod,t1,t2,t3,t4,t5)                                                                                  \
    class Stub_5_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5;                                                                                                            \
    public:                                                                                                                                         \
      Stub_5_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5);                                                                         \
      ~Stub_5_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) override;                                                                                           \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_6(xMethod,t1,t2,t3,t4,t5,t6)                                                                               \
    class Stub_6_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6;                                                                                                     \
    public:                                                                                                                                         \
      Stub_6_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6);                                                                   \
      ~Stub_6_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) override;                                                                                     \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_7(xMethod,t1,t2,t3,t4,t5,t6,t7)                                                                            \
    class Stub_7_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7;                                                                                              \
    public:                                                                                                                                         \
      Stub_7_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7);                                                             \
      ~Stub_7_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) override;                                                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_8(xMethod,t1,t2,t3,t4,t5,t6,t7,t8)                                                                         \
    class Stub_8_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8;                                                                                       \
    public:                                                                                                                                         \
      Stub_8_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8);                                                       \
      ~Stub_8_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) override;                                                                         \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_9(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9)                                                                      \
    class Stub_9_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9;                                                                                \
    public:                                                                                                                                         \
      Stub_9_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9);                                                 \
      ~Stub_9_##xMethod() override;                                                                                                                 \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) override;                                                                   \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_10(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10)                                                                 \
    class Stub_10_##xMethod : public IMessageQueueMessage                                                                                           \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10;                                                                       \
    public:                                                                                                                                         \
      Stub_10_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10);                                        \
      ~Stub_10_##xMethod() override;                                                                                                                \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) override;                                                           \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_11(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)                                                             \
    class Stub_11_##xMethod : public IMessageQueueMessage                                                                                           \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11;                                                              \
    public:                                                                                                                                         \
      Stub_11_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11);                                \
      ~Stub_11_##xMethod() override;                                                                                                                \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) override;                                                   \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_12(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12)                                                         \
    class Stub_12_##xMethod : public IMessageQueueMessage                                                                                           \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12;                                                     \
    public:                                                                                                                                         \
      Stub_12_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12);                        \
      ~Stub_12_##xMethod() override;                                                                                                                \
                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                 \
      const char *getMethodName() const override;                                                                                                   \
      void processMessage() override;                                                                                                               \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) override;                                           \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_13(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13)                                                                                                                 \
    class Stub_13_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13;                                                                                                        \
    public:                                                                                                                                                                                                     \
      Stub_13_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13);                                                                            \
      ~Stub_13_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) override;                                                                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_14(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14)                                                                                                             \
    class Stub_14_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14;                                                                                               \
    public:                                                                                                                                                                                                     \
      Stub_14_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14);                                                                    \
      ~Stub_14_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) override;                                                                                       \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_15(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15)                                                                                                         \
    class Stub_15_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15;                                                                                      \
    public:                                                                                                                                                                                                     \
      Stub_15_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15);                                                            \
      ~Stub_15_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) override;                                                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_16(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16)                                                                                                     \
    class Stub_16_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16;                                                                             \
    public:                                                                                                                                                                                                     \
      Stub_16_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16);                                                    \
      ~Stub_16_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) override;                                                                       \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_17(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17)                                                                                                 \
    class Stub_17_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17;                                                                    \
    public:                                                                                                                                                                                                     \
      Stub_17_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17);                                            \
      ~Stub_17_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) override;                                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_18(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18)                                                                                             \
    class Stub_18_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18;                                                           \
    public:                                                                                                                                                                                                     \
      Stub_18_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18);                                    \
      ~Stub_18_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) override;                                                       \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_19(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19)                                                                                         \
    class Stub_19_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19;                                                  \
    public:                                                                                                                                                                                                     \
      Stub_19_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19);                            \
      ~Stub_19_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) override;                                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_20(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20)                                                                                     \
    class Stub_20_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20;                                         \
    public:                                                                                                                                                                                                     \
      Stub_20_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20);                    \
      ~Stub_20_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) override;                                       \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_21(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21)                                                                                 \
    class Stub_21_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21;                                \
    public:                                                                                                                                                                                                     \
      Stub_21_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21);            \
      ~Stub_21_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) override;                               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_22(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22)                                                                             \
    class Stub_22_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22;                       \
    public:                                                                                                                                                                                                     \
      Stub_22_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22);    \
      ~Stub_22_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) override;                       \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_23(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23)                                                                         \
    class Stub_23_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23;              \
    public:                                                                                                                                                                                                     \
      Stub_23_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23); \
      ~Stub_23_##xMethod() override;                                                                                                                                                                            \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override;                                                                                                                                                             \
      const char *getMethodName() const override;                                                                                                                                                               \
      void processMessage() override;                                                                                                                                                                           \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) override;               \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_24(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24)                                                                     \
    class Stub_24_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24;     \
    public:                                                                                                                                                                                                     \
      Stub_24_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24); \
      ~Stub_24_##xMethod() override;                                                                                                                                                                                \
                                                                                                                                                                                                                    \
      const char *getDelegateName() const override;                                                                                                                                                                 \
      const char *getMethodName() const override;                                                                                                                                                                   \
      void processMessage() override;                                                                                                                                                                               \
    };                                                                                                                                                                                                              \
                                                                                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) override;           \

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_25(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24,t25)                                                                             \
    class Stub_25_##xMethod : public IMessageQueueMessage                                                                                                                                                                   \
    {                                                                                                                                                                                                                       \
    private:                                                                                                                                                                                                                \
      DelegatePtr mDelegate;                                                                                                                                                                                                \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25;        \
    public:                                                                                                                                                                                                                 \
      Stub_25_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25); \
      ~Stub_25_##xMethod() override;                                                                                                                                                                                        \
                                                                                                                                                                                                                            \
      const char *getDelegateName() const override;                                                                                                                                                                         \
      const char *getMethodName() const override;                                                                                                                                                                           \
      void processMessage() override;                                                                                                                                                                                       \
    };                                                                                                                                                                                                                      \
                                                                                                                                                                                                                            \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) override;           \



#else //ndef ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION



#define ZS_INTERNAL_DECLARE_PROXY_BEGIN(xInterface, xDelegateMustHaveQueue)                                   \
namespace zsLib                                                                                               \
{                                                                                                             \
  template<>                                                                                                  \
  class Proxy<xInterface> : public internal::Proxy<xInterface, xDelegateMustHaveQueue>                        \
  {                                                                                                           \
  public:                                                                                                     \
    struct Exceptions                                                                                         \
    {                                                                                                         \
      ZS_DECLARE_CUSTOM_EXCEPTION_ALT_BASE(DelegateGone, ProxyBase::Exceptions::DelegateGone)                 \
      ZS_DECLARE_CUSTOM_EXCEPTION_ALT_BASE(MissingDelegateMessageQueue, ProxyBase::Exceptions::MissingDelegateMessageQueue) \
    };                                                                                                        \
    ZS_DECLARE_TYPEDEF_PTR(xInterface, Delegate)                                                              \
    ZS_DECLARE_TYPEDEF_PTR(Proxy<xInterface>, ProxyType)                                                      \
                                                                                                              \
  public:                                                                                                    \
    Proxy(IMessageQueuePtr queue, DelegatePtr delegate, int line, const char *fileName) : internal::Proxy<xInterface, xDelegateMustHaveQueue>(queue, delegate, line, fileName) {}     \
    Proxy(IMessageQueuePtr queue, DelegateWeakPtr delegate, int line, const char *fileName) : internal::Proxy<xInterface, xDelegateMustHaveQueue>(queue, delegate, line, fileName) {} \
    Proxy(IMessageQueuePtr queue, bool throwsDelegateGone, int line, const char *fileName) : internal::Proxy<xInterface, xDelegateMustHaveQueue>(queue, throwsDelegateGone, line, fileName) {} \
                                                                                                              \
  public:                                                                                                     \
    static DelegatePtr create(DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__)   \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      IMessageQueuePtr queue;                                                                                 \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        delegate = proxy->getDelegate(throwDelegateGone);                                                     \
        if (!delegate)                                                                                        \
          return delegate;                                                                                    \
        queue = proxy->getQueue();                                                                            \
      }                                                                                                       \
                                                                                                              \
      MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
      if (associator)                                                                                         \
        queue =  associator->getAssociatedMessageQueue();                                                     \
                                                                                                              \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return delegate;                                                                                      \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, delegate, line, fileName);                                         \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr create(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__)   \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        delegate = proxy->getDelegate(throwDelegateGone);                                                     \
        if (!delegate)                                                                                        \
          return delegate;                                                                                    \
        if (!queue)                                                                                           \
          queue = proxy->getQueue();                                                                          \
      }                                                                                                       \
                                                                                                              \
      MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
      if (associator)                                                                                         \
        queue = (associator->getAssociatedMessageQueue() ? associator->getAssociatedMessageQueue() : queue);  \
                                                                                                              \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return delegate;                                                                                      \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, delegate, line, fileName);                                         \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr createUsingQueue(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__)   \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        delegate = proxy->getDelegate(throwDelegateGone);                                                     \
        if (!delegate)                                                                                        \
          return delegate;                                                                                    \
        if (!queue)                                                                                           \
          queue = proxy->getQueue();                                                                          \
      }                                                                                                       \
                                                                                                              \
      if (!queue) {                                                                                           \
        MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
        if (associator)                                                                                         \
          queue = (associator->getAssociatedMessageQueue() ? associator->getAssociatedMessageQueue() : queue);  \
      }                                                                                                       \
                                                                                                              \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return delegate;                                                                                      \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, delegate, line, fileName);                                         \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr createWeak(DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__)                       \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      IMessageQueuePtr queue;                                                                                 \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        delegate = proxy->getDelegate(throwDelegateGone);                                                     \
        if (!delegate)                                                                                        \
          return delegate;                                                                                    \
        queue = proxy->getQueue();                                                                            \
      }                                                                                                       \
                                                                                                              \
      MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
      if (associator)                                                                                         \
        queue =  associator->getAssociatedMessageQueue();                                                     \
                                                                                                              \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return delegate;                                                                                      \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, DelegateWeakPtr(delegate), line, fileName);                        \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr createWeak(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__) \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        delegate = proxy->getDelegate(throwDelegateGone);                                                     \
        if (!delegate)                                                                                        \
          return delegate;                                                                                    \
        if (!queue)                                                                                           \
          queue = proxy->getQueue();                                                                          \
      }                                                                                                       \
                                                                                                              \
      MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
      if (associator)                                                                                         \
        queue = (associator->getAssociatedMessageQueue() ? associator->getAssociatedMessageQueue() : queue);  \
                                                                                                              \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return delegate;                                                                                      \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, DelegateWeakPtr(delegate), line, fileName);                        \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr createWeakUsingQueue(IMessageQueuePtr queue, DelegatePtr delegate, bool throwDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__) \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        delegate = proxy->getDelegate(throwDelegateGone);                                                     \
        if (!delegate)                                                                                        \
          return delegate;                                                                                    \
        if (!queue)                                                                                           \
          queue = proxy->getQueue();                                                                          \
      }                                                                                                       \
                                                                                                              \
      if (!queue) {                                                                                           \
        MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
        if (associator)                                                                                         \
          queue = (associator->getAssociatedMessageQueue() ? associator->getAssociatedMessageQueue() : queue);  \
      }                                                                                                       \
                                                                                                              \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return delegate;                                                                                      \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, DelegateWeakPtr(delegate), line, fileName);                        \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr createNoop(IMessageQueuePtr queue, bool throwsDelegateGone = false, bool overrideDelegateMustHaveQueue = true, int line = __LINE__, const char *fileName = __FILE__) \
    {                                                                                                         \
      if (!queue) {                                                                                           \
        if ((xDelegateMustHaveQueue) && (overrideDelegateMustHaveQueue)) throwMissingMessageQueue();          \
        return DelegatePtr();                                                                                 \
      }                                                                                                       \
                                                                                                              \
      return make_shared<ProxyType>(queue, throwsDelegateGone, line, fileName);                               \
    }                                                                                                         \
                                                                                                              \
    static bool isProxy(DelegatePtr delegate)                                                                 \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return false;                                                                                         \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      return (proxy ? true : false);                                                                          \
    }                                                                                                         \
                                                                                                              \
    static DelegatePtr original(DelegatePtr delegate, bool throwDelegateGone = false)                         \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return delegate;                                                                                      \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        if (proxy->isNoop())                                                                                  \
          return delegate;                                                                                    \
        return proxy->getDelegate(throwDelegateGone);                                                         \
      }                                                                                                       \
      return delegate;                                                                                        \
    }                                                                                                         \
                                                                                                              \
    static IMessageQueuePtr getAssociatedMessageQueue(DelegatePtr delegate)                                   \
    {                                                                                                         \
      if (!delegate)                                                                                          \
        return IMessageQueuePtr();                                                                            \
                                                                                                              \
      ProxyType *proxy = dynamic_cast<ProxyType *>(delegate.get());                                           \
      if (proxy) {                                                                                            \
        return proxy->getQueue();                                                                             \
      }                                                                                                       \
                                                                                                              \
      MessageQueueAssociator *associator = dynamic_cast<MessageQueueAssociator *>(delegate.get());            \
      if (associator)                                                                                         \
        return associator->getAssociatedMessageQueue();                                                       \
                                                                                                              \
      return IMessageQueuePtr();                                                                              \
    }                                                                                                         \
                                                                                                              \
    void throwDelegateGone() const override                                                                   \
    {                                                                                                         \
      throw Exceptions::DelegateGone(ZS_GET_OTHER_SUBSYSTEM(::zsLib, zslib), ::zsLib::Log::Params("proxy points to destroyed delegate", getInterfaceName()), __FUNCTION__, __FILE__, __LINE__); \
    }                                                                                                         \
                                                                                                              \
    static void throwMissingMessageQueue()                                                                    \
    {                                                                                                         \
      throw Exceptions::MissingDelegateMessageQueue(ZS_GET_OTHER_SUBSYSTEM(::zsLib, zslib), ::zsLib::Log::Params("proxy missing message queue", getInterfaceName()), __FUNCTION__, __FILE__, __LINE__); \
    }

#define ZS_INTERNAL_DECLARE_PROXY_END()                                                                       \
  };                                                                                                          \
}

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_0(xConst, xMethod)                                              \
    void xMethod() xConst override {                                                                          \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod();                                                                               \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_1(xConst, xMethod,t1)                                           \
    void xMethod(t1 v1) xConst override {                                                                     \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1);                                                                             \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_2(xConst, xMethod,t1,t2)                                        \
    void xMethod(t1 v1, t2 v2) xConst override {                                                              \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2);                                                                          \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_3(xConst, xMethod,t1,t2,t3)                                     \
    void xMethod(t1 v1,t2 v2,t3 v3) xConst override {                                                         \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3);                                                                       \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_4(xConst, xMethod,t1,t2,t3,t4)                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4) xConst override {                                                   \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4);                                                                    \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_5(xConst, xMethod,t1,t2,t3,t4,t5)                               \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) xConst override {                                             \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4,v5);                                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_6(xConst, xMethod,t1,t2,t3,t4,t5,t6)                            \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) xConst override {                                       \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6);                                                              \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_7(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7)                         \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) xConst override {                                 \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7);                                                           \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_8(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8)                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) xConst override {                           \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8);                                                        \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_9(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9)                   \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) xConst override {                     \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9);                                                     \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_10(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10)              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) xConst override {             \
      if (ignoreMethodCall()) return;                                                                         \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10);                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_11(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) xConst override {         \
      if (ignoreMethodCall()) return;                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11);                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_12(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12)                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) xConst override {           \
      if (ignoreMethodCall()) return;                                                                                       \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12);                                                       \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_13(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13)                                                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) xConst override {                                                                                         \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13);                                                                                                                                         \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_14(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14)                                                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) xConst override {                                                                                 \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14);                                                                                                                                     \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_15(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15)                                                                                              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) xConst override {                                                                         \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15);                                                                                                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_16(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16)                                                                                          \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) xConst override {                                                                 \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16);                                                                                                                             \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_17(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17)                                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) xConst override {                                                         \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17);                                                                                                                         \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_18(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18)                                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) xConst override {                                                 \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18);                                                                                                                     \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_19(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19)                                                                              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) xConst override {                                         \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19);                                                                                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_20(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20)                                                                          \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) xConst override {                                 \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20);                                                                                                             \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_21(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21)                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) xConst override {                         \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21);                                                                                                         \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_22(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22)                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) xConst override {                 \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22);                                                                                                     \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_23(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23)                                                              \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) xConst override {         \
      if (ignoreMethodCall()) return;                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23);                                                                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_24(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24)                                                                  \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) xConst override {         \
      if (ignoreMethodCall()) return;                                                                                                                                                                                     \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24);                                                                                                     \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_25(xConst, xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24,t25)                                                                      \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) xConst override {         \
      if (ignoreMethodCall()) return;                                                                                                                                                                                             \
      getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25);                                                                                                         \
    }


#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_0(xConst, xMethod,r1)                                  \
    r1 xMethod() xConst override {                                                                          \
      return getDelegate()->xMethod();                                                                      \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_1(xConst, xMethod,r1,t1)                               \
    r1 xMethod(t1 v1) xConst override {                                                                     \
      return getDelegate()->xMethod(v1);                                                                    \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_2(xConst, xMethod,r1,t1,t2)                            \
    r1 xMethod(t1 v1, t2 v2) xConst override {                                                              \
      return getDelegate()->xMethod(v1,v2);                                                                 \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_3(xConst, xMethod,r1,t1,t2,t3)                         \
    r1 xMethod(t1 v1,t2 v2,t3 v3) xConst override {                                                         \
      return getDelegate()->xMethod(v1,v2,v3);                                                              \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_4(xConst, xMethod,r1,t1,t2,t3,t4)                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4) xConst override {                                                   \
      return getDelegate()->xMethod(v1,v2,v3,v4);                                                           \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_5(xConst, xMethod,r1,t1,t2,t3,t4,t5)                   \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) xConst override {                                             \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5);                                                        \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_6(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6)                \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) xConst override {                                       \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6);                                                     \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_7(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7)             \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) xConst override {                                 \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7);                                                  \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_8(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8)          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) xConst override {                           \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8);                                               \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_9(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9)       \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) xConst override {                     \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9);                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_10(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10)  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) xConst override {             \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10);                                        \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_11(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) xConst override {             \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11);                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_12(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12)      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) xConst override {         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12);                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_13(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13)                                                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) xConst override {                                                                                         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13);                                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_14(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14)                                                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) xConst override {                                                                                 \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14);                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_15(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15)                                                                                  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) xConst override {                                                                         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15);                                                                                                                        \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_16(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16)                                                                              \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) xConst override {                                                                 \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16);                                                                                                                    \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_17(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17)                                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) xConst override {                                                         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17);                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_18(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18)                                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) xConst override {                                                 \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18);                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_19(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19)                                                                  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) xConst override {                                         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19);                                                                                                        \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_20(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20)                                                              \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) xConst override {                                 \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20);                                                                                                    \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_21(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21)                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) xConst override {                         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21);                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_22(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22)                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) xConst override {                 \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22);                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_23(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23)                                                  \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) xConst override {         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23);                                                                                        \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_24(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24)                                                      \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) xConst override {         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24);                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_SYNC_RETURN_25(xConst, xMethod,r1,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24,t25)                                                          \
    r1 xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) xConst override {         \
      return getDelegate()->xMethod(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25);                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_0(xMethod)                                                                                                 \
    class Stub_0_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
    public:                                                                                                                                         \
      Stub_0_##xMethod(DelegatePtr delegate) : mDelegate(delegate) { }                                                                              \
      ~Stub_0_##xMethod() override { }                                                                                                              \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod();                                                                                                                       \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod() override {                                                                                                                       \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_0_##xMethod)                                                                                                \
      Stub_0_##xMethod##UniPtr stub(new Stub_0_##xMethod(getDelegate()));                                                                           \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_1(xMethod,t1)                                                                                              \
    class Stub_1_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1;                                                                                                                                        \
    public:                                                                                                                                         \
      Stub_1_##xMethod(DelegatePtr delegate,t1 v1) : mDelegate(delegate) {                                                                          \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_1_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1);                                                                                                                     \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1) override {                                                                                                                  \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_1_##xMethod)                                                                                                \
      Stub_1_##xMethod##UniPtr stub(new Stub_1_##xMethod(getDelegate(),v1));                                                                        \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_2(xMethod,t1,t2)                                                                                           \
    class Stub_2_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2;                                                                                                                                 \
    public:                                                                                                                                         \
      Stub_2_##xMethod(DelegatePtr delegate,t1 v1,t2 v2) : mDelegate(delegate) {                                                                    \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_2_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2);                                                                                                                  \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2) override {                                                                                                            \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_2_##xMethod)                                                                                                \
      Stub_2_##xMethod##UniPtr stub(new Stub_2_##xMethod(getDelegate(),v1,v2));                                                                     \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_3(xMethod,t1,t2,t3)                                                                                        \
    class Stub_3_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3;                                                                                                                          \
    public:                                                                                                                                         \
      Stub_3_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3) : mDelegate(delegate) {                                                              \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_3_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3);                                                                                                               \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3) override {                                                                                                      \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_3_##xMethod)                                                                                                \
      Stub_3_##xMethod##UniPtr stub(new Stub_3_##xMethod(getDelegate(),v1,v2,v3));                                                                  \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_4(xMethod,t1,t2,t3,t4)                                                                                     \
    class Stub_4_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4;                                                                                                                   \
    public:                                                                                                                                         \
      Stub_4_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4) : mDelegate(delegate) {                                                        \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_4_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4);                                                                                                            \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4) override {                                                                                                \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_4_##xMethod)                                                                                                \
      Stub_4_##xMethod##UniPtr stub(new Stub_4_##xMethod(getDelegate(),v1,v2,v3,v4));                                                               \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_5(xMethod,t1,t2,t3,t4,t5)                                                                                  \
    class Stub_5_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5;                                                                                                            \
    public:                                                                                                                                         \
      Stub_5_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) : mDelegate(delegate) {                                                  \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_5_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5);                                                                                                         \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5) override {                                                                                          \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_5_##xMethod)                                                                                                \
      Stub_5_##xMethod##UniPtr stub(new Stub_5_##xMethod(getDelegate(),v1,v2,v3,v4,v5));                                                            \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_6(xMethod,t1,t2,t3,t4,t5,t6)                                                                               \
    class Stub_6_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6;                                                                                                     \
    public:                                                                                                                                         \
      Stub_6_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) : mDelegate(delegate) {                                            \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_6_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6);                                                                                                      \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6) override {                                                                                    \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_6_##xMethod)                                                                                                \
      Stub_6_##xMethod##UniPtr stub(new Stub_6_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6));                                                         \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_7(xMethod,t1,t2,t3,t4,t5,t6,t7)                                                                            \
    class Stub_7_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7;                                                                                              \
    public:                                                                                                                                         \
      Stub_7_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) : mDelegate(delegate) {                                      \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_7_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7);                                                                                                   \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7) override {                                                                              \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_7_##xMethod)                                                                                                \
      Stub_7_##xMethod##UniPtr stub(new Stub_7_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7));                                                      \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_8(xMethod,t1,t2,t3,t4,t5,t6,t7,t8)                                                                         \
    class Stub_8_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8;                                                                                       \
    public:                                                                                                                                         \
      Stub_8_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) : mDelegate(delegate) {                                \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                            \
        internal::ProxyPack<t8>(m8, v8);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_8_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                               \
        internal::ProxyClean<t8>(m8);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8);                                                                                                \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8) override {                                                                        \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_8_##xMethod)                                                                                                \
      Stub_8_##xMethod##UniPtr stub(new Stub_8_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8));                                                   \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_9(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9)                                                                      \
    class Stub_9_##xMethod : public IMessageQueueMessage                                                                                            \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9;                                                                                \
    public:                                                                                                                                         \
      Stub_9_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) : mDelegate(delegate) {                          \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                            \
        internal::ProxyPack<t8>(m8, v8);                                                                                                            \
        internal::ProxyPack<t9>(m9, v9);                                                                                                            \
      }                                                                                                                                             \
      ~Stub_9_##xMethod() override {                                                                                                                \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                               \
        internal::ProxyClean<t8>(m8);                                                                                                               \
        internal::ProxyClean<t9>(m9);                                                                                                               \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9);                                                                                             \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9) override {                                                                  \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_9_##xMethod)                                                                                                \
      Stub_9_##xMethod##UniPtr stub(new Stub_9_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9));                                                \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_10(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10)                                                                 \
    class Stub_10_##xMethod : public IMessageQueueMessage                                                                                           \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10;                                                                       \
    public:                                                                                                                                         \
      Stub_10_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) : mDelegate(delegate) {                 \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                            \
        internal::ProxyPack<t8>(m8, v8);                                                                                                            \
        internal::ProxyPack<t9>(m9, v9);                                                                                                            \
        internal::ProxyPack<t10>(m10, v10);                                                                                                         \
      }                                                                                                                                             \
      ~Stub_10_##xMethod() override {                                                                                                               \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                               \
        internal::ProxyClean<t8>(m8);                                                                                                               \
        internal::ProxyClean<t9>(m9);                                                                                                               \
        internal::ProxyClean<t10>(m10);                                                                                                             \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10);                                                                                         \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10) override {                                                          \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_10_##xMethod)                                                                                               \
      Stub_10_##xMethod##UniPtr stub(new Stub_10_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10));                                          \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_11(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)                                                             \
    class Stub_11_##xMethod : public IMessageQueueMessage                                                                                           \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11;                                                              \
    public:                                                                                                                                         \
      Stub_11_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) : mDelegate(delegate) {         \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                            \
        internal::ProxyPack<t8>(m8, v8);                                                                                                            \
        internal::ProxyPack<t9>(m9, v9);                                                                                                            \
        internal::ProxyPack<t10>(m10, v10);                                                                                                         \
        internal::ProxyPack<t11>(m11, v11);                                                                                                         \
      }                                                                                                                                             \
      ~Stub_11_##xMethod() override {                                                                                                               \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                               \
        internal::ProxyClean<t8>(m8);                                                                                                               \
        internal::ProxyClean<t9>(m9);                                                                                                               \
        internal::ProxyClean<t10>(m10);                                                                                                             \
        internal::ProxyClean<t11>(m11);                                                                                                             \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11);                                                                                     \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11) override {                                                  \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_11_##xMethod)                                                                                               \
      Stub_11_##xMethod##UniPtr stub(new Stub_11_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11));                                      \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_12(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12)                                                         \
    class Stub_12_##xMethod : public IMessageQueueMessage                                                                                           \
    {                                                                                                                                               \
    private:                                                                                                                                        \
      DelegatePtr mDelegate;                                                                                                                        \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12;                                                     \
    public:                                                                                                                                         \
      Stub_12_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                            \
        internal::ProxyPack<t8>(m8, v8);                                                                                                            \
        internal::ProxyPack<t9>(m9, v9);                                                                                                            \
        internal::ProxyPack<t10>(m10, v10);                                                                                                         \
        internal::ProxyPack<t11>(m11, v11);                                                                                                         \
        internal::ProxyPack<t12>(m12, v12);                                                                                                         \
      }                                                                                                                                             \
      ~Stub_12_##xMethod() override {                                                                                                               \
        internal::ProxyClean<t1>(m1);                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                               \
        internal::ProxyClean<t8>(m8);                                                                                                               \
        internal::ProxyClean<t9>(m9);                                                                                                               \
        internal::ProxyClean<t10>(m10);                                                                                                             \
        internal::ProxyClean<t11>(m11);                                                                                                             \
        internal::ProxyClean<t12>(m12);                                                                                                             \
      }                                                                                                                                             \
                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                 \
      void processMessage() override {                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12);                                                                                 \
      }                                                                                                                                             \
    };                                                                                                                                              \
                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12) override {                                          \
      if (ignoreMethodCall()) return;                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_12_##xMethod)                                                                                               \
      Stub_12_##xMethod##UniPtr stub(new Stub_12_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12));                                  \
      mQueue->post(std::move(stub));                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_13(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13)                                                                                                                 \
    class Stub_13_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13;                                                                                                        \
    public:                                                                                                                                                                                                     \
      Stub_13_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) : mDelegate(delegate) {                                                     \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_13_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13);                                                                                                                                         \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13) override {                                                                                              \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_13_##xMethod)                                                                                                                                                           \
      Stub_13_##xMethod##UniPtr stub(new Stub_13_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13));                                                                                          \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_14(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14)                                                                                                             \
    class Stub_14_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14;                                                                                               \
    public:                                                                                                                                                                                                     \
      Stub_14_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) : mDelegate(delegate) {                                             \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_14_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14);                                                                                                                                     \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14) override {                                                                                      \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_14_##xMethod)                                                                                                                                                           \
      Stub_14_##xMethod##UniPtr stub(new Stub_14_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14));                                                                                      \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_15(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15)                                                                                                         \
    class Stub_15_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15;                                                                                      \
    public:                                                                                                                                                                                                     \
      Stub_15_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) : mDelegate(delegate) {                                     \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_15_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15);                                                                                                                                 \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15) override {                                                                              \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_15_##xMethod)                                                                                                                                                           \
      Stub_15_##xMethod##UniPtr stub(new Stub_15_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15));                                                                                  \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_16(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16)                                                                                                     \
    class Stub_16_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16;                                                                             \
    public:                                                                                                                                                                                                     \
      Stub_16_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) : mDelegate(delegate) {                             \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_16_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16);                                                                                                                             \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16) override {                                                                      \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_16_##xMethod)                                                                                                                                                           \
      Stub_16_##xMethod##UniPtr stub(new Stub_16_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16));                                                                              \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_17(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17)                                                                                                 \
    class Stub_17_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17;                                                                    \
    public:                                                                                                                                                                                                     \
      Stub_17_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) : mDelegate(delegate) {                     \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_17_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17);                                                                                                                         \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17) override {                                                              \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_17_##xMethod)                                                                                                                                                           \
      Stub_17_##xMethod##UniPtr stub(new Stub_17_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17));                                                                          \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_18(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18)                                                                                             \
    class Stub_18_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18;                                                           \
    public:                                                                                                                                                                                                     \
      Stub_18_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) : mDelegate(delegate) {             \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_18_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18);                                                                                                                     \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18) override {                                                      \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_18_##xMethod)                                                                                                                                                           \
      Stub_18_##xMethod##UniPtr stub(new Stub_18_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18));                                                                      \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_19(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19)                                                                                         \
    class Stub_19_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19;                                                  \
    public:                                                                                                                                                                                                     \
      Stub_19_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) : mDelegate(delegate) {     \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                     \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_19_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                         \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19);                                                                                                                 \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19) override {                                              \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_19_##xMethod)                                                                                                                                                           \
      Stub_19_##xMethod##UniPtr stub(new Stub_19_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19));                                                                  \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_20(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20)                                                                                     \
    class Stub_20_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20;                                         \
    public:                                                                                                                                                                                                     \
      Stub_20_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                     \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                     \
        internal::ProxyPack<t20>(m20, v20);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_20_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                         \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                         \
        internal::ProxyClean<t20>(m20);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19,m20);                                                                                                             \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20) override {                                      \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_20_##xMethod)                                                                                                                                                           \
      Stub_20_##xMethod##UniPtr stub(new Stub_20_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20));                                                              \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_21(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21)                                                                                 \
    class Stub_21_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21;                                \
    public:                                                                                                                                                                                                     \
      Stub_21_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                     \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                     \
        internal::ProxyPack<t20>(m20, v20);                                                                                                                                                                     \
        internal::ProxyPack<t21>(m21, v21);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_21_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                         \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                         \
        internal::ProxyClean<t20>(m20);                                                                                                                                                                         \
        internal::ProxyClean<t21>(m21);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19,m20,m21);                                                                                                         \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21) override {                              \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_21_##xMethod)                                                                                                                                                           \
      Stub_21_##xMethod##UniPtr stub(new Stub_21_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21));                                                          \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_22(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22)                                                                             \
    class Stub_22_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22;                       \
    public:                                                                                                                                                                                                     \
      Stub_22_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                     \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                     \
        internal::ProxyPack<t20>(m20, v20);                                                                                                                                                                     \
        internal::ProxyPack<t21>(m21, v21);                                                                                                                                                                     \
        internal::ProxyPack<t22>(m22, v22);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_22_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                         \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                         \
        internal::ProxyClean<t20>(m20);                                                                                                                                                                         \
        internal::ProxyClean<t21>(m21);                                                                                                                                                                         \
        internal::ProxyClean<t22>(m22);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19,m20,m21,m22);                                                                                                     \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22) override {                      \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_22_##xMethod)                                                                                                                                                           \
      Stub_22_##xMethod##UniPtr stub(new Stub_22_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22));                                                      \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_23(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23)                                                                         \
    class Stub_23_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23;              \
    public:                                                                                                                                                                                                     \
      Stub_23_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                        \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                        \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                        \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                        \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                        \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                        \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                        \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                        \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                        \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                     \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                     \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                     \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                     \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                     \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                     \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                     \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                     \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                     \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                     \
        internal::ProxyPack<t20>(m20, v20);                                                                                                                                                                     \
        internal::ProxyPack<t21>(m21, v21);                                                                                                                                                                     \
        internal::ProxyPack<t22>(m22, v22);                                                                                                                                                                     \
        internal::ProxyPack<t23>(m23, v23);                                                                                                                                                                     \
      }                                                                                                                                                                                                         \
      ~Stub_23_##xMethod() override {                                                                                                                                                                           \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                           \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                           \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                           \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                           \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                           \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                           \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                           \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                           \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                           \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                         \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                         \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                         \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                         \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                         \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                         \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                         \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                         \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                         \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                         \
        internal::ProxyClean<t20>(m20);                                                                                                                                                                         \
        internal::ProxyClean<t21>(m21);                                                                                                                                                                         \
        internal::ProxyClean<t22>(m22);                                                                                                                                                                         \
        internal::ProxyClean<t23>(m23);                                                                                                                                                                         \
      }                                                                                                                                                                                                         \
                                                                                                                                                                                                                \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                            \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                             \
      void processMessage() override {                                                                                                                                                                          \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19,m20,m21,m22,m23);                                                                                                 \
      }                                                                                                                                                                                                         \
    };                                                                                                                                                                                                          \
                                                                                                                                                                                                                \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23) override {              \
      if (ignoreMethodCall()) return;                                                                                                                                                                           \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_23_##xMethod)                                                                                                                                                           \
      Stub_23_##xMethod##UniPtr stub(new Stub_23_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23));                                                  \
      mQueue->post(std::move(stub));                                                                                                                                                                            \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_24(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24)                                                                     \
    class Stub_24_##xMethod : public IMessageQueueMessage                                                                                                                                                       \
    {                                                                                                                                                                                                           \
    private:                                                                                                                                                                                                    \
      DelegatePtr mDelegate;                                                                                                                                                                                    \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24;     \
    public:                                                                                                                                                                                                     \
      Stub_24_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                            \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                            \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                            \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                            \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                            \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                            \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                            \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                            \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                            \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                         \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                         \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                         \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                         \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                         \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                         \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                         \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                         \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                         \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                         \
        internal::ProxyPack<t20>(m20, v20);                                                                                                                                                                         \
        internal::ProxyPack<t21>(m21, v21);                                                                                                                                                                         \
        internal::ProxyPack<t22>(m22, v22);                                                                                                                                                                         \
        internal::ProxyPack<t23>(m23, v23);                                                                                                                                                                         \
        internal::ProxyPack<t24>(m24, v24);                                                                                                                                                                         \
      }                                                                                                                                                                                                             \
      ~Stub_24_##xMethod() override {                                                                                                                                                                               \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                               \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                               \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                               \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                               \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                               \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                               \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                               \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                               \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                               \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                             \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                             \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                             \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                             \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                             \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                             \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                             \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                             \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                             \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                             \
        internal::ProxyClean<t20>(m20);                                                                                                                                                                             \
        internal::ProxyClean<t21>(m21);                                                                                                                                                                             \
        internal::ProxyClean<t22>(m22);                                                                                                                                                                             \
        internal::ProxyClean<t23>(m23);                                                                                                                                                                             \
        internal::ProxyClean<t24>(m24);                                                                                                                                                                             \
      }                                                                                                                                                                                                             \
                                                                                                                                                                                                                    \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                                \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                                 \
      void processMessage() override {                                                                                                                                                                              \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19,m20,m21,m22,m23,m24);                                                                                                 \
      }                                                                                                                                                                                                             \
    };                                                                                                                                                                                                              \
                                                                                                                                                                                                                    \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24) override {          \
      if (ignoreMethodCall()) return;                                                                                                                                                                               \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_24_##xMethod)                                                                                                                                                               \
      Stub_24_##xMethod##UniPtr stub(new Stub_24_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24));                                                  \
      mQueue->post(std::move(stub));                                                                                                                                                                                \
    }

#define ZS_INTERNAL_DECLARE_PROXY_METHOD_25(xMethod,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14,t15,t16,t17,t18,t19,t20,t21,t22,t23,t24,t25)                                                                             \
    class Stub_25_##xMethod : public IMessageQueueMessage                                                                                                                                                                   \
    {                                                                                                                                                                                                                       \
    private:                                                                                                                                                                                                                \
      DelegatePtr mDelegate;                                                                                                                                                                                                \
      t1 m1; t2 m2; t3 m3; t4 m4; t5 m5; t6 m6; t7 m7; t8 m8; t9 m9; t10 m10; t11 m11; t12 m12; t13 m13; t14 m14; t15 m15; t16 m16; t17 m17; t18 m18; t19 m19; t20 m20; t21 m21; t22 m22; t23 m23; t24 m24; t25 m25;        \
    public:                                                                                                                                                                                                                 \
      Stub_25_##xMethod(DelegatePtr delegate,t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) : mDelegate(delegate) { \
        internal::ProxyPack<t1>(m1, v1);                                                                                                                                                                                    \
        internal::ProxyPack<t2>(m2, v2);                                                                                                                                                                                    \
        internal::ProxyPack<t3>(m3, v3);                                                                                                                                                                                    \
        internal::ProxyPack<t4>(m4, v4);                                                                                                                                                                                    \
        internal::ProxyPack<t5>(m5, v5);                                                                                                                                                                                    \
        internal::ProxyPack<t6>(m6, v6);                                                                                                                                                                                    \
        internal::ProxyPack<t7>(m7, v7);                                                                                                                                                                                    \
        internal::ProxyPack<t8>(m8, v8);                                                                                                                                                                                    \
        internal::ProxyPack<t9>(m9, v9);                                                                                                                                                                                    \
        internal::ProxyPack<t10>(m10, v10);                                                                                                                                                                                 \
        internal::ProxyPack<t11>(m11, v11);                                                                                                                                                                                 \
        internal::ProxyPack<t12>(m12, v12);                                                                                                                                                                                 \
        internal::ProxyPack<t13>(m13, v13);                                                                                                                                                                                 \
        internal::ProxyPack<t14>(m14, v14);                                                                                                                                                                                 \
        internal::ProxyPack<t15>(m15, v15);                                                                                                                                                                                 \
        internal::ProxyPack<t16>(m16, v16);                                                                                                                                                                                 \
        internal::ProxyPack<t17>(m17, v17);                                                                                                                                                                                 \
        internal::ProxyPack<t18>(m18, v18);                                                                                                                                                                                 \
        internal::ProxyPack<t19>(m19, v19);                                                                                                                                                                                 \
        internal::ProxyPack<t20>(m20, v20);                                                                                                                                                                                 \
        internal::ProxyPack<t21>(m21, v21);                                                                                                                                                                                 \
        internal::ProxyPack<t22>(m22, v22);                                                                                                                                                                                 \
        internal::ProxyPack<t23>(m23, v23);                                                                                                                                                                                 \
        internal::ProxyPack<t24>(m24, v24);                                                                                                                                                                                 \
        internal::ProxyPack<t25>(m25, v25);                                                                                                                                                                                 \
      }                                                                                                                                                                                                                     \
      ~Stub_25_##xMethod() override {                                                                                                                                                                                       \
        internal::ProxyClean<t1>(m1);                                                                                                                                                                                       \
        internal::ProxyClean<t2>(m2);                                                                                                                                                                                       \
        internal::ProxyClean<t3>(m3);                                                                                                                                                                                       \
        internal::ProxyClean<t4>(m4);                                                                                                                                                                                       \
        internal::ProxyClean<t5>(m5);                                                                                                                                                                                       \
        internal::ProxyClean<t6>(m6);                                                                                                                                                                                       \
        internal::ProxyClean<t7>(m7);                                                                                                                                                                                       \
        internal::ProxyClean<t8>(m8);                                                                                                                                                                                       \
        internal::ProxyClean<t9>(m9);                                                                                                                                                                                       \
        internal::ProxyClean<t10>(m10);                                                                                                                                                                                     \
        internal::ProxyClean<t11>(m11);                                                                                                                                                                                     \
        internal::ProxyClean<t12>(m12);                                                                                                                                                                                     \
        internal::ProxyClean<t13>(m13);                                                                                                                                                                                     \
        internal::ProxyClean<t14>(m14);                                                                                                                                                                                     \
        internal::ProxyClean<t15>(m15);                                                                                                                                                                                     \
        internal::ProxyClean<t16>(m16);                                                                                                                                                                                     \
        internal::ProxyClean<t17>(m17);                                                                                                                                                                                     \
        internal::ProxyClean<t18>(m18);                                                                                                                                                                                     \
        internal::ProxyClean<t19>(m19);                                                                                                                                                                                     \
        internal::ProxyClean<t20>(m20);                                                                                                                                                                                     \
        internal::ProxyClean<t21>(m21);                                                                                                                                                                                     \
        internal::ProxyClean<t22>(m22);                                                                                                                                                                                     \
        internal::ProxyClean<t23>(m23);                                                                                                                                                                                     \
        internal::ProxyClean<t24>(m24);                                                                                                                                                                                     \
        internal::ProxyClean<t25>(m25);                                                                                                                                                                                     \
      }                                                                                                                                                                                                                     \
                                                                                                                                                                                                                            \
      const char *getDelegateName() const override {return typeid(Delegate).name();}                                                                                                                                        \
      const char *getMethodName() const override {return #xMethod;}                                                                                                                                                         \
      void processMessage() override {                                                                                                                                                                                      \
        mDelegate->xMethod(m1,m2,m3,m4,m5,m6,m7,m8,m9,m10,m11,m12,m13,m14,m15,m16,m17,m18,m19,m20,m21,m22,m23,m24,m25);                                                                                                     \
      }                                                                                                                                                                                                                     \
    };                                                                                                                                                                                                                      \
                                                                                                                                                                                                                            \
    void xMethod(t1 v1,t2 v2,t3 v3,t4 v4,t5 v5,t6 v6,t7 v7,t8 v8,t9 v9,t10 v10,t11 v11,t12 v12,t13 v13,t14 v14,t15 v15,t16 v16,t17 v17,t18 v18,t19 v19,t20 v20,t21 v21,t22 v22,t23 v23,t24 v24,t25 v25) override {          \
      if (ignoreMethodCall()) return;                                                                                                                                                                                       \
      ZS_INTERNAL_DECLARE_STUB_PTR(Stub_25_##xMethod)                                                                                                                                                                       \
      Stub_25_##xMethod##UniPtr stub(new Stub_25_##xMethod(getDelegate(),v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,v17,v18,v19,v20,v21,v22,v23,v24,v25));                                                      \
      mQueue->post(std::move(stub));                                                                                                                                                                                        \
    }

#endif //ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#endif //ZSLIB_INTERNAL_PROXY_H_a1792950ebd2df4b616a6b341965c42d
