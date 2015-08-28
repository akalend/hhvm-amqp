/*
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MIT
 *
 * Portions created by Alexandre Kalendarev are Copyright (c) 2015
 * Alexandre Kalendarev. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ***** END LICENSE BLOCK *****
 */

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
  s_port("port"),
  s_PORT("AMQP_PORT"),
  s_NOPARM("AMQP_NOPARAM"),
  s_NOACK("AMQP_NOACK"),
  s_AMQPChannel("AMQPChannel"),
  s_amqp_connection("amqp_connection")
  ;



//////////////////    module   /////////////////////////



void AmqpExtension::moduleInit() {
		
	HHVM_ME(AMQPConnection, connect);
	HHVM_ME(AMQPConnection, isConnected);
	HHVM_ME(AMQPConnection, disconnect);

	HHVM_ME(AMQPChannel, __construct);
	HHVM_ME(AMQPChannel, isConnected);


	HHVM_ME(AMQPQueue, __construct);
	HHVM_ME(AMQPQueue, bind);
	// HHVM_ME(AMQPQueue, setName);


	Native::registerNativeDataInfo<AmqpExtension>(s_AMQPConnection.get(),
													 Native::NDIFlags::NO_SWEEP);

    Native::registerConstant<KindOfInt64>(s_PORT.get(), 5672);
    Native::registerConstant<KindOfInt64>(s_NOPARM.get(), AMQP_NOPARAM);
    Native::registerConstant<KindOfInt64>(s_NOACK.get(), AMQP_NOACK);

	loadSystemlib();
}

void AmqpExtension::moduleShutdown() {
	
	// auto *data = Native::data<AMQPConnection>(this_);
	// if (data->conn) {
	// 	amqp_connection_close(conn->conn);
	// 	amqp_destroy_connection(data->conn);
	// 	data->conn = NULL;
	// }
}

	
//////////////////    static    /////////////////////////
static AmqpExtension  s_amqp_extension;




// ------------------------------------------------------

bool amqpConnect( ObjectData* this_) {
	
	// conn = amqp_new_connection();
	
	auto *data = Native::data<AMQPConnection>(this_);

  	// printf( "connect to %s:%d\n", data->host, data->port);

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

		// printf("%s cnn 0x%lX\n", __FUNCTION__,data->conn);

		return data->is_connected = true;
	}

	data->err = AMQP_ERROR_LOGIN;	
	return data->is_connected = false;
}


amqp_channel_t getChannelSlot(AMQPChannel *channel) {
	if (channel->used_slots >= AMQP_MAX_CHANNELS + 1) {
		return 0;
	}

	amqp_channel_t slot;

	for (slot = 1; slot < AMQP_MAX_CHANNELS + 1; slot++) {
		if (channel->slots[slot] == 0) {
			return slot;
		}
	}
}



// ---------------------------------------------------------------------------------------------------



// ------------------------------  AMQPConnect ------------------------------------------

bool HHVM_METHOD(AMQPConnection, isConnected) {
	
	auto *data = Native::data<AMQPConnection>(this_);
	return data->is_connected;
}

bool HHVM_METHOD(AMQPConnection, disconnect, int64_t parm) {
	auto *data = Native::data<AMQPConnection>(this_);

		//TODO amqp_close_channel
		// printf("%s cnn 0x%lX\n", __FUNCTION__,data->conn);
	data->is_connected = false;
	amqp_rpc_reply_t res = amqp_connection_close(data->conn, AMQP_REPLY_SUCCESS);
	if (res.reply_type) return true;
	if (parm == AMQP_NOACK)
		raise_warning("Failing to send the ack to the broker");
	return false;
}

bool HHVM_METHOD(AMQPConnection, reconnect) {
	auto *data = Native::data<AMQPConnection>(this_);

	if (data->is_connected) {
		data->is_connected = false;
		
		//TODO amqp_close_channel

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
  
	auto *data = Native::data<AMQPConnection>(this_);

	/* not implement */
  	// bool is_persisten = this_->o_get(s_is_persisten, false, s_AMQPConnection).toBoolean();

	// if (data->is_connected) {

	// 	assert(data->conn != NULL);
	// 	if (is_persisten) {
	// 		raise_warning("Attempt to start transient connection while persistent transient one already established. Continue.");
	// 	}

	// 	return true;
	// }

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

// ------------------------------  AMQPChannel ------------------------------------------

void HHVM_METHOD(AMQPChannel, __construct, const Variant& amqpConnect) {
	
	auto src_data = Native::data<AMQPConnection>(amqpConnect.toObject());
	auto *data = Native::data<AMQPChannel>(this_);
	if (!src_data)
		raise_error( "Error input data");

	data->amqpCnn = src_data;

	// if (!data->slots) {
	// 	data->slots = cmalloc(AMQP_MAX_CHANNELS+1, sizeof(amqp_channel_t));
	// }

//	amqp_channel_t slot = getChannelSlot(data);	

	/* Check that we got a valid channel */
	// if (!slot) {
	// 	raise_warning( "Could not create channel. Connection has no open channel slots remaining.");
	// 	return;
	// }

	// channel_id 
	amqp_channel_open(src_data->conn, data->channel_id );
}


bool HHVM_METHOD(AMQPChannel, isConnected) {
	

	auto *data = Native::data<AMQPChannel>(this_);
	if (!data->amqpCnn)
		raise_warning("The AMQPConnection class is`nt binding");

	return data->amqpCnn->is_connected;
}


// ------------------------------  AMQPQueue ------------------------------------------

void HHVM_METHOD(AMQPQueue, __construct, const Variant& amqpQueue) {
	auto src_data = Native::data<AMQPChannel>(amqpQueue.toObject());
	auto *data = Native::data<AMQPQueue>(this_);

	if (!src_data)
		raise_error( "Error input data");

	data->amqpCh = src_data;

};

void HHVM_METHOD(AMQPQueue, bind, const String& exchangeName, const String& routingKey) {

}





HHVM_GET_MODULE(amqp);
} // namespace
