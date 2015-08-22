#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

namespace HPHP {

#define AMQP_PORT  5672

bool HHVM_METHOD(AMQPConnection, connect);
bool HHVM_METHOD(AMQPConnection, isConnected);
bool HHVM_METHOD(AMQPConnection, reconnect);
bool HHVM_METHOD(AMQPConnection, disconnect);


enum amqp_error_code {
	AMQP_ERR_NONE = 0,
	AMQP_ERR_CANNOT_OPEN_SOCKET,
	AMQP_ERR_CANNOT_CREATE_SOCKET,
	AMQP_ERROR_LOGIN
};

class AmqpData{
public:
		amqp_socket_t *socket = NULL;
		amqp_connection_state_t conn = NULL;
		bool is_connected = false;
		char* host = NULL;
		char* vhost = NULL;
		char* password = NULL;
		char* login = NULL;
		short port = AMQP_PORT;
		short err = 0;
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