<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();
	var_dump($ret);

	// $ch = new AMQPChannel(1);
	var_dump($cnn->isConnected());

	var_dump($cnn->disconnect());


	// $cnn2 = new AMQPConnection(['host'=>'sasa']);
	// $cnn2->connect();
	// var_dump($cnn2->isConnected());

	// var_dump($cnn->isConnected());
	// var_dump($cnn->getHost());