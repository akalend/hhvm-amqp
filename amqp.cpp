#include "hphp/runtime/ext/extension.h"
namespace HPHP {

	class AmqpExtension : public Extension {
		public:
			AmqpExtension(): Extension("amqp", "0.1.0") {}
		
			void moduleInit() override {
				
				loadSystemlib();
			} 

	} s_amqp_extension;

HHVM_GET_MODULE(amqp);
} // namespace
