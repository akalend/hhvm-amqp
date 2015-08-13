#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context

namespace HPHP {

const StaticString
  s_AMQPConnection("AMQPConnection"),
  s_host("host"),
  s_host("vhost"),
  s_host("login"),
  s_host("password"),
  s_host("timeout"),
  s_host("connect_timeout"),
  s_host("is_persisten"),
  s_port("port");


	bool HHVM_METHOD(AMQPConnection, connect) {
	  
	  printf( "connect to %s:%ld\n", this_->o_get(s_host, false, s_AMQPConnection).toString().c_str(), this_->o_get(s_port, false, s_AMQPConnection).toInt64() );
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
