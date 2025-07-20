#include <iostream>
#include <unistd.h>
#include "hashtable.h"
#include "server.h"

int main(int argc, 	char* argv[]) {

	auto server = new Server<int, int>(10, "test", 30, 10);

	server->run();

	sleep(5);

	server->stop();

}

