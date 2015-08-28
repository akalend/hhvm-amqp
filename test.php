<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();
	var_dump($ret);


	$ch = new AMQPChannel($cnn);

	$queue = new AMQPQueue($ch);


	var_dump($ch->isConnected());
	var_dump($cnn->disconnect(AMQP_NOPARAM));
	var_dump($ch->isConnected());
// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("mamamama");
	var_dump($queue->getName());
	$queue->bind("xxxxxx", "kkk");
