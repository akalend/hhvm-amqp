<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();
	var_dump($ret);

	$ch = new AMQPChannel($cnn);


	var_dump($ch->isConnected());
	var_dump($cnn->disconnect(AMQP_NOPARAM));
	var_dump($ch->isConnected());
// $cnn->disconnect(AMQP_NOPARAM);
