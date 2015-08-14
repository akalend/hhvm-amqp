#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context
#include "hphp/runtime/vm/native-data.h"

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
	Native::registerNativeDataInfo<AmqpExtension>(s_AMQPConnection.get(),
                                            Native::NDIFlags::NO_SWEEP);

	loadSystemlib();
}

void AmqpExtension::moduleShutdown() {

}

	


//////////////////    static    /////////////////////////
// bool AmqpExtension::is_connected = false;
// amqp_socket_t* AmqpExtension::socket = NULL;
// amqp_connection_state_t AmqpExtension::conn = NULL;

static AmqpExtension  s_amqp_extension;


bool HHVM_METHOD(AMQPConnection, isConnected) {
	
	auto *data = Native::data<AmqpData>(this_);
	return data->is_connected;
}


bool HHVM_METHOD(AMQPConnection, connect) {
  
	auto *data = Native::data<AmqpData>(this_);
  	printf( "connect to %s:%ld\n", this_->o_get(s_host, false, s_AMQPConnection).
  					toString().c_str(), this_->o_get(s_port, false, s_AMQPConnection).toInt64() );


  	bool is_persisten = this_->o_get(s_is_persisten, false, s_AMQPConnection).toBoolean();
	if (data->is_connected) {

		assert(data->conn != NULL);
		if (is_persisten) {
			raise_warning("Attempt to start transient connection while persistent transient one already established. Continue.");
		}

		return true;
	}

	assert(data->conn == NULL);
	assert(!data->is_connected);

	if (is_persisten){
		/* not implement */
	}



	data->is_connected = true;
	return true;
}





HHVM_GET_MODULE(amqp);
} // namespace
