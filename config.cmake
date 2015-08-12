HHVM_EXTENSION(amqp amqp.cpp)
include_directories(include)

HHVM_SYSTEMLIB(amqp ext_amqp.php)