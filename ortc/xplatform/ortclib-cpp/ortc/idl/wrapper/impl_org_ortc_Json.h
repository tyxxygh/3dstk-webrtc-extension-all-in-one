
#pragma once

#include "types.h"
#include "generated/org_ortc_Json.h"

#include <zsLib/XML.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Json : public wrapper::org::ortc::Json
        {
          JsonWeakPtr thisWeak_;
          zsLib::XML::ElementPtr native_;

          Json();
          virtual ~Json();

          // methods Json
          virtual void wrapper_init_org_ortc_Json(String jsonString) override;
          virtual String toString() override;

          static JsonPtr toWrapper(zsLib::XML::ElementPtr rootEl);
          static zsLib::XML::ElementPtr toNative(wrapper::org::ortc::JsonPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

