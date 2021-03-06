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

#define AMQP_TRACE printf("%s:%d\n", __FUNCTION__, __LINE__);


#define AMQP_PORT  5672
#define AMQP_MAX_CHANNELS 64    //max 65535 

enum amqp_param {
	AMQP_ERROR 		= -1,
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
	AMQP_NOWAIT 	= 4096,
	AMQP_REQUEUE 	= 8192,
	AMQP_NOLOCAL 	= 16384
};


bool HHVM_METHOD(AMQPContext, connect);
void HHVM_METHOD(AMQPContext, init);


void HHVM_METHOD(AMQPConnection, __destruct);
bool HHVM_METHOD(AMQPConnection, connect);
bool HHVM_METHOD(AMQPConnection, isConnected);
bool HHVM_METHOD(AMQPConnection, reconnect);
bool HHVM_METHOD(AMQPConnection, disconnect, int64_t parm);
void HHVM_METHOD(AMQPConnection, init);

void HHVM_METHOD(AMQPChannel, __construct, const Variant& amqpConnect);
void HHVM_METHOD(AMQPChannel, __destruct);

bool HHVM_METHOD(AMQPChannel, isConnected);

void HHVM_METHOD(AMQPQueue, __construct, const Variant& amqpQueue);
void HHVM_METHOD(AMQPQueue, bind, const String& exchangeName, const String& routingKey);
int64_t HHVM_METHOD(AMQPQueue, declare);
int64_t HHVM_METHOD(AMQPQueue, delete);
Variant HHVM_METHOD(AMQPQueue, get, int64_t flag = AMQP_NOPARAM);
bool HHVM_METHOD(AMQPQueue, ack, int64_t delivery_tag, int64_t flags);
bool HHVM_METHOD(AMQPQueue, cancel, const String& consumer_tag);

void HHVM_METHOD(AMQPExchange, __construct, const Variant& amqpQueue);
bool HHVM_METHOD(AMQPExchange, bind, const String& queueName, const String& routingKey);
bool HHVM_METHOD(AMQPExchange, declare);
bool HHVM_METHOD(AMQPExchange, delete);
bool HHVM_METHOD(AMQPExchange, publish, const Variant& message, const String& routing_key, int64_t flags, const Array& arguments);

enum amqp_error_code {
	AMQP_ERR_NONE = 0,
	AMQP_ERR_CANNOT_OPEN_SOCKET,
	AMQP_ERR_CANNOT_CREATE_SOCKET,
	AMQP_ERROR_LOGIN,
	AMQP_ERR_CHANNEL_CLOSE
};


enum amqp_channel_status {
	AMQP_CHANNEL_CLOSED = 0,
	AMQP_CHANNEL_OPENED = 1,
	AMQP_CHANNEL_RECLOSED = 2,
};
 


// struct CurlMultiResource : SweepableResourceData {
//   DECLARE_RESOURCE_ALLOCATION(CurlMultiResource)
//   CLASSNAME_IS("curl_multi")
//   const String& o_getClassNameHook() const override { return classnameof(); }
//   bool isInvalid() const override { return !m_multi; }

//   CurlMultiResource();
//   ~CurlMultiResource() { close(); }
//   void close();

//   bool setOption(int option, const Variant& value);
//   void add(const Resource& ch) { m_easyh.append(ch); }
//   const Array& getEasyHandles() const { return m_easyh; }

//   void remove(req::ptr<CurlResource> curle);
//   Resource find(CURL *cp);

//   CURLM* get();
//   void check_exceptions();

//  private:
//   CURLM *m_multi;
//   Array m_easyh;
// };


class AMQPContext {

public:
	
	bool is_connected = false;
	char* host = NULL;
	char* vhost = NULL;
	char* password = NULL;
	char* login = NULL;
	short port = AMQP_PORT;
	short err = 0;
	short channel_id = 0;
	short max_id = 0;
	
private:
	int8_t* opened_channels{nullptr};

