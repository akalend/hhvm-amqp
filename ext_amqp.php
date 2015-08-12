<?hh

class AMQPConnection {
	/* Methods */
	
	public  function __construct (){ }

	public function  connect ( array $parms = []){}
	public function  disconnect ( ){}
	public function  getHost ( ){}
	public function  getLogin ( ){}
	public function  getPassword ( ){}
	public function  getPort ( ){}
	public function  getVhost ( ){}
	public function  isConnected ( ){}
	public function  reconnect ( ){}
	public function  setHost ( string $host ){}
	public function  setLogin ( string $login ){}
	public function  setPassword ( string $password ){}
	public function  setPort ( int $port ){}
	public function  setVhost (  $vhost ){}

	/* internal */
	private $host = 'localhost';
	private $port = '5672';
	private $Login = 'quest';
	private $password = '';
	private $vhost = '/';
	private $parms = [];
}
