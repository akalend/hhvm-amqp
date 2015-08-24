<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();
	var_dump($ret);

	$ch = new AMQPChannel($cnn);

	$cnn2 = new AMQPConnection( ['port'=>  AMQP_PORT]  );
	$ret = $cnn2->connect();
	var_dump($ret);

	var_dump($cnn2->isConnected());
	var_dump($cnn2->disconnect(AMQP_NOPARAM));
	var_dump($cnn2->isConnected());
echo '------------',"\n";
	var_dump($cnn->isConnected());
	var_dump($ch->isConnected());
	var_dump($cnn->isConnected());

	var_dump($cnn->disconnect(AMQP_NOPARAM));
// $cnn->disconnect(AMQP_NOPARAM);
