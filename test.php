<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['port'=>  AMQP_PORT]);
	$ret = $cnn->connect();
	var_dump($ret);

	$ch = new AMQPChannel($cnn);


	var_dump($cnn->isConnected());

	var_dump($ch->isConnected());

	var_dump($cnn->disconnect(AMQP_NOPARAM));