	std::shared_ptr<amqp_connection_state_t> m_conn;

	void initChannels() {
		opened_channels = static_cast<int8_t*>(calloc(AMQP_MAX_CHANNELS, sizeof(int8_t)));
		channel_id = 0;	
	}

	void deinitChannels() {
		free(opened_channels);
	}

public:
  	AMQPContext(); 
  	~AMQPContext();

private:
  void *m_context;

	amqp_socket_t *socket{nullptr};
	amqp_connection_state_t conn = NULL;

};

class SocketData : public ResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(SocketData)
 
  SocketData(void *context, int64_t type) {
    m_socket = nullptr; //zmq_socket(context, type);
  }
  
  ~SocketData() {
    close();
  }        
          
  CLASSNAME_IS("SocketData");
  // overriding ResourceData
  const String& o_getClassNameHook() const { return classnameof(); }

  void close() {
    if (!isValid())
        return;
    
    //zmq_close(m_socket);
    m_socket = nullptr;
  }

  bool isValid() { return m_socket != nullptr; }
  
  void *get() { return m_socket; }

 private:
  void *m_socket;
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
	short max_id = 0;

	AMQPConnection() { /* new AMQPConnection */ }
	AMQPConnection(const AMQPConnection&) = delete;
	AMQPConnection& operator=(const AMQPConnection& src) {
	/* clone $instanceOfAMQPConnection */
		throw Object(SystemLib::AllocExceptionObject(
			  "Cloning AMQPConnection is not allowed"
	));
  }

  ~AMQPConnection() {};

  	void init() {
		is_connected = false;
		conn = NULL;
		max_id = 0;
		channel_id = 0;
  	}

  	int incChannel() {
  		channel_id++;

  			printf("channel_id=%d max_id=%d\n", channel_id,max_id);

  		if (channel_id > max_id) 
  			max_id = channel_id;
  		  	printf("max_id=%d\n", max_id);

  		return channel_id;
  	}

	void initChannels() {
		AMQP_TRACE;
		channel_open = static_cast<int8_t*>(calloc(AMQP_MAX_CHANNELS, sizeof(int8_t)));
		channel_id = 0;	
	}

	void deinitChannel() {
		AMQP_TRACE;
		free(channel_open);
		channel_open = NULL;
	}

	bool getChannel(int num) {
		if (num >= AMQP_MAX_CHANNELS) return AMQP_ERROR;		
		return  static_cast<bool>(*(channel_open + num));
	}

	void setChannel(int num) {
		if (num >= AMQP_MAX_CHANNELS) return;
		*(channel_open + num) = 1;		
	}

	void resetChannel(int num) {
		if (num >= AMQP_MAX_CHANNELS) return;
		*(channel_open + num) = 0;
	}


	void channelClose(int channel_id) {

		if (getChannel(channel_id)){
			resetChannel(channel_id);

			if (channel_id == max_id)
				max_id--;
				printf("max_id=%d\n", max_id);
		}
	}


 private:
	int8_t* channel_open;

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
	
	int is_open = 0;
	amqp_channel_t channel_id = 1;
	AMQPConnection* amqpCnn = NULL;

};


class AMQPCannelContext  {
public:
  static AMQPCannelContext *GetPersistent(int64_t io_threads);
  static void SetPersistent(int64_t io_threads, AMQPCannelContext *context);

private:
  static std::string GetHash(const char *name, int64_t io_threads);
  static AMQPCannelContext *GetCachedImpl(const char *name, int64_t io_threads);
  static void SetCachedImpl(const char *name, int64_t io_threads, AMQPCannelContext *context);

public:
  AMQPCannelContext(int64_t io_threads);
  ~AMQPCannelContext();

  CLASSNAME_IS("AMQPChannelContext")

  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }
  virtual bool isInvalid() const { return m_context == nullptr; }

  void *get() { return m_context; }

private:
  void *m_context;

};



//AMQPContext *get_context(Object obj);



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