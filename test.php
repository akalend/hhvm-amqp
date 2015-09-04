#!/usr/bin/hhvm
<?php
	// print_r( get_loaded_extensions() );

printf("--------- %d  ---------\n", __LINE__);

	$cnn = new AMQPConnection();

printf("--------- %d  ---------\n", __LINE__);

	$ret = $cnn->connect();
	echo "-------- connect Ok ------\n";


	$ch = new AMQPChannel($cnn);
	$queue = new AMQPQueue($ch);

// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("test_q");

	$queue->setFlags(AMQP::AUTOACK );//| AMQP::AUTOACK); AMQP::AUTODELETE
	$message = $queue->get( );

	var_dump($queue);

printf("--------- %d  ---------\n", __LINE__);

	// $ret = $queue->ack(1);
// echo "---------  30 ---------\n";

	var_dump($ret);
printf("--------- %d  ---------\n", __LINE__);
// var_dump($cnn);
	$cnn->disconnect();
