#include "shared_mem.h"
#include "hashtable.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <errno.h>

typedef struct {
	std::thread thread;
	bool terminate;
} thread_wrap;



template <typename K, typename T>
class Server {
	const char* mem_name;
	size_t port_count;
	size_t thread_count;
	std::unique_ptr<Hashtable<K, T>> table;
	std::vector<thread_wrap> workers;
	SH_MEM<K, T> *sh_mem;
	bool running = false;

	void handle_op(size_t i) {
		if (sh_mem->op_ports[i].status != INCOMING)
			return;

		switch (sh_mem->op_ports[i].type) {
			case INSERT:
				table->insert(sh_mem->op_ports[i].key, sh_mem->op_ports[i].value.value());
				sh_mem->op_ports[i].status = EMPTY;
				return;

			case REMOVE:
				table->remove(sh_mem->op_ports[i].key);
				sh_mem->op_ports[i].status = EMPTY;
				return;

			case GET:
				sh_mem->op_ports[i].value = table->get(sh_mem->op_ports[i].key);
				if (sh_mem->op_ports[i].value)
					std::cout << "Port " << i << ": " << sh_mem->op_ports[i].value.value() << "\n";
				sh_mem->op_ports[i].status = FINISHED;
				return;

			default:
				std::cerr << "Got unkown operation at port " << i << "\n";
		}
	}

	void thread_runner(size_t id) {
		while (! workers[id].terminate) {

			if (sh_mem->op_head != sh_mem->op_tail) {
				std::unique_lock<std::mutex> lock(sh_mem->tail_lock);
				if (sh_mem->op_head == sh_mem->op_tail)
					continue;


				size_t port_num = sh_mem->op_tail;
				sh_mem->op_tail = (sh_mem->op_tail + 1) % port_count;
				lock.unlock();
				handle_op(port_num);
			}			
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}	
	}

	void cleanup_thread() {
		while(! workers[thread_count].terminate) {
			if (sh_mem->ret_tail != sh_mem->op_tail) {

				if (sh_mem->op_ports[sh_mem->ret_tail].status == EMPTY)
					sh_mem->ret_tail = (sh_mem->ret_tail + 1) % port_count;

			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
	}


public:
	Server(const char* mem_name, size_t bucket_num, size_t port_count, size_t thread_count) {
		this->mem_name = mem_name;
		this->table = std::make_unique<Hashtable<K, T>>(bucket_num);
		this->port_count = port_count;
		this->thread_count = thread_count;


		int shmem = shm_open(mem_name, O_CREAT | O_RDWR, 0666);
		if (!shmem) {
			std::cerr << "Opening shared memory failed: " << strerror(errno) << "\n";
			throw std::runtime_error("");
		}


		if (ftruncate(shmem, shared_mem_size<K, T>(port_count)) == -1) {
			std::cerr << "Ftruncate failed: " << strerror(errno) << "\n";
			if (shm_unlink(mem_name) == -1) {
				std::cerr << "Could not unlink shared memory correctly: " << strerror(errno) << "\n";
			};
			throw std::runtime_error("");
		};


		sh_mem = (SH_MEM<K, T> *) mmap(0, shared_mem_size<K, T>(port_count), PROT_WRITE | PROT_READ, MAP_SHARED, shmem, 0);
		if (sh_mem == MAP_FAILED) {
			std::cerr << "Mmap failed: " << strerror(errno) << "\n";
			if (shm_unlink(mem_name) == -1) {
				std::cerr << "Could not unlink shared memory correctly: " << strerror(errno) << "\n";
			};
			throw std::runtime_error("");
		}	
	}

	~Server() {
		if (running)
			stop();
		
		if(shm_unlink(mem_name) == -1) {
			std::cerr << "Could not unlink shared memory correctly: " << strerror(errno) << "\n";
		}
		if (munmap(sh_mem, shared_mem_size<K, T>(port_count)) == -1) {
			std::cerr << "Could not unmap shared memory correctly: " << strerror(errno) << "\n";
		}

	}

	void run() {
		sh_mem->ret_tail = 0;
		sh_mem->op_tail = 0;
		sh_mem->op_head = 0;
		sh_mem->tail_lock.unlock();
		sh_mem->head_lock.unlock();
		std::fill_n(sh_mem->op_ports, port_count, Operation<K, T>{});

		workers = std::vector<thread_wrap>(thread_count + 1);

		for (size_t i = 0; i < thread_count; i++) {
			workers[i] = {std::thread( [this, i] { thread_runner(i); }), false};
		}
		workers[thread_count] = {std::thread( [this] { cleanup_thread(); }), false};
		running = true;
	}

	void stop() {
		for (size_t i = 0; i < thread_count + 1; i++) {
			workers[i].terminate = true;
		}
		for (size_t i = 0; i < thread_count + 1; i++) {
			workers[i].thread.join();
		}
		running = false;

	}

};

