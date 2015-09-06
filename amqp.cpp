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
#include "hphp/runtime/base/type-object.h"  // Object

#include "hphp/runtime/vm/native-data.h"

#include "hphp/system/systemlib.h"
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_ssl_socket.h>
#include "hhvm_amqp.h"

namespace HPHP {

const StaticString
	s_AMQPConnection("AMQPConnection"),
	s_AMQPChannel("AMQPChannel"),
	s_AMQPQueue("AMQPQueue"),
	s_AMQPEnvelope("AMQPEnvelope"),
	s_AMQPExchange("AMQPExchange"),
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
	s_amqp_connection("amqp_connection"),
	s_name("name"),
	s_flags("flags"),
	s_delivery_tag("delivery_tag"),
	s_body("body"),
	s_message("message"),
	s_type("type")
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
	HHVM_ME(AMQPQueue, declare);
	HHVM_ME(AMQPQueue, delete);
	HHVM_ME(AMQPQueue, ack);
	HHVM_ME(AMQPQueue, get);


	HHVM_ME(AMQPExchange, __construct);
	HHVM_ME(AMQPExchange, bind);
	HHVM_ME(AMQPExchange, declare);


	Native::registerNativeDataInfo<AmqpExtension>(s_AMQPConnection.get(),
													 Native::NDIFlags::NO_SWEEP);

	Native::registerConstant<KindOfInt64>(s_PORT.get(), AMQP_PORT);
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

	printf("%s:%d\n", __FUNCTION__,__LINE__);

	auto *data = Native::data<AMQPConnection>(this_);
	assert(data);
	assert(data->conn);
	assert(data->channel_id);
		//TODO amqp_close_channel

	printf("channel_id=%d\n", data->channel_id);
	printf("%s:%d\n", __FUNCTION__,__LINE__);
	amqp_rpc_reply_t res = amqp_channel_close(data->conn, data->channel_id, AMQP_REPLY_SUCCESS);

	printf("%s:%d\n", __FUNCTION__,__LINE__);

	res = amqp_connection_close(data->conn, AMQP_REPLY_SUCCESS);
	data->is_connected = false;

	printf("%s:%d\n", __FUNCTION__,__LINE__);

	amqp_maybe_release_buffers_on_channel(data->conn, data->channel_id);

	data->channel_id = 0;

	printf("%s:%d\n", __FUNCTION__,__LINE__);

	if (res.reply_type) return true;

	// if (parm == AMQP_NOACK)
	// 	raise_warning("Failing to send the ack to the broker");

	amqp_destroy_connection(data->conn);
	data->conn = NULL;

	return false;
}

