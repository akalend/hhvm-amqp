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

	// $queue->declare();
	
	echo "------- get  -------\n";

	// echo "------  delete  --------\n";	
	//$queue->setFlags(0);
	
	$ret = $queue->get();
	var_dump($ret);

	$ret = $queue->ack( $ret['delivery_tag'] );
	var_dump($ret);

exit();
	echo "------  bind  --------\n";

	$queue->bind("exxx", "kkk");	
	echo "------  disconnect  --------\n";



	var_dump($cnn->disconnect(AMQP::NOPARAM));
