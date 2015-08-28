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
int HHVM_METHOD(AMQPQueue, declare);
int HHVM_METHOD(AMQPQueue, delete);

enum amqp_error_code {
	AMQP_ERR_NONE = 0,
	AMQP_ERR_CANNOT_OPEN_SOCKET,
	AMQP_ERR_CANNOT_CREATE_SOCKET,
	AMQP_ERROR_LOGIN
};


enum amqp_param {
	AMQP_NOPARAM = 0,
	AMQP_NOACK = 1,
	AMQP_PASSIVE = 2,		// passive
	AMQP_DURABLE = 4,		// durable 
	AMQP_EXCLUSIVE = 8,		// exclusive
	AMQP_AUTODELETE = 16,	// autodelete
	AMQP_IFUNUSED = 32,
	AMQP_IFEMPTY = 64,
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
			  "Cloning AMQPConnection is not allowed"
	));
  }

  ~AMQPChannel() {

	printf("destructor %s\n", __FUNCTION__ );
  };
	
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

  ~AMQPQueue() {

	printf("destructor %s\n", __FUNCTION__ );
  };
	
	int parms = AMQP_AUTODELETE;
	int message_count = 0;
	int consumer_count = 0;
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