bool HHVM_METHOD(AMQPConnection, reconnect) {
	auto *data = Native::data<AMQPConnection>(this_);

	if (data->is_connected) {
		data->is_connected = false;
		
		amqp_rpc_reply_t res = amqp_channel_close(data->conn, data->channel_id, AMQP_REPLY_SUCCESS);		

		amqp_connection_close(data->conn, AMQP_REPLY_SUCCESS);
		// close connection

		amqp_maybe_release_buffers_on_channel(data->conn, data->channel_id);
		data->channel_id = 0;
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

	data->channel_id = 1; // init first channel
	data->amqpCnn = src_data;
	src_data->channel_id = static_cast<short>(data->channel_id);

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
	amqp_rpc_reply_t r = amqp_get_rpc_reply(src_data->conn);
	if (r.reply_type != AMQP_RESPONSE_NORMAL)
		raise_warning("The AMQPChannel class: open channel error");
	
	printf("channel_id=%d\n", data->channel_id);

	// if (data->prefetch_count) {
	// 	amqp_basic_qos(
	// 		src_data->conn,
	// 		data->channel_id,
	// 		0,							/* prefetch window size */
	// 		data->prefetch_count,	    /* prefetch message count */
	// 		 /* NOTE that RabbitMQ has reinterpreted global flag field. See https://www.rabbitmq.com/amqp-0-9-1-reference.html#basic.qos.global for details */
	// 		0							/* global flag */
	// 	);

	// 	amqp_rpc_reply_t res = amqp_get_rpc_reply(src_data->conn);

	// 	if (res.reply_type != AMQP_RESPONSE_NORMAL) {

	// 	}
	// }
}


bool HHVM_METHOD(AMQPChannel, isConnected) {

	auto *data = Native::data<AMQPChannel>(this_);
	if (!data->amqpCnn)
		raise_warning("The AMQPConnection class is`nt binding whith connection");

	return data->amqpCnn->is_connected;
}


// ------------------------------  AMQPQueue ------------------------------------------

void HHVM_METHOD(AMQPQueue, __construct, const Variant& amqpChannel) {
	auto src_data = Native::data<AMQPChannel>(amqpChannel.toObject());
	auto *data = Native::data<AMQPQueue>(this_);

	if (!src_data)
		raise_error( "Error input data");

	data->amqpCh = src_data;
};

void HHVM_METHOD(AMQPQueue, bind, const String& exchangeName, const String& routingKey) {

	auto *data = Native::data<AMQPQueue>(this_);
	if (!data)
		raise_error( "Error input data");

	if (!data->amqpCh)
		raise_warning("The AMQPQueue class is`nt binding with AMQPChannel");

	const char* queue = const_cast<char* >(this_->o_get(s_name, false, s_AMQPQueue).toString().c_str());
	const char* exchange = const_cast<char* >(exchangeName.c_str());
	const char* bindingkey = const_cast<char* >(routingKey.c_str());

	amqp_queue_bind(data->amqpCh->amqpCnn->conn , data->amqpCh->channel_id,
				amqp_cstring_bytes(queue),
				amqp_cstring_bytes(exchange),
				amqp_cstring_bytes(bindingkey),
				amqp_empty_table);

	if( (amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn)).reply_type != AMQP_RESPONSE_NORMAL )
		raise_warning("The AMQPQueue class: binding error");

}

int64_t HHVM_METHOD(AMQPQueue, declare){

	auto *data = Native::data<AMQPQueue>(this_);
	if (!data)
		raise_error( "Error input data");

	data->message_count=0;
	
	if (!data->amqpCh)
		raise_warning("The AMQPQueue class is`nt binding with AMQPChannel");

	const char* queue = const_cast<char* >(this_->o_get(s_name, false, s_AMQPQueue).toString().c_str());

	int64_t flags = this_->o_get(s_flags, false, s_AMQPQueue).toInt64();

	amqp_queue_declare_ok_t *r = amqp_queue_declare(data->amqpCh->amqpCnn->conn,
								data->amqpCh->channel_id,
								amqp_cstring_bytes(queue), 	// queue name
								(flags & AMQP_PASSIVE)    ? 1 : 0,				// passive
								(flags & AMQP_DURABLE)    ? 1 : 0, 				// durable 
								(flags & AMQP_EXCLUSIVE)  ? 1 : 0,				// exclusive
								(flags & AMQP_AUTODELETE) ? 1 : 0,				// autodelete
								amqp_empty_table);								// arguments
								

	if (!r) {
		if  (AMQP_RESPONSE_NORMAL != (amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn)).reply_type)
			raise_warning("The AMQPQueue class: declare error");

		return 0;
	} 


	data->message_count = r->message_count;
	data->consumer_count = r->consumer_count;
		
	// data-queue_name = amqp_bytes_malloc_dup(r->queue);
	// if (queue_name.bytes == NULL) {
	//   fprintf(stderr, "The AMQPQueue class: Out of memory while copying queue name");
	// }

	return data->message_count;
};


int64_t HHVM_METHOD(AMQPQueue, delete) {

	auto *data = Native::data<AMQPQueue>(this_);
	if (!data)
		raise_error( "Error input data");

	data->message_count=0;
	
	if (!data->amqpCh)
		raise_warning("The AMQPQueue class is`nt binding with AMQPChannel");

	const char* queue = const_cast<char* >(this_->o_get(s_name, false, s_AMQPQueue).toString().c_str());
	int64_t flags = this_->o_get(s_flags, false, s_AMQPQueue).toInt64();


	amqp_queue_delete_ok_t *r = amqp_queue_delete(data->amqpCh->amqpCnn->conn,
									data->amqpCh->channel_id,
									amqp_cstring_bytes(queue),
									(flags & AMQP_IFUNUSED) ? 1 : 0,
									(flags & AMQP_IFEMPTY)  ? 1 : 0);

	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn);
		raise_warning("The AMQPQueue class: delete queue error");

		return -1;
	}

	data->message_count = r->message_count;

	return data->message_count;
}



