
#include "impl_org_ortc_RTCRtcpTransport.h"
#include "impl_org_ortc_RTCDtlsTransport.h"
#include "impl_org_ortc_RTCIceTransport.h"

using ::zsLib::String;
using ::zsLib::Optional;
using ::zsLib::Any;
using ::zsLib::AnyPtr;
using ::zsLib::AnyHolder;
using ::zsLib::Promise;
using ::zsLib::PromisePtr;
using ::zsLib::PromiseWithHolder;
using ::zsLib::PromiseWithHolderPtr;
using ::zsLib::eventing::SecureByteBlock;
using ::zsLib::eventing::SecureByteBlockPtr;
using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::make_shared;
using ::std::list;
using ::std::set;
using ::std::map;


ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtcpTransport::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtcpTransport::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCRtcpTransport::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtcpTransport::RTCRtcpTransport()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtcpTransportPtr wrapper::org::ortc::RTCRtcpTransport::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCRtcpTransport>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCRtcpTransport::~RTCRtcpTransport()
{
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr native)
{
  if (!native) return WrapperImplTypePtr();

  auto pThis = make_shared<WrapperImplType>();
  pThis->thisWeak_ = pThis;
  pThis->native_ = native;
  return pThis;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();

  {
    auto impl = std::dynamic_pointer_cast<RTCDtlsTransport>(wrapper);
    if (impl) return RTCDtlsTransport::toNative(impl);
  }
  {
    auto impl = std::dynamic_pointer_cast<RTCIceTransport>(wrapper);
    if (impl) return RTCIceTransport::toNative(impl);
  }

  {
    auto impl = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
    if (impl) return impl->native_;
  }
  return NativeTypePtr();
}
