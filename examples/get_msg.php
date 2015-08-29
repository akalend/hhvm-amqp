<?php
// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	echo "------  connect ...  --------\n";

	$ret = $cnn->connect();

	echo "------  connect Ok  --------\n";

var_dump($ret);

	$ch = new AMQPChannel($cnn);
	$q = new AMQPQueue($ch);

	echo "------  queue Ok  --------\n";

// $cnn->disconnect(AMQP_NOPARAM);
$q->setName('direct_messages');
// 	echo "------  declare  --------\n";

// $q->declare();

// 	echo "------  bind  --------\n";

// $q->bind('amq.direct', 'route_to_everybody');

	echo "------  get  --------\n";

$envelope = $q->get();

var_dump($envelope);


	$cnn->disconnect();
