#include <unistd.h>
#include "server.h"

int main(int argc, 	char* argv[]) {

	auto server = new Server<int, int>("/my_mem", 10, 30, 10);

	server->run();

	while (true) {}

	server->stop();

}

