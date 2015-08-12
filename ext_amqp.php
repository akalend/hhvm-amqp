<?hh

class AMQPConnection {
	
	/* internal */
	private $host = 'localhost';
	private $port = 5672;
	private $login = 'quest';
	private $vhost = '/';
	private $password = '';


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

	public function  connect ( ){}

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
	
}
