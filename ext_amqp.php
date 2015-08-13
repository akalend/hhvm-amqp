<?hh

class AMQPConnection {
	
	/* internal */
	private $host = 'localhost';
	private $port = 5672;
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

	public function  disconnect ( ){}
	
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

	public function  isConnected ( ){}
	public function  reconnect ( ){}

	  <<__Native>>
	public function  connect (): bool;

	
}


 AMQPChannel {
	/* Methods */	
	function __construct ( $amqp_connection ) {}
	public function commitTransaction () {}
	public function isConnected ( void ): void
	public function qos ( int $size , int $count ) {}
	public function rollbackTransaction ( void ) {}
	public function setPrefetchCount ( int $count ) {}
	public function setPrefetchSize ( int $size ) {}
	public function startTransaction ( void ) {}
}
