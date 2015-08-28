<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();

	$ch = new AMQPChannel($cnn);
	$queue = new AMQPQueue($ch);

// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("mamamama");
	echo "--------------\n";

	// $queue->declare();
	
	echo "------  bind  --------\n";

	$queue->bind("exxx", "kkk");	
	echo "------  disconnect  --------\n";



	var_dump($cnn->disconnect(AMQP::NOPARAM));
