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
	
	loadSystemlib();
}

void AmqpExtension::moduleShutdown() {

}

	


//////////////////    static    /////////////////////////
int AmqpExtension::is_connected = 0;
amqp_socket_t* AmqpExtension::socket;
amqp_connection_state_t AmqpExtension::conn;

static AmqpExtension  s_amqp_extension;



bool HHVM_METHOD(AMQPConnection, connect) {
  
  AmqpExtension::is_connected = 1;
  printf( "connect to %s:%ld\n", this_->o_get(s_host, false, s_AMQPConnection).toString().c_str(), this_->o_get(s_port, false, s_AMQPConnection).toInt64() );
	return true;
}





HHVM_GET_MODULE(amqp);
} // namespace
