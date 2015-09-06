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
	const  MULTIPLE = 256;
	const  INTERNAL = 512;
	const  TYPE_DIRECT = 'direct';
	const  TYPE_FANOUT = 'fanout';
	const  TYPE_TOPIC  = 'topic';
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


class AMQPEnvelope  {

	/* internal */
	private $delivery_tag = NULL;
	private $exchange = NULL;
	private $consumer_tag = NULL;
	private $routing_key = NULL;
	private $message = NULL;
	private $channel = NULL;
	//private $content_type = NULL;
	private $redelivered = NULL;
	private $app_id = NULL;
	private $cluster_id = NULL;
	private $user_id = NULL;
	private $expiration = NULL;
	private $timestamp = NULL;
	private $body = NULL;
	private $type = NULL;
	private $message_id = NULL;
	private $reply_to = NULL;
	private $correlation_id = NULL;
	private $priority = NULL;
	private $delivery_mode = NULL;
	private $content_encoding = NULL;
	private $content_type = NULL;

	/* Metody */

	public function getDeliveryTag(){
		return $this->delivery_tag;
	}

	public function getAppId (){
		return $this->app_id;
	}

	public function getBody ( ) {
		$this->body;
	}

	public function getContentEncoding () {
		return $this->content_encoding;
	}

	public function getContentType () {
		return $this->content_type;
	}

	public function getCorrelationId () {
		return $this->correlation_id;
	}

	public function getExchange () {
		return $this->exchange;
	}

	public function getExpiration () {
		return $this->expiration;
	}

	public function getMessageId () {
		return $this->message_id;
	}

	public function getPriority () {
		return $this->priority;
	}

	public function getReplyTo () {
		return $this->reply_to;
	}

	public function getRoutingKey () {
		return $this->routing_key;
	}

	public function getTimeStamp () {
		return $this->timestamp;
	}

	public function getType () {
		return $this->type;
	}

	public function getUserId () {
		return $this->user_id;
	}

	public function isRedelivery () {
		return $this->redelivered;
	}

	// public function getHeader ( string $header_key )
	//public array getHeaders ( void )
}


<<__NativeData("AMQPQueue")>>
class AMQPQueue {

	/* internal */
	private $name = '';
	private $flags = 0;
	private $message = NULL;

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
	public function ack (int $delivery_tag = -1, int $flags = AMQP_NOPARAM) : bool;

	<<__Native>>
	public function delete (): int;

	<<__Native>>
	public function get (int $flags=AMQP_NOACK) : mixed;


	public function getMessage () {
		return $this->message;
	}

	public function getFlags () {
		return $this->flags;
	}

	public function setFlags ( int $flags ){
		$this->flags = $flags;
	}

	
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

<<__NativeData("AMQPExchange")>>
class AMQPExchange {
	
	private $name = NULL;
	private $type = NULL;
	private $flags = NULL;


	/* Methods */
	<<__Native>>
	public function __construct ( AMQPChannel $amqp_channel );


	public function setName ( string $exchange_name ) {
		$this->name = $exchange_name;
	}

	public function getName () {
		return $this->name;
	}

	public function setType ( string $exchange_type ) {
		$this->type = $exchange_type;
	}

	public function getType () {
		return $this->type;
	}

	public function setFlags ( int $flags ) {
		$this->flags = $flags;
	}

	public function getFlags () {
		return $this->flags;
	}

	<<__Native>>
	public function bind ( string $destination_queue_name , string $routing_key ) : bool;

	<<__Native>>
	public function declare(): bool;

	
// public bool delete ([ int $flags = AMQP_NOPARAM ] )
// public mixed getArgument ( string $key )
// public array getArguments ( void )


// public bool publish ( string $message , string $routing_key [, int $flags = AMQP_NOPARAM [, array $attributes = array() ]] )
// public void setArgument ( string $key , mixed $value )
// public void setArguments ( array $arguments )

}



class AMQPException extends Exception {
}
