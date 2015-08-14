#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "hhvm_amqp.h"

namespace HPHP {

const StaticString
  s_AMQPConnection("AMQPConnection"),
  s_host("host"),
  s_vhost("vhost"),
  s_login("login"),
  s_password("password"),
  s_timeout("timeout"),
  s_connect_timeout("connect_timeout"),
  s_is_persisten("is_persisten"),
  s_port("port");



//////////////////    module   /////////////////////////



void AmqpExtension::moduleInit() {
		
	HHVM_ME(AMQPConnection, connect);
	HHVM_ME(AMQPConnection, isConnected);
	
	loadSystemlib();
}

void AmqpExtension::moduleShutdown() {

}

	


//////////////////    static    /////////////////////////
bool AmqpExtension::is_connected = false;
amqp_socket_t* AmqpExtension::socket;
amqp_connection_state_t AmqpExtension::conn;

static AmqpExtension  s_amqp_extension;




bool HHVM_METHOD(AMQPConnection, isConnected) {
	return AmqpExtension::is_connected;
}


bool HHVM_METHOD(AMQPConnection, connect) {
  
  AmqpExtension::is_connected = true;
  printf( "connect to %s:%ld\n", this_->o_get(s_host, false, s_AMQPConnection).toString().c_str(), this_->o_get(s_port, false, s_AMQPConnection).toInt64() );
	return true;
}





HHVM_GET_MODULE(amqp);
} // namespace
