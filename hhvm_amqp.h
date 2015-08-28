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
// String HHVM_METHOD(AMQPQueue, getName); 
// void HHVM_METHOD(AMQPQueue, setName, const String& name); 


enum amqp_error_code {
	AMQP_ERR_NONE = 0,
	AMQP_ERR_CANNOT_OPEN_SOCKET,
	AMQP_ERR_CANNOT_CREATE_SOCKET,
	AMQP_ERROR_LOGIN
};

enum amqp_param {
	AMQP_NOPARAM = 0,
	AMQP_NOACK
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

  	amqp_channel_t channel_id = 0;
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