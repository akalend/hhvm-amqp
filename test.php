<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['host'=>'highload.ru']);
	$ret = $cnn->connect();
	var_dump($ret);
<<<<<<< HEAD

	// $ch = new AMQPChannel(1);

	var_dump($cnn->disconnect());

=======
>>>>>>> parent of e1b043b... fix disconnect method
	// var_dump($cnn->isConnected());

	// $cnn2 = new AMQPConnection(['host'=>'sasa']);
	// $cnn2->connect();
	// var_dump($cnn2->isConnected());

	// var_dump($cnn->isConnected());
	// var_dump($cnn->getHost());