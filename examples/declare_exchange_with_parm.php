<?php
	$cnn = new AMQPConnection();
	$ret = $cnn->connect();
	$ch = new AMQPChannel($cnn);

	$ex = new AMQPExchange($ch);
	$ex->setName('exchange-' . microtime(true));
	$ex->setType(AMQP::TYPE_FANOUT);
	$ex->setArguments(array("x-ha-policy" => "all"));
	$ex->setFlags(AMQP::PASSIVE | AMQP::DURABLE | AMQP::AUTODELETE | AMQP::INTERNAL);

	var_dump($ex);


	$cnn->disconnect();