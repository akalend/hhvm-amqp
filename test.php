<?php
	// print_r( get_loaded_extensions() );

		echo "--------------\n";

	$cnn = new AMQPConnection();

		echo "--------------\n";

	$ret = $cnn->connect();
	echo "-------- connect Ok ------\n";


	$ch = new AMQPChannel($cnn);
	$queue = new AMQPQueue($ch);

// $cnn->disconnect(AMQP_NOPARAM);

	$queue->setName("mamamama");
	echo "--------------\n";

	$queue->setFlags(AMQP::AUTODELETE);

	// $queue->declare();
	

	// echo "------  delete  --------\n";	
	$res = $queue->setFlags(0);
	var_dump($res);

exit();
	echo "------  bind  --------\n";

	$queue->bind("exxx", "kkk");	
	echo "------  disconnect  --------\n";



	var_dump($cnn->disconnect(AMQP::NOPARAM));
