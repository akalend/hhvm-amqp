#!/usr/bin/hhvm
<?php
	// print_r( get_loaded_extensions() );

class User {
 	private $name = '';
	private $login = '';
 	private $password = 12345;

 	function __construct($name,$login){
 		$this->name = $name;
 		$this->login = $login;
 	}
 	
 	function getPswd() {
 		return $this->password;
 	}

 	function setPswd($password) {
 		return $this->password = $password;
 	}

}

// $user = new User('Sasha' ,'kalendarev');

printf("--------- %d  ---------\n", __LINE__);

	$cnn = new AMQPConnection(['port'=>5677]);
// printf("--------- %d  ---------\n", __LINE__);

	// $cnn = new AMQPConnection();
	printf("--------- %d  ---------\n", __LINE__);
	var_dump($cnn->isConnected());
//exit;

	printf("--------- %d  ---------\n", __LINE__);

	$ret = $cnn->connect();
	echo "-------- connect Ok ------\n";
	var_dump($cnn->isConnected());

	// printf("--------- %d  ---------\n", __LINE__);
	// $channel = new AMQPChannel($cnn);


	printf("--------- %d  ---------\n", __LINE__);
	$channel2 = new AMQPChannel($cnn);

	printf("--------- %d  ---------\n", __LINE__);

	$queue = new AMQPQueue($channel2);

	printf("--------- %d  ---------\n", __LINE__);

	$queue->setName("test_q");


	printf("--------- %d  ---------\n", __LINE__);

	$queue->declare();
	printf("--------- %d  ---------\n", __LINE__);

	// $queue->setFlags(AMQP::AUTOACK);
	$res = $queue->get(AMQP::AUTOACK); // AMQP::AUTOACK ????

	printf("--------- %d  ---------\n", __LINE__);
	
	var_dump($res);
	printf("--------- %d  ---------\n", __LINE__);

	// $queue->ack();

// printf("--------- %d  ---------\n, __LINE__");
// $testExchange = new AMQPExchange($channel2);

// $testExchange->setName('test_e');
// // $testExchange->delete();

// $testExchange->setType('direct');
// printf("--------- %d  ---------\n", __LINE__);
// $testExchange->declare();
// printf("--------- %d  ---------\n", __LINE__);

	// $cnn->disconnect();

// 	printf("--------- %d  ---------\n", __LINE__);

























	$cnn->disconnect();

	exit();



	if ($message)
	 	var_dump($message->getHeader('sss'));

printf("--------- %d  ---------\n", __LINE__);
	var_dump($queue->getArguments());
	// var_dump($queue);

// 	$msg = $queue->getMessage();
// 	var_dump($message->getContentType());

// printf("--------- %d  ---------\n", __LINE__);

	// $ret = $queue->ack();
// 	var_dump($ret);
// printf("--------- %d  ---------\n", __LINE__);
// var_dump($cnn);
	$cnn->disconnect();
