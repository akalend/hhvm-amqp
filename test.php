<?php
	// print_r( get_loaded_extensions() );
	$cnn = new AMQPConnection(['host'=>'ak']);
	print_r($cnn->getHost());
	print_r($cnn->getPort());