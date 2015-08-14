#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

namespace HPHP {


bool HHVM_METHOD(AMQPConnection, connect);
bool HHVM_METHOD(AMQPConnection, isConnected);

class AmqpExtension : public Extension {

	public:
		static amqp_socket_t *socket;
		static amqp_connection_state_t conn;
		static bool is_connected ;


		AmqpExtension(): Extension("amqp", "0.1.0") {}
	
		void moduleInit() override;
		void moduleShutdown() override;

};

} // end namespace