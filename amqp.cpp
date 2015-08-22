#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"  // g_context
#include "hphp/runtime/vm/native-data.h"

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_ssl_socket.h>
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
	
	HHVM_ME(AMQPConnection, reconnect);
	HHVM_ME(AMQPConnection, disconnect);	
	HHVM_ME(AMQPConnection, connect);
	HHVM_ME(AMQPConnection, isConnected);
	Native::registerNativeDataInfo<AmqpExtension>(s_AMQPConnection.get(),
                                            Native::NDIFlags::NO_SWEEP);

	loadSystemlib();
}

void AmqpExtension::moduleShutdown() {
	
	// auto *data = Native::data<AmqpData>(this_);
	// if (data->conn) {
	// 	amqp_connection_close(conn->conn);
	// 	amqp_destroy_connection(data->conn);
	// 	data->conn = NULL;
	// }
}

	
//////////////////    static    /////////////////////////
static AmqpExtension  s_amqp_extension;



bool amqpConnect( ObjectData* this_) {
	
	// conn = amqp_new_connection();
	
	auto *data = Native::data<AmqpData>(this_);


  	printf( "connect to %s:%d\n", data->host, data->port);

	data->conn = amqp_new_connection();
	int channel_MAX = 0;
	int frame_MAX = 131072;
	int heartbeat = 0;

	amqp_socket_t *socket =  amqp_tcp_socket_new(data->conn);
	if (!socket) {
		data->err = AMQP_ERR_CANNOT_CREATE_SOCKET;
		return false;
	}
  
	int status = amqp_socket_open(socket, data->host, data->port);
	if (status) {
		data->err = AMQP_ERR_CANNOT_OPEN_SOCKET;
		return false;
	}


	amqp_rpc_reply_t res = amqp_login(data->conn, data->vhost, channel_MAX, frame_MAX,
							heartbeat, AMQP_SASL_METHOD_PLAIN, data->login, data->password);


	if ( res.reply_type == AMQP_RESPONSE_NORMAL) {

		return data->is_connected = true;
	}

	data->err = AMQP_ERROR_LOGIN;	
	return data->is_connected = false;
}

bool HHVM_METHOD(AMQPConnection, isConnected) {
	
	auto *data = Native::data<AmqpData>(this_);
	return data->is_connected;
}

bool HHVM_METHOD(AMQPConnection, disconnect) {
	auto *data = Native::data<AmqpData>(this_);
	amqp_rpc_reply_t res = amqp_connection_close(data->conn, AMQP_REPLY_SUCCESS);
	if ( AMQP_RESPONSE_NORMAL == res.reply_type) return true;
	
	raise_warning("Failing to send the ack to the broker");
	return false;
}

bool HHVM_METHOD(AMQPConnection, reconnect) {
	auto *data = Native::data<AmqpData>(this_);

	if (data->is_connected) {
		data->is_connected = false;
		amqp_connection_close(data->conn, AMQP_REPLY_SUCCESS);
		// close connection
	}

	data->host = const_cast<char* >(this_->o_get(s_host, false, s_AMQPConnection).toString().c_str());
	data->port = static_cast<short>(this_->o_get(s_port, false, s_AMQPConnection).toInt64());
	data->vhost = const_cast<char* >(this_->o_get(s_vhost, false, s_AMQPConnection).toString().c_str());
	data->password = const_cast<char* >(this_->o_get(s_password, false, s_AMQPConnection).toString().c_str());
	data->login = const_cast<char* >(this_->o_get(s_login, false, s_AMQPConnection).toString().c_str());


	if (!amqpConnect(this_)) {

		if (data->err == AMQP_ERR_CANNOT_OPEN_SOCKET) { 
				raise_warning("Can'not open socket");}

		if (data->err == AMQP_ERR_CANNOT_CREATE_SOCKET) { 
				raise_warning("Can'not create socket");}

		return false;
	}

	return true;
}

bool HHVM_METHOD(AMQPConnection, connect) {
  
	auto *data = Native::data<AmqpData>(this_);


  	bool is_persisten = this_->o_get(s_is_persisten, false, s_AMQPConnection).toBoolean();
	if (data->is_connected) {

		assert(data->conn != NULL);
		if (is_persisten) {
			//raise_warning("Attempt to start transient connection while persistent transient one already established. Continue.");
		}

		return true;
	}

	assert(data->conn == NULL);
	assert(!data->is_connected);

		/* not implement */
	// if (is_persisten){
	// }

		/* not implement */	
	// if (this_->o_get(s_timeout,false,s_AMQPConnection).toDouble() > 0) {
	// }
	
	data->host = const_cast<char* >(this_->o_get(s_host, false, s_AMQPConnection).toString().c_str());
	data->port = static_cast<short>(this_->o_get(s_port, false, s_AMQPConnection).toInt64());
	data->vhost = const_cast<char* >(this_->o_get(s_vhost, false, s_AMQPConnection).toString().c_str());
	data->password = const_cast<char* >(this_->o_get(s_password, false, s_AMQPConnection).toString().c_str());
	data->login = const_cast<char* >(this_->o_get(s_login, false, s_AMQPConnection).toString().c_str());


	if (!amqpConnect(this_)) {

		if (data->err == AMQP_ERR_CANNOT_OPEN_SOCKET) { 
				raise_warning("Can'not open socket");}

		if (data->err == AMQP_ERR_CANNOT_CREATE_SOCKET) { 
				raise_warning("Can'not create socket");}

		return false;
	}

	return true;
}





HHVM_GET_MODULE(amqp);
} // namespace
