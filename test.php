#!/usr/bin/hhvm
<?php
	// print_r( get_loaded_extensions() );

printf("--------- %d  ---------\n", __LINE__);

	$cnn = new AMQPConnection(['port'=>5672]);

printf("--------- %d  ---------\n", __LINE__);

	$ret = $cnn->connect();
	echo "-------- connect Ok ------\n";


	$ch = new AMQPChannel($cnn);
	$queue = new AMQPQueue($ch);

// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("test_q");

	$queue->setFlags(AMQP::AUTOACK);//| ); AMQP::AUTODELETE
	$message = $queue->get( );

printf("--------- %d  ---------\n", __LINE__);
	var_dump($message);
// printf("--------- %d  ---------\n", __LINE__);

	if ($message)
	 	var_dump($message->getHeader('sss'));

printf("--------- %d  ---------\n", __LINE__);
	var_dump($queue->getArguments());
	// var_dump($queue);

// 	$msg = $queue->getMessage();
// 	var_dump($message->getContentType());

// printf("--------- %d  ---------\n", __LINE__);

	// $ret = $queue->ack();
// 	var_dump($ret);
// printf("--------- %d  ---------\n", __LINE__);
// var_dump($cnn);
	$cnn->disconnect();
