#!/usr/bin/hhvm
<?php
	// print_r( get_loaded_extensions() );

		echo "--------------\n";

	$cnn = new AMQPConnection(['port'=>5672]);

		echo "--------------\n";

	$ret = $cnn->connect();
	echo "-------- connect Ok ------\n";


	$ch = new AMQPChannel($cnn);
	$queue = new AMQPQueue($ch);

// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("test_q");

	$queue->setFlags(AMQP::AUTODELETE );//| AMQP::AUTOACK);
	$message = $queue->get();

	var_dump($queue->getMessage());
	$ret = $queue->ack();
	var_dump($ret);

	var_dump($cnn->disconnect(AMQP::NOPARAM));