Variant HHVM_METHOD(AMQPQueue, get) {

	Object ob{Unit::loadClass(s_AMQPEnvelope.get())};
	auto *data = Native::data<AMQPQueue>(this_);
	if (!data)
		raise_error( "Error input data");
	
	if (!data->amqpCh)
		raise_warning("The AMQPQueue class is`nt binding with AMQPChannel");

	const char* queue = const_cast<char* >(this_->o_get(s_name, false, s_AMQPQueue).toString().c_str());
	int64_t flags = this_->o_get(s_flags, false, s_AMQPQueue).toInt64();


	amqp_rpc_reply_t res = amqp_basic_get(
		data->amqpCh->amqpCnn->conn,
		data->amqpCh->channel_id,
		amqp_cstring_bytes(queue),
		(AMQP_AUTOACK & flags) ? 1 : 0
	);



	if (res.reply_type != AMQP_RESPONSE_NORMAL ) {
		printf("The AMQPQueue response code: %d\n", res.reply_type);
		// raise_warning("The AMQPQueue: response error");
		return Object();
	}

	if (AMQP_BASIC_GET_EMPTY_METHOD == res.reply.id) {
		printf("The AMQPQueue: AMQP_BASIC_GET_EMPTY_METHOD\n");
		
		return Object();
	}

	assert(AMQP_BASIC_GET_OK_METHOD == res.reply.id);


	/* Fill the envelope from response */
	amqp_basic_get_ok_t *get_ok_method = static_cast<amqp_basic_get_ok_t*>(res.reply.decoded);

	amqp_envelope_t envelope;

	envelope.channel      = data->amqpCh->channel_id;
	envelope.consumer_tag = amqp_empty_bytes;
	envelope.delivery_tag = get_ok_method->delivery_tag;
	envelope.redelivered  = get_ok_method->redelivered;
	envelope.exchange     = amqp_bytes_malloc_dup(get_ok_method->exchange);
	envelope.routing_key  = amqp_bytes_malloc_dup(get_ok_method->routing_key);

	res = amqp_read_message(
		data->amqpCh->amqpCnn->conn,
		data->amqpCh->channel_id,
		&envelope.message,
		0
	);


	if (res.reply_type == AMQP_RESPONSE_NORMAL)
		printf("read: AMQP_RESPONSE_NORMAL\n" );
	else
		return Object();	


	amqp_bytes_t* message = &envelope.message.body;

	envelope.delivery_tag = get_ok_method->delivery_tag;
	envelope.redelivered  = get_ok_method->redelivered;

	Variant v_null;
	v_null.setNull();

	Variant v_tmp;

	ob.o_set(String("exchange"),
			(envelope.exchange.len) ? Variant(static_cast<char*>(envelope.exchange.bytes)) : v_null,
			s_AMQPEnvelope);

	v_tmp.setNull();
	if (envelope.consumer_tag.len) {
		v_tmp = Variant( std::string(static_cast<char*>(envelope.routing_key.bytes), envelope.routing_key.len));
	}

	ob.o_set(
		String("consumer_tag"),
		v_tmp,
		s_AMQPEnvelope);


	v_tmp.setNull();
	if (envelope.routing_key.len) {
		v_tmp = Variant(std::string(static_cast<char*>(envelope.routing_key.bytes), envelope.routing_key.len));
	}
	ob.o_set(
		String("routing_key"),
		v_tmp,
		s_AMQPEnvelope);


	v_tmp.setNull();
	if (message->len) {
		v_tmp = Variant(std::string(static_cast<char*>(message->bytes), message->len));
	}
	ob.o_set(
		s_body,
		v_tmp,
		s_AMQPEnvelope);


	ob.o_set(
		String("channel"),
		Variant(static_cast<int64_t>(envelope.channel)),
		s_AMQPEnvelope);
	

	ob.o_set(s_delivery_tag, Variant(envelope.delivery_tag), s_AMQPEnvelope);


	if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.content_type.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.content_type.bytes), envelope.message.properties.content_type.len));

			ob.o_set(
				String("content_type"),
				v_tmp,
				s_AMQPEnvelope);

		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.content_encoding.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.content_encoding.bytes), envelope.message.properties.content_encoding.len));

			ob.o_set(
				String("content_encoding"),
				v_tmp,
				s_AMQPEnvelope);
		}
	}

