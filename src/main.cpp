#include <iostream>
#include "hashtable.h"

int main(int argc, 	char* argv[]) {

	auto ht = new Hashtable<int, int>(10);

	for (int i = 0; i < 100; i++) {
		ht->insert(i, i + 1000);
	}

	std::cout << "Insert end \n";

	for (int i = 99; i >= 0; i--) {
		std::optional<int> opt = ht->get(i);
		if (opt){
			std::cout << opt.value() << "\n";
		}
		else {
			std::cout << "No value at key " << i << "\n";
		}
	}


	for (int i = 0; i < 100; i += 2) {
		ht->remove(i);
	}


	std::cout << "Removed even numbers\n";

	for (int i = 99; i >= 0; i--) {
		std::optional<int> opt = ht->get(i);
		if (opt){
			std::cout << opt.value() << "\n";
		}
		else {
			std::cout << "No value at key " << i << "\n";
		}
	}
}

