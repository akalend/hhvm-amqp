# HHVM AMQP bindings 
### Build Status (in the devepolment)

Object-oriented PHP bindings for the AMQP C library (https://github.com/alanxz/rabbitmq-c)

# Requirements:

    RabbitMQ C library, commonly known as librabbitmq (librabbitmq >= 0.6.0 required).

# Installation

	# Download the rabbitmq-c library @ version 0-9-1
	  git clone git://github.com/alanxz/rabbitmq-c.git
	  cd rabbitmq-c
	  # Enable and update the codegen git submodule
	  git submodule init
	  git submodule update
	  # Configure, compile and install
	  autoreconf -i && ./configure && make && sudo make install

	  