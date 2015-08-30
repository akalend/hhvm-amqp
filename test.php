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

	$queue->setName("direct_messages");

	$queue->setFlags(AMQP::AUTODELETE);

	$queue->declare();
	
	echo "------- get  -------\n";

	// echo "------  delete  --------\n";	
	$res = $queue->setFlags(0);
	
	var_dump($queue->get());

exit();
	echo "------  bind  --------\n";

	$queue->bind("exxx", "kkk");	
	echo "------  disconnect  --------\n";



	var_dump($cnn->disconnect(AMQP::NOPARAM));
