#include "hashtable.h"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <vector>

typedef enum {
	INSERT,
	DELETE, 
	GET
} OP_Type;

typedef enum {
	EMPTY,
	INCOMING,
	FINISHED,
	ERROR
} OP_STATUS;

template <typename K, typename T>
struct Operation {
	std::mutex lock;
	OP_Type type;
	OP_STATUS status;
	K key;
	std::optional<T> value;
};

template <typename K, typename T>
struct shmem {
	Operation<K, T> op_ports[];
};

typedef struct {
	std::thread thread;
	bool terminate;
} thread_wrap;

template <typename K, typename T>
class Server {
	std::unique_ptr<Hashtable<K, T>> table;
	size_t port_count;
	size_t thread_count;
	std::vector<thread_wrap> workers;
	Operation<K, T> *ports;

public:
	Server(size_t bucket_num, const char* mem_name, size_t port_count, size_t thread_count) {
		this->table = std::make_unique<Hashtable<K, T>>(bucket_num);
		this->port_count = port_count;
		this->thread_count = thread_count;

		int shmem = shm_open(mem_name, O_CREAT | O_RDWR, 0666);
		ftruncate(shmem, sizeof(Operation<K, T>) * port_count);
		ports = (Operation<K, T> *) mmap(0, sizeof(Operation<K, T>) * port_count, PROT_WRITE | PROT_READ, MAP_SHARED, shmem, 0);
	}

	void handle_op(size_t i) {
		std::unique_lock<std::mutex> lock(ports[i].lock);
		if (ports[i].status != INCOMING)
			return;

		switch (ports[i].type) {
			case INSERT:
				table->insert(ports[i].key, ports[i].value.value());
				ports[i].status = EMPTY;
				return;

			case DELETE:
				table->remove(ports[i].key);
				ports[i].status = EMPTY;
				return;

			case GET:
				ports[i].value = table->get(ports[i].key);
				ports[i].status = FINISHED;
				return;

			default:
				std::cerr << "Got unkown operation at port " << i << "\n";
		}
	}

	void thread_runner(size_t id, size_t low_port, size_t high_port) {
		while (! workers[id].terminate) {

			size_t c = 0;
			for (size_t i = low_port; i < high_port; i++) {
				if (ports[i].status == INCOMING) {
					handle_op(i);	
					c++;
				}
			}
	
			if (!c) 
				std::this_thread::sleep_for(std::chrono::milliseconds(50));

		}	
	}

	void run() {
		workers = std::vector<thread_wrap>(thread_count);

		size_t port_per_thread = port_count / thread_count;
		size_t extra_ports = port_count % thread_count;
		size_t cur = 0;
		for (size_t i = 0; i < thread_count; i++) {
			size_t upper_port = cur += port_per_thread;
			if (extra_ports) {
				upper_port++;
				extra_ports--;
			}

			workers[i] = {std::thread( [this, i, cur, upper_port] { thread_runner(i, cur, upper_port); }), false};

			cur = upper_port;
		}
	}

	void stop() {
		for (size_t i = 0; i < thread_count; i++) {
			workers[i].terminate = true;
		}
		for (size_t i = 0; i < thread_count; i++) {
			workers[i].thread.join();
		}

	}

};

