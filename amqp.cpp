#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context

namespace HPHP {



	bool HHVM_METHOD(AMQPConnection, connect) {
	  // this_->o_get(s_name, false, Hello);
	  return true;
	}



	class AmqpExtension : public Extension {
		public:
			AmqpExtension(): Extension("amqp", "0.1.0") {}
		
			void moduleInit() override {
				
				HHVM_ME(AMQPConnection, connect);

				loadSystemlib();
			} 

	} s_amqp_extension;

HHVM_GET_MODULE(amqp);
} // namespace