// TODO AMQP_BASIC_HEADERS_FLAG

	if (envelope.message.properties._flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {

		ob.o_set(
			String("delivery_mode"),
			Variant(static_cast<int64_t>(envelope.message.properties.delivery_mode)), // int64_t
			s_AMQPEnvelope);
	}


	if (envelope.message.properties._flags & AMQP_BASIC_PRIORITY_FLAG) {

		ob.o_set(
			String("priority"),
			Variant(static_cast<int64_t>(envelope.message.properties.priority)), // int64_t
			s_AMQPEnvelope);
	}


	if (envelope.message.properties._flags & AMQP_BASIC_CORRELATION_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.reply_to.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.correlation_id.bytes), envelope.message.properties.correlation_id.len));

			ob.o_set(
				String("correlation_id"),
				v_tmp,
				s_AMQPEnvelope);

		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_REPLY_TO_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.reply_to.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.reply_to.bytes), envelope.message.properties.reply_to.len));

			ob.o_set(
				String("reply_to"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_EXPIRATION_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.expiration.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.expiration.bytes), envelope.message.properties.expiration.len));

			ob.o_set(
				String("expiration"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_MESSAGE_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.message_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.message_id.bytes), envelope.message.properties.message_id.len));

			ob.o_set(
				String("message_id"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_TIMESTAMP_FLAG) {

		ob.o_set(
			String("timestamp"),
			Variant(static_cast<int64_t>(envelope.message.properties.timestamp)), // int64_t
				s_AMQPEnvelope);
	}


	if (envelope.message.properties._flags & AMQP_BASIC_TYPE_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.type.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.type.bytes), envelope.message.properties.type.len));

			ob.o_set(
				String("type"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_USER_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.user_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.user_id.bytes), envelope.message.properties.user_id.len));

			ob.o_set(
				String("user_id"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}

	if (envelope.message.properties._flags & AMQP_BASIC_APP_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.app_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.app_id.bytes), envelope.message.properties.app_id.len));

			ob.o_set(
				String("app_id"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}

	if (envelope.message.properties._flags & AMQP_BASIC_CLUSTER_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.cluster_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.cluster_id.bytes), envelope.message.properties.cluster_id.len));

			ob.o_set(
				String("cluster_id"),
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}

	ob.o_set(
		String("redelivered"),
		Variant(envelope.redelivered),
		s_AMQPEnvelope);


	amqp_destroy_envelope(&envelope);
	
	this_->o_set( String("message"), Variant(ob), s_AMQPQueue );
	
	return ob;

}


bool HHVM_METHOD(AMQPQueue, ack, int64_t delivery_tag, int64_t flags) {

	auto *data = Native::data<AMQPQueue>(this_);
	if (!data)
		raise_error( "Error input data");

	if (!data->amqpCh)
		raise_error( "Unbind AMQPChannel class");

	if (!data->amqpCh->amqpCnn)
		raise_error( "Unbind AMQPConnection class");

	if (!data->amqpCh->amqpCnn->conn){
		raise_error( "Error connection");	
	}
	
	if (data->amqpCh->amqpCnn->is_connected == false) {
		raise_warning("AMQP disconnect");
		return false;
	}

	uint64_t _flags;
	_flags =  flags ? flags : this_->o_get(s_flags, false, s_AMQPQueue).toInt64();
	
	if (delivery_tag == -1 ) {
	
		const Object body = this_->o_get(s_message, false, s_AMQPQueue).toObject();
		delivery_tag = body->o_get( s_delivery_tag, false, s_AMQPEnvelope ).toInt64() ;
		if (!delivery_tag)
			raise_warning("AMQP ACK: undefined delivery_tag");
			return false;
	}
	
	// printf("%s:%d\n", __FUNCTION__, __LINE__);

	int status = amqp_basic_ack(
					data->amqpCh->amqpCnn->conn,
					data->amqpCh->channel_id,
					delivery_tag,
                    _flags & AMQP_MULTIPLE ? 1 : 0);

	// printf("%s:%d\n", __FUNCTION__, __LINE__);

	if (status != AMQP_STATUS_OK) {
		/* Emulate library error */
		amqp_rpc_reply_t res;
		res.reply_type 	  = AMQP_RESPONSE_LIBRARY_EXCEPTION;
		res.library_error = status;

		raise_warning("The AMQPQueue class: ack error");

		amqp_maybe_release_buffers_on_channel(data->amqpCh->amqpCnn->conn, data->amqpCh->channel_id);
		return false;
	}

	return true;
}




