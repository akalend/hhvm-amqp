<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['host'=>'localhost']);
	$ret = $cnn->connect();
	var_dump($ret);
var_dump($cnn->disconnect());

	// var_dump($cnn->isConnected());

	// $cnn2 = new AMQPConnection(['host'=>'sasa']);
	// $cnn2->connect();
	// var_dump($cnn2->isConnected());

	// var_dump($cnn->isConnected());
	// var_dump($cnn->getHost());