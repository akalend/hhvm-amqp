<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['port'=>  AMQP_PORT]);
	$ret = $cnn->connect();
	var_dump($ret);

	// $ch = new AMQPChannel(1);
	var_dump($cnn->isConnected());

	var_dump($cnn->disconnect(AMQP_NOPARAM));

	var_dump(AMQP::PORT);
	// $cnn2 = new AMQPConnection(['host'=>'sasa']);
	// $cnn2->connect();
	// var_dump($cnn2->isConnected());

	// var_dump($cnn->isConnected());
	// var_dump($cnn->getHost());