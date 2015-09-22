#!/usr/bin/hhvm
<?php
$rabbit = new AMQPConnection(
	// array('host' => '127.0.0.1', 'port' => '5672', 'login' => 'guest', 'password' => 'guest')
	);
printf("--------- %d  ---------\n", __LINE__);
$res = $rabbit->connect();
printf("--------- %d  ---------\n", __LINE__);

var_dump($res);
if (!$res) {
	die("connection false\n");
}
printf("--------- %d  ---------\n", __LINE__);


$testChannel = new AMQPChannel($rabbit);
printf("--------- %d  ---------\n", __LINE__);
$testExchange = new AMQPExchange($testChannel);

$testExchange->setName('test_e');
// $testExchange->delete();

$testExchange->setType('direct');
printf("--------- %d  ---------\n", __LINE__);
$testExchange->declare();
printf("--------- %d  ---------\n", __LINE__);
unset($testChannel);
printf("--------- %d  ---------\n", __LINE__);
$testChannel = new AMQPChannel($rabbit);
printf("--------- %d  ---------\n", __LINE__);
$q = new AMQPQueue($testChannel);
$q->setName('test_q');
$q->declare();
$q->bind('test_e', 'scan');
printf("--------- %d  ---------\n", __LINE__);
$rabbit->disconnect();
