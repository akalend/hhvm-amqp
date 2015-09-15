/** ***** BEGIN LICENSE BLOCK *****
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

#define NOPARAM -1

#define AMQP_TRACE printf("%s:%d\n", __FUNCTION__, __LINE__);

#define GET_CLASS_DATA_AND_CHECK( class_name ) 			\
	auto *data = Native::data<class_name>(this_);		\
	if (!data)											\
		raise_error( "Error input data");				\
	if (!data->amqpCh)									\
		raise_warning("The ##class_name## class is`nt binding with AMQPChannel");	\
	if (!data->amqpCh->amqpCnn)							\
		raise_error( "Unbind AMQPConnection class");	\
	if (!data->amqpCh->amqpCnn->conn){					\
		raise_error( "Error connection");				\
	}													\
	if (data->amqpCh->amqpCnn->is_connected == false) {	\
		raise_warning("AMQP disconnect");				\
		return false;									\
	}

#define ANALYZE_RESPONSE_AND_RETURN()					\
	amqp_rpc_reply_t res = amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn);	\
	if (res.reply_type != AMQP_RESPONSE_NORMAL) {		\
		raise_warning("AMQP response error");			\
		return false;									\
	}													\
	return true;



#define ADD_AMQP_STRING_PROPERTY(var, field,flag ) 		\
	switch (var.getType()) {							\
		case KindOfNull : 								\
		case KindOfUninit : 							\
			break;										\
		case KindOfString :								\
		case KindOfStaticString :						\
			props._flags |= flag;						\
			props.field = amqp_cstring_bytes( var.toString().c_str() );\
			break;										\
		default:										\
			raise_warning("value argument error");		\
	}


#define ADD_AMQP_LONG_PROPERTY(var, field,flag ) 		\
	if (var.getType() == KindOfInt64 ){					\
		props._flags |= flag;							\
		props.field = var.toInt64();					\
	}


namespace HPHP {

const StaticString
	s_AMQPConnection("AMQPConnection"),
	s_AMQPChannel("AMQPChannel"),
	s_AMQPQueue("AMQPQueue"),
	s_AMQPEnvelope("AMQPEnvelope"),
	s_AMQPExchange("AMQPExchange"),
	s_exchange("exchange"),
	s_routing_key("routing_key"),
	s_queue_count("queue_count"),
	s_host("host"),
	s_vhost("vhost"),
	s_login("login"),
	s_password("password"),
	s_timeout("timeout"),
	s_channel("channel"),
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
	s_type("type"),
	s_app_id("app_id"),
	s_user_id("user_id"),
	s_correlation_id("correlation_id"),
	s_reply_to("reply_to"),
	s_message_id("message_id"),
	s_arguments("arguments"),
	s_content_type("content_type"),
	s_content_encoding("content_encoding"),
	s_expiration("expiration"),
	s_delivery_mode("delivery_mode"),
	s_priority("priority"),
	s_timestamp("timestamp"),
	s_consumer_tag("consumer_tag"),	
	s_AMQP_EX_TYPE_DIRECT("AMQP_EX_TYPE_DIRECT"),
	s_AMQP_EX_TYPE_FANOUT("AMQP_EX_TYPE_FANOUT"),
	s_AMQP_EX_TYPE_TOPIC("AMQP_EX_TYPE_TOPIC"),
	s_AMQP_EX_TYPE_HEADERS("AMQP_EX_TYPE_HEADERS"),
	s_headers("headers"),
	s_direct("direct"),
	s_fanout("fanout"),
	s_topic("topic"),
	s_AMQP_PASSIVE("AMQP_PASSIVE"),
	s_AMQP_DURABLE("AMQP_DURABLE"),
	s_AMQP_EXCLUSIVE("AMQP_EXCLUSIVE"),
	s_AMQP_AUTODELETE("AMQP_AUTODELETE"),
	s_AMQP_IFUNUSED("AMQP_IFUNUSED"),
	s_AMQP_IFEMPTY("AMQP_IFEMPTY"),
	s_AMQP_AUTOACK("AMQP_AUTOACK"),
	s_AMQP_MULTIPLE("AMQP_MULTIPLE"),
	s_AMQP_INTERNAL("AMQP_INTERNAL"),
	s_AMQP_MANDATORY("AMQP_MANDATORY"),
	s_AMQP_IMMEDIATE("AMQP_IMMEDIATE"),
	s_AMQP_NOWAIT("s_AMQP_NOWAIT"),
	s_AMQP_REQUEUE("AMQP_REQUEUE"),
	s_AMQP_NOLOCAL("AMQP_NOLOCAL"),
	s_x_type("x-type"),
	s_bool("bool"),
	s_int("int"),
	s_double("double"),
	s_null("null")
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
	HHVM_ME(AMQPQueue, cancel);

	HHVM_ME(AMQPExchange, __construct);
	HHVM_ME(AMQPExchange, bind);
	HHVM_ME(AMQPExchange, declare);
	HHVM_ME(AMQPExchange, delete);
	HHVM_ME(AMQPExchange, publish);

	Native::registerNativeDataInfo<AmqpExtension>(s_AMQPConnection.get(),
													 Native::NDIFlags::NO_SWEEP);

	Native::registerConstant<KindOfInt64>(s_PORT.get(), AMQP_PORT);
	Native::registerConstant<KindOfInt64>(s_NOPARM.get(), AMQP_NOPARAM);
	Native::registerConstant<KindOfInt64>(s_NOACK.get(), AMQP_NOACK);

	Native::registerConstant<KindOfInt64>(s_AMQP_PASSIVE.get(), 	AMQP_PASSIVE);
	Native::registerConstant<KindOfInt64>(s_AMQP_DURABLE.get(), 	AMQP_DURABLE);
	Native::registerConstant<KindOfInt64>(s_AMQP_AUTODELETE.get(), 	AMQP_AUTODELETE);
	Native::registerConstant<KindOfInt64>(s_AMQP_IFUNUSED.get(), 	AMQP_IFUNUSED);
	Native::registerConstant<KindOfInt64>(s_AMQP_IFEMPTY.get(), 	AMQP_IFEMPTY);
	Native::registerConstant<KindOfInt64>(s_AMQP_AUTOACK.get(), 	AMQP_AUTOACK);
	Native::registerConstant<KindOfInt64>(s_AMQP_MULTIPLE.get(), 	AMQP_MULTIPLE);
	Native::registerConstant<KindOfInt64>(s_AMQP_INTERNAL.get(), 	AMQP_INTERNAL);
	Native::registerConstant<KindOfInt64>(s_AMQP_MANDATORY.get(), 	AMQP_MANDATORY);
	Native::registerConstant<KindOfInt64>(s_AMQP_IMMEDIATE.get(), 	AMQP_IMMEDIATE);
	Native::registerConstant<KindOfInt64>(s_AMQP_NOLOCAL.get(), 	AMQP_NOLOCAL);
	Native::registerConstant<KindOfInt64>(s_AMQP_NOWAIT.get(), 		AMQP_NOWAIT);
	Native::registerConstant<KindOfInt64>(s_AMQP_REQUEUE.get(), 	AMQP_REQUEUE);

	Native::registerConstant<KindOfStaticString>(s_AMQP_EX_TYPE_HEADERS.get(), s_headers.get());
	Native::registerConstant<KindOfStaticString>(s_AMQP_EX_TYPE_DIRECT.get(), s_direct.get());
	Native::registerConstant<KindOfStaticString>(s_AMQP_EX_TYPE_FANOUT.get(), s_fanout.get());
	Native::registerConstant<KindOfStaticString>(s_AMQP_EX_TYPE_TOPIC.get(), s_topic.get());


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


    amqp_table_t client_properties;
	client_properties.entries = (amqp_table_entry_t *) calloc(2, sizeof(amqp_table_entry_t));

	const char hhvm_amqp[9] = {'h','h','v','m','_','a','m','q','p'}; 
	const char client_type[11] = {'c','l','i','e','n','t','-','t','y','p','e'}; 

	const char author[16] = {'h','h','v','m','_','a','m','q','p',' ','a','u','t','h','o','r'}; 
	const char name[10] = {'K','a','l','e','n','d','a','r','e','v'};

	client_properties.num_entries=2;
	client_properties.entries[0].value.kind = AMQP_FIELD_KIND_UTF8;
	client_properties.entries[0].value.value.bytes.len = 9;
	client_properties.entries[0].value.value.bytes.bytes = (amqp_bytes_t*)hhvm_amqp;
	client_properties.entries[0].key.len = 11;
	client_properties.entries[0].key.bytes = (amqp_bytes_t*)client_type;
	client_properties.entries[1].value.kind = AMQP_FIELD_KIND_UTF8;
	client_properties.entries[1].value.value.bytes.len = 10;
	client_properties.entries[1].value.value.bytes.bytes = (amqp_bytes_t*) name;
	client_properties.entries[1].key.len = 16;
	client_properties.entries[1].key.bytes = (amqp_bytes_t*) author;

	
	amqp_rpc_reply_t res = amqp_login_with_properties(
			data->conn, 
			data->vhost, 
			channel_MAX, 
			frame_MAX,
			heartbeat, 
			&client_properties,
			AMQP_SASL_METHOD_PLAIN, 
			data->login, 
			data->password);

	free(client_properties.entries);


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

	amqp_rpc_reply_t res = amqp_channel_close(data->conn, data->channel_id, AMQP_REPLY_SUCCESS);

	// printf("%s:%d\n", __FUNCTION__,__LINE__);

	res = amqp_connection_close(data->conn, AMQP_REPLY_SUCCESS);
	data->is_connected = false;

	amqp_maybe_release_buffers_on_channel(data->conn, data->channel_id);

	data->channel_id = 0;

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
	
	// printf("channel_id=%d\n", data->channel_id);

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


	// GET_CLASS_DATA_AND_CHECK( AMQPQueue );

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

	GET_CLASS_DATA_AND_CHECK( AMQPQueue );

	data->message_count=0;

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

	GET_CLASS_DATA_AND_CHECK( AMQPQueue );

	data->message_count=0;
	
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

		return NOPARAM;
	}

	data->message_count = r->message_count;

	return data->message_count;
}


bool HHVM_METHOD(AMQPQueue, cancel, const String& consumer_tag) {

	GET_CLASS_DATA_AND_CHECK( AMQPQueue );


	// взять consumer_tag из $this->message

	amqp_basic_cancel_ok_t *r = amqp_basic_cancel(data->amqpCh->amqpCnn->conn,
									data->amqpCh->channel_id,
									amqp_cstring_bytes(consumer_tag.c_str()));
		


	if (!r) {
		amqp_rpc_reply_t res = amqp_get_rpc_reply(data->amqpCh->amqpCnn->conn);
		raise_warning("The AMQPQueue class: cancel queue error");

		return false;
	}

	return true;	
}


Variant HHVM_METHOD(AMQPQueue, get) {

	Object ob{Unit::loadClass(s_AMQPEnvelope.get())};

	GET_CLASS_DATA_AND_CHECK( AMQPQueue );


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
// printf("count %d\n", get_ok_method->message_count);
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


	if (res.reply_type != AMQP_RESPONSE_NORMAL)
		return Object();	

//		printf("read: AMQP_RESPONSE_NORMAL\n" );


	amqp_bytes_t* message = &envelope.message.body;

	envelope.delivery_tag = get_ok_method->delivery_tag;
	envelope.redelivered  = get_ok_method->redelivered;

	Variant v_null;
	v_null.setNull();

	Variant v_tmp;

	ob.o_set(s_queue_count,
			(get_ok_method->message_count) ? Variant(static_cast<int64_t>(get_ok_method->message_count)) : v_null,
			s_AMQPEnvelope);

	ob.o_set(s_exchange,
			(envelope.exchange.len) ? Variant(static_cast<char*>(envelope.exchange.bytes)) : v_null,
			s_AMQPEnvelope);

	v_tmp.setNull();
	if (envelope.consumer_tag.len) {
		v_tmp = Variant( std::string(static_cast<char*>(envelope.consumer_tag.bytes), envelope.consumer_tag.len));
	}

	ob.o_set(
		s_consumer_tag,
		v_tmp,
		s_AMQPEnvelope);


	v_tmp.setNull();
	if (envelope.routing_key.len) {
		v_tmp = Variant(std::string(static_cast<char*>(envelope.routing_key.bytes), envelope.routing_key.len));
	}
	ob.o_set(
		s_routing_key,
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
		s_channel,
		Variant(static_cast<int64_t>(envelope.channel)),
		s_AMQPEnvelope);
	

	ob.o_set(s_delivery_tag, Variant(envelope.delivery_tag), s_AMQPEnvelope);


	if (envelope.message.properties._flags & AMQP_BASIC_HEADERS_FLAG) {
		
		// amqp_table_t *table;
		Array headers;
		int i;
		for (i = 0; i < envelope.message.properties.headers.num_entries; i++) {
			amqp_table_entry_t *entry = &(envelope.message.properties.headers.entries[i]);
		
			String key(std::string(static_cast<char*>(entry->key.bytes), entry->key.len));
			Variant value;
		
			switch (entry->value.kind) {
				case AMQP_FIELD_KIND_BOOLEAN: {
						value =static_cast<bool>(entry->value.value.boolean);
						break;
					}
					
				case AMQP_FIELD_KIND_I8: {
					value =static_cast<int64_t>(entry->value.value.i8);
					break;
					}
				case AMQP_FIELD_KIND_U8:
					value =static_cast<int64_t>(entry->value.value.u8);
					break;
				case AMQP_FIELD_KIND_I16:
					value =static_cast<int64_t>(entry->value.value.i16);
					break;
				case AMQP_FIELD_KIND_U16:
					value =static_cast<int64_t>(entry->value.value.u16);
					break;
				case AMQP_FIELD_KIND_I32:
					value =static_cast<int64_t>(entry->value.value.i32);
					break;
				case AMQP_FIELD_KIND_U32:
					value =static_cast<int64_t>(entry->value.value.u16);
					break;
				case AMQP_FIELD_KIND_I64:
				case AMQP_FIELD_KIND_U64:
					value =static_cast<int64_t>(entry->value.value.i64);
					break;
				case AMQP_FIELD_KIND_F32:
					value =static_cast<float>(entry->value.value.f32);
					break;
				case AMQP_FIELD_KIND_F64:
					value =static_cast<double>(entry->value.value.f64);
					break;
				case AMQP_FIELD_KIND_UTF8:
				case AMQP_FIELD_KIND_BYTES:
				printf("str type\n");
					value = std::string(static_cast<char*>(entry->value.value.bytes.bytes), entry->value.value.bytes.len);
					break;
				case AMQP_FIELD_KIND_ARRAY:
				printf("arr type\n");
					// {
					// 	int j;
					// 	array_init(value);
					// 	for (j = 0; j < entry->value.value.array.num_entries; ++j) {
					// 		switch (entry->value.value.array.entries[j].kind) {
					// 			case AMQP_FIELD_KIND_UTF8:
					// 				add_next_index_stringl(
					// 					value,
					// 					entry->value.value.array.entries[j].value.bytes.bytes,
					// 					entry->value.value.array.entries[j].value.bytes.len,
					// 					1
					// 				);
					// 				break;
					// 			case AMQP_FIELD_KIND_TABLE:
					// 				{
					// 					zval *subtable;
					// 					MAKE_STD_ZVAL(subtable);
					// 					array_init(subtable);
					// 					parse_amqp_table(
					// 						&(entry->value.value.array.entries[j].value.table),
					// 						subtable
					// 					);
					// 					add_next_index_zval(value, subtable);
					// 				}
					// 				break;
					// 		}
					// 	}
					// }
					break;
				case AMQP_FIELD_KIND_TABLE:
					printf("table type\n");
				 //    array_init(value);
					// parse_amqp_table(&(entry->value.value.table), value);
					break;
				case AMQP_FIELD_KIND_TIMESTAMP:
					value =static_cast<double>(entry->value.value.u64);
					break;
				case AMQP_FIELD_KIND_VOID:
				case AMQP_FIELD_KIND_DECIMAL:
				default:
					value.setNull();
					break;
			}

			headers.add(key,value,true);
		}

		ob.o_set(
			s_headers,
			headers,
			s_AMQPEnvelope);
	}

	if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_TYPE_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.content_type.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.content_type.bytes), envelope.message.properties.content_type.len));

			ob.o_set(
				s_content_type,
				v_tmp,
				s_AMQPEnvelope);

		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_CONTENT_ENCODING_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.content_encoding.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.content_encoding.bytes), envelope.message.properties.content_encoding.len));

			ob.o_set(
				s_content_encoding,
				v_tmp,
				s_AMQPEnvelope);
		}
	}

// TODO AMQP_BASIC_HEADERS_FLAG

	if (envelope.message.properties._flags & AMQP_BASIC_DELIVERY_MODE_FLAG) {

		ob.o_set(
			s_delivery_mode,
			Variant(static_cast<int64_t>(envelope.message.properties.delivery_mode)), // int64_t
			s_AMQPEnvelope);
	}


	if (envelope.message.properties._flags & AMQP_BASIC_PRIORITY_FLAG) {

		ob.o_set(
			s_priority,
			Variant(static_cast<int64_t>(envelope.message.properties.priority)), // int64_t
			s_AMQPEnvelope);
	}


	if (envelope.message.properties._flags & AMQP_BASIC_CORRELATION_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.correlation_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.correlation_id.bytes), envelope.message.properties.correlation_id.len));

			ob.o_set(
				s_correlation_id,
				v_tmp,
				s_AMQPEnvelope);

		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_REPLY_TO_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.reply_to.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.reply_to.bytes), envelope.message.properties.reply_to.len));

			ob.o_set(
				s_reply_to,
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_EXPIRATION_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.expiration.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.expiration.bytes), envelope.message.properties.expiration.len));

			ob.o_set(
				s_expiration,
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_MESSAGE_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.message_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.message_id.bytes), envelope.message.properties.message_id.len));

			ob.o_set(
				s_message_id,
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_TIMESTAMP_FLAG) {

		ob.o_set(
			s_timestamp,
			Variant(static_cast<int64_t>(envelope.message.properties.timestamp)), // int64_t
				s_AMQPEnvelope);
	}


	if (envelope.message.properties._flags & AMQP_BASIC_TYPE_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.type.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.type.bytes), envelope.message.properties.type.len));

			ob.o_set(
				s_type,
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}


	if (envelope.message.properties._flags & AMQP_BASIC_USER_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.user_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.user_id.bytes), envelope.message.properties.user_id.len));

			ob.o_set(
				s_user_id,
				Variant(v_tmp),
				s_AMQPEnvelope);
		}
	}

	if (envelope.message.properties._flags & AMQP_BASIC_APP_ID_FLAG) {

		v_tmp.setNull();
		if (envelope.message.properties.app_id.len) {
			v_tmp = Variant(std::string(static_cast<char*>(envelope.message.properties.app_id.bytes), envelope.message.properties.app_id.len));

			ob.o_set(
				s_app_id,
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

	this_->o_set( s_message, Variant(ob), s_AMQPQueue );
	
	return ob;
}


bool HHVM_METHOD(AMQPQueue, ack, int64_t delivery_tag, int64_t flags) {

	GET_CLASS_DATA_AND_CHECK( AMQPExchange );

	uint64_t _flags;
	_flags =  flags ? flags : this_->o_get(s_flags, false, s_AMQPQueue).toInt64();
	
	if (delivery_tag == NOPARAM ) {
	
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

	ANALYZE_RESPONSE_AND_RETURN();
}

bool HHVM_METHOD(AMQPExchange, declare){

	GET_CLASS_DATA_AND_CHECK( AMQPExchange );

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


	ANALYZE_RESPONSE_AND_RETURN();
}

bool HHVM_METHOD(AMQPExchange, delete){

	GET_CLASS_DATA_AND_CHECK( AMQPExchange );

	const char* exchange = const_cast<char* >(this_->o_get(s_name, false, s_AMQPExchange).toString().c_str());
	int64_t flags = this_->o_get(s_flags, false, s_AMQPExchange).toInt64();


	amqp_exchange_delete(
		data->amqpCh->amqpCnn->conn, 
		data->amqpCh->channel_id, 
		amqp_cstring_bytes(exchange), 
		(flags & AMQP_IFUNUSED)  ? 1 : 0);

	ANALYZE_RESPONSE_AND_RETURN();
}


bool HHVM_METHOD(AMQPExchange, publish, 
				const Variant& message, 
				const String& routing_key, 
				int64_t flags = AMQP_NOPARAM, 
				const Array& arguments = Array{}) {

	GET_CLASS_DATA_AND_CHECK( AMQPExchange );

	int64_t _flags = this_->o_get(s_flags, false, s_AMQPExchange).toInt64();

	if ( flags == NOPARAM)
		flags = _flags; 

	Array args = this_->o_get(s_arguments, false, s_AMQPExchange).toArray();
	
	amqp_bytes_t message_bytes;
	// switch (message.getType()) {
	// 	case KindOfString:
	// 	case KindOfStaticString: {
	// 		message_bytes = amqp_cstring_bytes(message.toString().c_str());
	// 		break;
	// 	}
	// 	case KindOfInt64: 
	// 		message_bytes = amqp_cstring_bytes(message.toString().c_str());
	// 		if (arguments.size() && arguments[s_headers] ) {
	
	// 			Variant hd = Variant(arguments[s_headers]);
	// 			hd.add(
	// 				s_x_type,
	// 				 Variant(s_int),
	// 				false); 
	// 		} else {
				// args.add(
				// 	s_x_type,
				// 	Variant("int"),
				// 	true);
			// }
			// break;
		// case KindOfDouble: 
		// 	message_bytes = amqp_cstring_bytes(message.toString().c_str());
		// 	if (arguments.size()) {
		// 		arguments.add(
		// 			s_x_type,
		// 			s_double,
		// 			true); 
		// 	} else {
		// 		args.add(
		// 			s_x_type,
		// 			s_double,
		// 			true);
		// 	}
		// 	break;
		// case KindOfNull: 
		// 	message_bytes = amqp_cstring_bytes("0");
		// 	if (arguments.size()) {
		// 		arguments.add(
		// 			s_x_type,
		// 			s_null,
		// 			true); 
		// 	} else {
		// 		args.add(
		// 			s_x_type,
		// 			s_null,
		// 			true);
		// 	}
		// 	break;
		// case KindOfBoolean: 
		// 	message_bytes = amqp_cstring_bytes(message.toBoolean() ? "1" : "0");
		// 	if (arguments.size()) {
		// 		arguments.add(
		// 			s_x_type,
		// 			s_bool,
		// 			true); 
		// 	} else {
		// 		args.add(
		// 			s_x_type,
		// 			s_bool,
		// 			true);
		// 	}
		// 	break;

	// 	default:
	// 		raise_warning("this type no implement");
	// }


	amqp_basic_properties_t props;
	if (arguments.size()) {

		Variant ct = Variant(arguments[s_content_type]);

		props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG;
		switch (ct.getType()) {
			case KindOfNull : 
				props.content_type = amqp_cstring_bytes("text/plain");
				break;
			case KindOfString :
			case KindOfStaticString :
				props.content_type = amqp_cstring_bytes( ct.toString().c_str() );
				break;
			default:
				raise_warning("arguments value key error");			
		}

		Variant ce = Variant(arguments[s_content_encoding]);
		ADD_AMQP_STRING_PROPERTY(ce, content_encoding, AMQP_BASIC_CONTENT_ENCODING_FLAG );
		
		Variant app_id = Variant(arguments[s_app_id]);
		ADD_AMQP_STRING_PROPERTY(app_id, app_id,AMQP_BASIC_APP_ID_FLAG );

		Variant user_id = Variant(arguments[s_user_id]);
		ADD_AMQP_STRING_PROPERTY(user_id, user_id,AMQP_BASIC_USER_ID_FLAG );

		Variant message_id = Variant(arguments[s_message_id]);
		ADD_AMQP_STRING_PROPERTY(message_id, message_id,AMQP_BASIC_USER_ID_FLAG );

		Variant correlation_id = Variant(arguments[s_correlation_id]);
		ADD_AMQP_STRING_PROPERTY(correlation_id, correlation_id,AMQP_BASIC_CORRELATION_ID_FLAG );

		Variant reply_to = Variant(arguments[s_reply_to]);
		ADD_AMQP_STRING_PROPERTY(reply_to, reply_to,AMQP_BASIC_REPLY_TO_FLAG );

		Variant type = Variant(arguments[s_type]);
		ADD_AMQP_STRING_PROPERTY(type, type, AMQP_BASIC_TYPE_FLAG );

		Variant ep = Variant(arguments[s_expiration]);
		ADD_AMQP_STRING_PROPERTY(ep, expiration, AMQP_BASIC_EXPIRATION_FLAG );

		Variant dm = Variant(arguments[s_delivery_mode]);
		ADD_AMQP_LONG_PROPERTY(dm, delivery_mode, AMQP_BASIC_DELIVERY_MODE_FLAG );
		if (dm.getType() == KindOfNull) { 
			props.delivery_mode = 1;
			props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
		}

		Variant pr = Variant(arguments[s_priority]);
		ADD_AMQP_LONG_PROPERTY(dm, priority, AMQP_BASIC_PRIORITY_FLAG );

		Variant ts = Variant(arguments[s_timestamp]);
		ADD_AMQP_LONG_PROPERTY(ts, timestamp, AMQP_BASIC_TIMESTAMP_FLAG );

		Variant hd = Variant(arguments[s_headers]);

		switch(hd.getType()) {
			case KindOfNull: break;
			case KindOfArray : {
				
				AMQP_TRACE
		// ??????	
				amqp_table_t *headers = (amqp_table_t *) malloc(sizeof(amqp_table_t));
				char type[16];
				headers->entries = (amqp_table_entry_t *) calloc( hd.toArray().size(), sizeof(amqp_table_entry_t));

				ArrayData* hdata = hd.toArray().get();
				for (ssize_t pos = hdata->iter_begin(); pos != hdata->iter_end();
							pos = hdata->iter_advance(pos)){
					const char* key = hdata->getKey(pos).toString().c_str();
					const Variant val = hdata->getValue(pos);
					amqp_table_entry_t *table;
					amqp_field_value_t *field;

					AMQP_TRACE
			
					table = &headers->entries[headers->num_entries++];
					field = &table->value;
					switch(val.getType()) {
						case KindOfBoolean:
							field->kind 			= AMQP_FIELD_KIND_BOOLEAN;
							field->value.boolean 	= (amqp_boolean_t) val.toBoolean();
							break;
						case KindOfDouble:
							field->kind 			= AMQP_FIELD_KIND_F64;
							field->value.f64 		= val.toDouble();
							break;
						case KindOfInt64:
							field->kind 			= AMQP_FIELD_KIND_I64;
							field->value.i64 		= val.toInt64();
							break;
						case KindOfString:
						case KindOfStaticString:
							field->kind        		= AMQP_FIELD_KIND_UTF8;
							// strValue           = ; // strndup()
							field->value.bytes 		= amqp_cstring_bytes(  val.toString().c_str());
							break;
						case KindOfArray:
							// field->kind = AMQP_FIELD_KIND_TABLE;
							raise_warning("the field array is not implement");
							break;
						default:
							switch(val.getType()) {
								case KindOfNull:	 strcpy(type, "null"); break;
								case KindOfObject:	 strcpy(type, "object"); break;
								case KindOfResource: strcpy(type, "resource"); break;
								default:			 strcpy(type, "unknown");
							}
					}
				

				table->key = amqp_cstring_bytes(key);


				}

				props.headers = *headers;
				props._flags |= AMQP_BASIC_HEADERS_FLAG;
				break;
			}
			default:
				raise_warning("error header type, must be Array");
				printf("type=%d\n", hd.getType());
		}

	} else {

		// Array args = this_->o_get(s_arguments, false, s_AMQPExchange).toArray();

		Variant ct = Variant(args[s_content_type]);

		props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG;
		switch (ct.getType()) {
			case KindOfNull : 
				props.content_type = amqp_cstring_bytes("text/plain");
				break;
			case KindOfString :
			case KindOfStaticString :
				props.content_type = amqp_cstring_bytes( ct.toString().c_str() );
				break;
			default:
				raise_warning("arguments value key error");			
		}

		Variant ep = Variant(args[s_expiration]);
		ADD_AMQP_STRING_PROPERTY(ep, expiration, AMQP_BASIC_EXPIRATION_FLAG );

		Variant ce = Variant(args[String("content_encoding")]);
		ADD_AMQP_STRING_PROPERTY(ce, content_encoding, AMQP_BASIC_CONTENT_ENCODING_FLAG );
		
		Variant app_id = Variant(args[s_app_id]);
		ADD_AMQP_STRING_PROPERTY(app_id, app_id, AMQP_BASIC_APP_ID_FLAG );

		Variant user_id = Variant(args[s_user_id]);
		ADD_AMQP_STRING_PROPERTY(user_id, user_id, AMQP_BASIC_USER_ID_FLAG );

		Variant message_id = Variant(args[s_message_id]);
		ADD_AMQP_STRING_PROPERTY(message_id, message_id, AMQP_BASIC_USER_ID_FLAG );

		Variant correlation_id = Variant(args[s_correlation_id]);
		ADD_AMQP_STRING_PROPERTY(correlation_id, correlation_id, AMQP_BASIC_CORRELATION_ID_FLAG );

		Variant reply_to = Variant(args[s_reply_to]);
		ADD_AMQP_STRING_PROPERTY(reply_to, reply_to, AMQP_BASIC_REPLY_TO_FLAG );

		Variant tp = Variant(args[s_type]);
		ADD_AMQP_STRING_PROPERTY(tp, type, AMQP_BASIC_TYPE_FLAG );

		Variant dm = Variant(args[s_delivery_mode]);
		ADD_AMQP_LONG_PROPERTY(dm, delivery_mode, AMQP_BASIC_DELIVERY_MODE_FLAG );
		if (dm.getType() == KindOfNull) { 
			props.delivery_mode = 1;
			props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
		}

		Variant pr = Variant(args[s_priority]);
		ADD_AMQP_LONG_PROPERTY(pr, priority, AMQP_BASIC_PRIORITY_FLAG );

		Variant ts = Variant(args[s_timestamp]);
		ADD_AMQP_LONG_PROPERTY(ts, timestamp, AMQP_BASIC_TIMESTAMP_FLAG );
	
	} // end if 


	const char* exchange = const_cast<char* >(this_->o_get(s_name, false, s_AMQPExchange).toString().c_str());

	amqp_basic_publish(data->amqpCh->amqpCnn->conn,
			data->amqpCh->channel_id,
			amqp_cstring_bytes(exchange),
			amqp_cstring_bytes(routing_key.c_str()),
			(flags & AMQP_MANDATORY)  ? 1 : 0, 		// mandatory
			(flags & AMQP_IMMEDIATE)  ? 1 : 0,			// immediate
			&props,
			message_bytes);



	ANALYZE_RESPONSE_AND_RETURN();
}

HHVM_GET_MODULE(amqp);
} // namespace
