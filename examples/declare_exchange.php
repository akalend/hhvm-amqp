<?php
// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	echo "------  connect ...  --------\n";

	$ret = $cnn->connect();

	echo "------  connect Ok  --------\n";

	$ch = new AMQPChannel($cnn);
	$ex = new AMQPExchange($ch);
	$q = new AMQPQueue($ch);

	echo "------  exchange Ok  --------\n";

// $cnn->disconnect(AMQP_NOPARAM);
	$ex->setName('test_e');
	$ex->setType(AMQP::TYPE_DIRECT);


	$ex->declare();
	echo "------  declare Ok  --------\n";

	$q->setName('test_q');
	$q->declare();


	$res = $ex->bind("test_q", "kkk" );
	echo "------  binding Ok  --------\n";
	var_dump($res);

	$cnn->disconnect();