<?hh

 AMQPChannel {
	 /* Methods 	*/
	public  function __construct ( AMQPConnection $amqp_connection ) {
		$this->cnn = $amqp_connection;
	}
	
	<<__Native>>
	public function isConnected (): bool;

	// public function commitTransaction () {}
	
	// public function qos ( int $size , int $count ) {}
	// public function rollbackTransaction ( void ) {}
	// public function setPrefetchCount ( int $count ) {}
	// public function setPrefetchSize ( int $size ) {}
	// public function startTransaction ( void ) {}
}
