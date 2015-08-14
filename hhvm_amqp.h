#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

namespace HPHP {


bool HHVM_METHOD(AMQPConnection, connect);
bool HHVM_METHOD(AMQPConnection, isConnected);


class AmqpData{
public:
		amqp_socket_t *socket;
		amqp_connection_state_t conn;
		bool is_connected ;

		AmqpData(){
			socket = NULL;
			conn = NULL;
			is_connected = NULL;
		}

};

class AmqpExtension : public Extension {

	public:
		AmqpExtension(): Extension("amqp", "0.1.0"){
			m_data = AmqpData();
		}
	
		void moduleInit() override;
		void moduleShutdown() override;

	private:
		AmqpData m_data;
};

} // end namespace