#!/usr/bin/hhvm
<?php

	$cnn = new AMQPContext();
	
	var_dump($cnn);

	
	printf("--------- %d  ---------\n", __LINE__);
	$ret = $cnn->connect();
	echo "-------- connect Ok ------\n";
	var_dump($cnn->isConnected());