// ------------------------------  AMQPExchange ------------------------------------------

void HHVM_METHOD(AMQPExchange, __construct, const Variant& amqpChannel) {
	
	auto src_data = Native::data<AMQPChannel>(amqpChannel.toObject());
	auto *data = Native::data<AMQPExchange>(this_);

	if (!src_data)
		raise_error( "Error input data");

	data->amqpCh = src_data;

}


bool HHVM_METHOD(AMQPExchange, bind, const String& queueName, const String& routingKey) {


	auto *data = Native::data<AMQPExchange>(this_);
	if (!data)
		raise_error( "Error input data");

	if (!data->amqpCh)
		raise_warning("The AMQPExchange class is`nt binding with AMQPChannel");

	const char* exchange = const_cast<char* >(this_->o_get(s_name, false, s_AMQPExchange).toString().c_str());
	const char* queue = const_cast<char* >(queueName.c_str());
	const char* bindingkey = const_cast<char* >(routingKey.c_str());

	amqp_queue_bind(data->amqpCh->amqpCnn->conn , 
				data->amqpCh->channel_id,
				amqp_cstring_bytes(queue),
				amqp_cstring_bytes(exchange),
				amqp_cstring_bytes(bindingkey),
				amqp_empty_table);

	if( (amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn)).reply_type != AMQP_RESPONSE_NORMAL )
		raise_warning("The AMQPExchange class: binding error");

	return true;
}

bool HHVM_METHOD(AMQPExchange, declare){

	auto *data = Native::data<AMQPExchange>(this_);
	if (!data)
		raise_error( "Error input data");

	
	if (!data->amqpCh)
		raise_warning("The AMQPExchange class is`nt binding with AMQPChannel");

	if (!data->amqpCh->amqpCnn)
		raise_error( "Unbind AMQPConnection class");

	if (!data->amqpCh->amqpCnn->conn){
		raise_error( "Error connection");	
	}
	
	if (data->amqpCh->amqpCnn->is_connected == false) {
		raise_warning("AMQP disconnect");
		return false;
	}

	const char* exchange = const_cast<char* >(this_->o_get(s_name, false, s_AMQPExchange).toString().c_str());
	const char* type = const_cast<char* >(this_->o_get(s_type, false, s_AMQPExchange).toString().c_str());



	int64_t flags = this_->o_get(s_flags, false, s_AMQPExchange).toInt64();

	amqp_exchange_declare(
		data->amqpCh->amqpCnn->conn,		// state connection state
		data->amqpCh->channel_id, 			// channel the channel to do the RPC on
		amqp_cstring_bytes(exchange), 		// exchange name
		amqp_cstring_bytes(type), 			// type
		(flags & AMQP_PASSIVE)  ? 1 : 0, 	// passive flag
		(flags & AMQP_DURABLE)  ? 1 : 0, 	// durable flag
		(flags & AMQP_AUTODELETE)  ? 1 : 0, // autodelete flag
		(flags & AMQP_INTERNAL)  ? 1 : 0, 	// internal flag
		amqp_empty_table); 					// arguments



	amqp_rpc_reply_t res = amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn);

	/* handle any errors that occured outside of signals */
	if (res.reply_type != AMQP_RESPONSE_NORMAL) {
		raise_warning("AMQP response error");
		return false;
	}
	
	return true;
}

HHVM_GET_MODULE(amqp);
} // namespace
