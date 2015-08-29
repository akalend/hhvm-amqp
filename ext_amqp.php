<?hh
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


/**
*  class of AMQP constants
*/
final class AMQP {
	const  PORT = 5672;
	const  NOACK = 1;
	const  NOPARAM = 0;
	const  PASSIVE = 2;			// passive
	const  DURABLE = 4;			// durable 
	const  EXCLUSIVE = 8;		// exclusive
	const  AUTODELETE = 16;		// autodelete
	const  IF_UNUSED = 32;
	const  IF_EMPTY = 64;
	const  AUTOACK = 128;
}

/**
*	AMQPConnection class
*
*/
<<__NativeData("AMQPConnection")>>
class AMQPConnection {
	
	/* internal */
	private $host = 'localhost';
	private $port = 5672; // `
	private $login = 'guest';
	private $vhost = '/';
	private $password = 'guest';

	private $timeout = 15;
	private $connect_timeout = 1;	
	private $is_persisten = 1;


	/* Methods */
	public  function __construct (array $parms = []){ 
		if (isset($parms['host']))
			$this->host = $parms['host'];
		
		if (isset($parms['port']))
			$this->port = $parms['port'];
		
		if (isset($parms['login']))
			$this->login = $parms['login'];
		
		if (isset($parms['vhost']))
			$this->vhost = $parms['vhost'];
		
		if (isset($parms['password']))
			$this->password = $parms['password'];

		if (isset($parms['is_persisten']))
			$this->is_persisten = $parms['is_persisten'];

		if (isset($parms['timeout']))
			$this->timeout = $parms['timeout'];

		if (isset($parms['connect_timeout']))
			$this->connect_timeout = $parms['connect_timeout'];

	}

	public function  setHost ( string $host ){
		$this->host = $host;
	}

	public function  setLogin ( string $login ){
		$this->login = $login;
	}

	public function  setPassword ( string $password ){
		$this->password = $password;
	}

	public function  setPort ( int $port ){
		$this->port = $port;
	}

	public function  setVhost ( string $vhost ){
		$this->vhost = $vhost;
	}
	
	public function  getHost (){
		return $this->host;
	}

	public function  getLogin (){
		return $this->login;
	}

	public function  getPassword (){
		return $this->password;
	}

	public function  getPort ( ){
		return $this->port;
	}

	public function  getVhost (){
		return $this->vhost;
	}

	<<__Native>>
	public function  disconnect(int $parm = 0 ): bool;
	
	<<__Native>>
	public function  reconnect(): bool;
	
	<<__Native>>
	public function  isConnected(): bool;	

	  <<__Native>>
	public function  connect(): bool;

}

<<__NativeData("AMQPConnection")>>
class AMQPChannel {

 	/* Properties  */


	 /* Methods 	*/
	<<__Native>>
	public  function __construct ( AMQPConnection $amqp_connection ) : void;
	
	<<__Native>>
	public function isConnected (): bool;

	// public function commitTransaction () {}
	
	// public function qos ( int $size , int $count ) {}
	// public function rollbackTransaction ( void ) {}
	// public function setPrefetchCount ( int $count ) {}
	// public function setPrefetchSize ( int $size ) {}
	// public function startTransaction ( void ) {}
}


<<__NativeData("AMQPQueue")>>
class AMQPQueue {

	/* internal */
	private $name = '';
	private $flags = 0;

	/* Methods */
	<<__Native>>
	public function __construct (AMQPChannel $amqp_channel );

	
 	public function getName (){
 		return $this->name;
 	}
	
	public function setName ( string $queue_name ){
		$this->name	= $queue_name;
	}


	<<__Native>>
	public function bind ( string $exchange_name , string $routing_key ) : void;

	<<__Native>>
	public function declare (): int;

	<<__Native>>
	public function delete (): int;

	<<__Native>>
	public function get (int $flags=AMQP_NOACK) : mixed;

	
	public function getFlags () {
		return $this->flags;
	}

	public function setFlags ( int $flags ){
		$this->flags = $flags;
	}

	

	// public function ack ( int $delivery_tag , int $flags = AMQP_NOPARAM){}
	// public function cancel ([ string $consumer_tag = "" ] ){}
	// public function consume ( callable $callback [, int $flags = AMQP_NOPARAM ] ){}

	// public function getArgument ( string $key ): mixed
	// public function getArguments ( void ): array
	// public function nack ( string $delivery_tag, string $flags = AMQP_NOPARAM  ){}
	// public function purge ( void ){}
	// public function setArgument ( string $key , mixed $value ){}
	// public function setArguments ( array $arguments ){}
	// public function unbind ( string $exchange_name , string $routing_key ){}
}

<<__NativeData("AMQPConnection")>>
class AMQPEnvelope {
/* Metody */
// public string getAppId ( void )
// public string getBody ( void )
// public string getContentEncoding ( void )
// public string getContentType ( void )
// public string getCorrelationId ( void )
// public string getDeliveryTag ( void )
// public string getExchange ( void )
// public string getExpiration ( void )
// public string getHeader ( string $header_key )
// public array getHeaders ( void )
// public string getMessageId ( void )
// public string getPriority ( void )
// public string getReplyTo ( void )
// public string getRoutingKey ( void )
// public string getTimeStamp ( void )
// public string getType ( void )
// public string getUserId ( void )
// public bool isRedelivery ( void )
}



//  AMQPExchange {
// /* Methods */
// public bool bind ( string $destination_exchange_name , string $source_exchange_name , string $routing_key )
// public __construct ( AMQPChannel $amqp_channel )
// public int declare ( void )
// public bool delete ([ int $flags = AMQP_NOPARAM ] )
// public mixed getArgument ( string $key )
// public array getArguments ( void )
// public int getFlags ( void )
// public string getName ( void )
// public string getType ( void )
// public bool publish ( string $message , string $routing_key [, int $flags = AMQP_NOPARAM [, array $attributes = array() ]] )
// public void setArgument ( string $key , mixed $value )
// public void setArguments ( array $arguments )
// public void setFlags ( int $flags )
// public void setName ( string $exchange_name )
// public string setType ( string $exchange_type )
// }



class AMQPException extends Exception {
}
