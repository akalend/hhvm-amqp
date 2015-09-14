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

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

namespace HPHP {

#define AMQP_PORT  5672
#define AMQP_MAX_CHANNELS 65535 


bool HHVM_METHOD(AMQPConnection, connect);
bool HHVM_METHOD(AMQPConnection, isConnected);
bool HHVM_METHOD(AMQPConnection, reconnect);
bool HHVM_METHOD(AMQPConnection, disconnect, int64_t parm);


void HHVM_METHOD(AMQPChannel, __construct, const Variant& amqpConnect);
bool HHVM_METHOD(AMQPChannel, isConnected);

void HHVM_METHOD(AMQPQueue, __construct, const Variant& amqpQueue);
void HHVM_METHOD(AMQPQueue, bind, const String& exchangeName, const String& routingKey);
int64_t HHVM_METHOD(AMQPQueue, declare);
int64_t HHVM_METHOD(AMQPQueue, delete);
Variant HHVM_METHOD(AMQPQueue, get);
bool HHVM_METHOD(AMQPQueue, ack, int64_t delivery_tag, int64_t flags);


void HHVM_METHOD(AMQPExchange, __construct, const Variant& amqpQueue);
bool HHVM_METHOD(AMQPExchange, bind, const String& queueName, const String& routingKey);
bool HHVM_METHOD(AMQPExchange, declare);
bool HHVM_METHOD(AMQPExchange, delete);
bool HHVM_METHOD(AMQPExchange, publish, const String& message, const String& routing_key, int64_t flags, const Array& arguments);

enum amqp_error_code {
	AMQP_ERR_NONE = 0,
	AMQP_ERR_CANNOT_OPEN_SOCKET,
	AMQP_ERR_CANNOT_CREATE_SOCKET,
	AMQP_ERROR_LOGIN
};

 
enum amqp_param {
	AMQP_NOPARAM 	= 0,
	AMQP_NOACK 		= 1,
	AMQP_PASSIVE 	= 2,		// passive
	AMQP_DURABLE 	= 4,		// durable 
	AMQP_EXCLUSIVE 	= 8,		// exclusive
	AMQP_AUTODELETE = 16,	// autodelete
	AMQP_IFUNUSED 	= 32,
	AMQP_IFEMPTY 	= 64,
	AMQP_AUTOACK 	= 128,
	AMQP_MULTIPLE 	= 256,
	AMQP_INTERNAL 	= 512,
	AMQP_MANDATORY 	= 1024,
	AMQP_IMMEDIATE	= 2048,
	AMQP_NOWAIT 	= 4096;
	AMQP_REQUEUE 	= 8192;
	AMQP_NOLOCAL 	= 16384;
};


class AMQPConnection {
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
	short channel_id = 0;


	AMQPConnection() { /* new AMQPConnection */ }
	AMQPConnection(const AMQPConnection&) = delete;
	AMQPConnection& operator=(const AMQPConnection& src) {
	/* clone $instanceOfAMQPConnection */
		throw Object(SystemLib::AllocExceptionObject(
			  "Cloning AMQPConnection is not allowed"
	));
  }

  ~AMQPConnection() {};

};


class AMQPChannel {
 public:

	AMQPChannel(){};

	AMQPChannel(const AMQPChannel&) = delete;	
	AMQPChannel& operator=(const AMQPChannel& src) {
	/* clone $instanceOfAMQPConnection */
		throw Object(SystemLib::AllocExceptionObject(
			  "Cloning AMQPChannel is not allowed"
	));
  }

  ~AMQPChannel() {};
	
	int used_slots = 0;
	int prefetch_count = 0;

	amqp_channel_t channel_id = 1;
	amqp_channel_t *slots;
	AMQPConnection* amqpCnn = NULL;
};



class AMQPQueue {
 public:

	AMQPQueue(){};	

	AMQPQueue(const AMQPQueue&) = delete;	
	AMQPQueue& operator=(const AMQPQueue& src) {
	/* clone $instanceOfAMQPConnection */
		throw Object(SystemLib::AllocExceptionObject(
			  "Cloning AMQPConnection is not allowed"
	));
  }

  ~AMQPQueue() {};
	
	int parms = AMQP_AUTODELETE;
	int message_count = 0;
	int consumer_count = 0;
	AMQPChannel* amqpCh = NULL;
	char* name = NULL;
	Object envelope; 
};


// class AMQPEnvelope {
// private:
// 	amqp_envelope_t _envelope;

// public:

// 	int refCount=0;

// 	AMQPEnvelope(){};	

// 	AMQPEnvelope(const AMQPEnvelope&) = delete;	

// 	AMQPEnvelope(const amqp_envelope_t envelope) {
// 		refCount = 0;
// 		_envelope = envelope;
// 	}

// 	AMQPEnvelope& operator=(const AMQPEnvelope& src) {
// 	 clone $instanceOfAMQPConnection 
// 		throw Object(SystemLib::AllocExceptionObject(
// 			  "Cloning AMQPConnection is not allowed"
// 		));
//   	}

//   int incRefCount() {
//   	return ++refCount;
//   }

//   int decRefAndRelease() {
//   	--refCount;
//   	if (refCount == 0) {
//   		// release
//   	}
//   	return refCount;
//   }
// };

class AMQPExchange {
 public:

	AMQPExchange(){};	

	AMQPExchange(const AMQPExchange&) = delete;	
	AMQPExchange& operator=(const AMQPExchange& src) {
	/* clone $instanceOfAMQPConnection */
		throw Object(SystemLib::AllocExceptionObject(
			  "Cloning AMQPConnection is not allowed"
	));
  }

  ~AMQPExchange() {};
	
	int parms = AMQP_AUTODELETE;
	AMQPChannel* amqpCh = NULL;
	char* name = NULL;
};



class AmqpExtension : public Extension {

	public:
		AmqpExtension(): Extension("amqp", "0.1.0"){}
	
		void moduleInit() override;
		void moduleShutdown() override;

};


} // end namespace