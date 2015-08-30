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

	$queue->setName("q_zzz");

	$queue->setFlags(AMQP::AUTODELETE);

	// $queue->declare();
	
	echo "------- get  -------\n";

	// echo "------  delete  --------\n";	
	//$queue->setFlags(0);
	
	$ret = $queue->get();
	var_dump($ret);

exit();
	echo "------  bind  --------\n";

	$queue->bind("exxx", "kkk");	
	echo "------  disconnect  --------\n";



	var_dump($cnn->disconnect(AMQP::NOPARAM));
