#include <cstring>
#include <sstream>
#include <string>
#include <unistd.h>
#include "server.h"


void print_help() {
	std::cout << "You're on your own for now\n";
}


int main(int argc, 	char* argv[]) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_help();
			return 0;
		}
	}
	if (argc < 2) {
		std::cerr << "No name for shared memory specified\n";
		return 1;
	}
	if (argc < 3) {
		std::cerr << "No number of hash buckets specified\n";
		return 1;
	}
	long long int buckets;
	std::string bstr(argv[2]);
	std::stringstream sstream(bstr);
	sstream >> buckets;
	if (buckets <= 0) {
		std::cerr << "Number of buckets must be a valid positive number\n";
		return 1;
	}

	long long int ports = 31;
	if (argc >= 4) {
		std::string pstr(argv[3]);
		std::stringstream psstream(pstr);
		psstream >> ports;
		if (ports <= 0) {
			std::cerr << "Number of queue ports was not a valid positive number -> will be set to default (30)\n";
			ports = 31;
		}
		else {
			ports++;
		}
	}

	if (argc > 4)
		std::cout << "Extra arguments ignored\n";

	auto server = new Server<int, int>(argv[1], (size_t) buckets, (size_t) ports, 10);

	server->run();

	while (true) {
		std::string input_line;
		std::getline(std::cin , input_line);
		std::istringstream iss(input_line);
		if (input_line.length() == 1 && input_line[0] == 'q')
			break;
	}

	server->stop();

}

