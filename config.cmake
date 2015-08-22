
include_directories(include)

HHVM_EXTENSION(amqp amqp.cpp)
HHVM_SYSTEMLIB(amqp ext_amqp_connect.php)
HHVM_SYSTEMLIB(amqp ext_amqp_channel.php)

link_directories(  /usr/local/lib/x86_64-linux-gnu/)

add_dependencies(amqp rabbitmq)

target_link_libraries(amqp -lrabbitmq)



