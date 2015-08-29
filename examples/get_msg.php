<?php
// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();

	$ch = new AMQPChannel($cnn);
	$queue = new AMQPQueue($ch);

// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("test_queue");
	$msg = $queue->get();
	
	var_dump($msg);
	
	$cnn->disconnect();
