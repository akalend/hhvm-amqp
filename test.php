<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['host'=>'ak']);
	// $cnn->connect();
	var_dump($cnn->isConnected());