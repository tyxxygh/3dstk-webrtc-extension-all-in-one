#include "pch.h"

#include <org/ortc/RTCCertificate.h>
#include <org/ortc/RTCDtlsTransport.h>
#include <org/ortc/helpers.h>
#include <org/ortc/Error.h>

using namespace ortc;

namespace Org
{
  namespace Ortc
  {
    using std::make_shared;

    ZS_DECLARE_TYPEDEF_PTR(Internal::Helper, UseHelper)

    namespace Internal
    {
      class RTCGenerateCertificatePromiseObserver : public zsLib::IPromiseResolutionDelegate
      {
      public:
        RTCGenerateCertificatePromiseObserver(Concurrency::task_completion_event<RTCCertificate^> tce) : mTce(tce) {}

        virtual void onPromiseResolved(PromisePtr promise) override
        {
          ICertificate::PromiseWithCertificatePtr certificatePromise = ZS_DYNAMIC_PTR_CAST(ICertificate::PromiseWithCertificate, promise);
          auto nativeCert = certificatePromise->value();
          if (!nativeCert)
          {
            Error ^error = nullptr;
            mTce.set_exception(error);
            return;
          }
          auto ret = RTCCertificate::Convert(nativeCert);
          mTce.set(ret);
        }

        virtual void onPromiseRejected(PromisePtr promise) override
        {
          auto reason = promise->reason<Any>();
          auto error = Error::CreateIfGeneric(reason);
          mTce.set_exception(error);
        }

      private:
        Concurrency::task_completion_event<RTCCertificate^> mTce;
      };

    } // namespace internal

    RTCCertificate::RTCCertificate(ICertificatePtr certificate) :
      _nativePointer(certificate)
    {
    }

    IAsyncOperation<RTCCertificate^>^ RTCCertificate::GenerateCertificate() {

      ICertificateTypes::PromiseWithCertificatePtr promise;
      try {
        promise = ICertificate::generateCertificate();
      } catch (const NotSupportedError &e) {
        ORG_ORTC_THROW_NOT_SUPPORTED(UseHelper::ToCx(e.what()));
      }

      return Concurrency::create_async([promise]() -> RTCCertificate^ {
        Concurrency::task_completion_event<RTCCertificate^> tce;

        auto pDelegate(make_shared<Internal::RTCGenerateCertificatePromiseObserver>(tce));

        promise->then(pDelegate);
        promise->background();
        auto tceTask = Concurrency::task<RTCCertificate^>(tce);

        return tceTask.get();
      });
    }

    IAsyncOperation<RTCCertificate^>^ RTCCertificate::GenerateCertificate(Platform::String^ algorithmIdentifier) {

      ICertificateTypes::PromiseWithCertificatePtr promise;
      try {
        promise = ICertificate::generateCertificate(UseHelper::FromCx(algorithmIdentifier).c_str());
      } catch(const NotSupportedError &e) {
        ORG_ORTC_THROW_NOT_SUPPORTED(UseHelper::ToCx(e.what()));
      }

      return Concurrency::create_async([promise]() -> RTCCertificate^ {
        Concurrency::task_completion_event<RTCCertificate^> tce;

        auto pDelegate(make_shared<Internal::RTCGenerateCertificatePromiseObserver>(tce));

        promise->then(pDelegate);
        promise->background();
        auto tceTask = Concurrency::task<RTCCertificate^>(tce);

        return tceTask.get();
      });
    }

    Windows::Foundation::DateTime RTCCertificate::Expires::get()
    {
      ORG_ORTC_THROW_INVALID_STATE_IF(!_nativePointer)

      auto expires = _nativePointer->expires();
      return UseHelper::ToCx(expires);
    }

    RTCDtlsFingerprint^ RTCCertificate::Fingerprint::get()
    {
      if (!_nativePointer) return nullptr;
      return Internal::ToCx(_nativePointer->fingerprint());
    }

  } // namespace ortc
} // namespace org
