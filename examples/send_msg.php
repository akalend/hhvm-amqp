<?php
// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['port'=>5677]);
	// $cnn = new AMQPConnection();
	echo "------  connect ...  --------\n";

	$ret = $cnn->connect();

	echo "------  connect Ok  --------\n";

	$ch = new AMQPChannel($cnn);
	$ex = new AMQPExchange($ch);

	echo "------  exchange Ok  --------\n";

// $cnn->disconnect(AMQP_NOPARAM);
	$ex->setName('test_e');
	
	define('PLAIN', 'text/plain');

	$ex->setArgument('content_type', 'text/json');
	// $ex->setArgument('headers', ['xxx'=>123]);
	$headers = ['sss'=> 'asd', 'xxx'=> 321];

	echo "------  publish ...  --------\n";

	$ex->publish(true,
			'scan', 
			AMQP::NOPARAM,
			['content_type'=> 'text/xml', 	'headers' => $headers]);

// $testExchange->publish($msg, 'scan', AMQP_NOPARAM, 
// 	['headers' => ['x-model'=>'object', 'x-type' => 123]]);


	echo "------  publish Ok  --------\n";

	$cnn->disconnect();