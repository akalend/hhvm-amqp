<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['host'=>'ak']);
	// $cnn->connect();
	var_dump($cnn->isConnected());

	$cnn2 = new AMQPConnection(['host'=>'sasa']);
	$cnn2->connect();
	var_dump($cnn2->isConnected());

	var_dump($cnn->isConnected());
	var_dump($cnn->getHost());