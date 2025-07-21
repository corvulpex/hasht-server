#include <mutex>
#include <optional>

typedef enum {
	INSERT,
	REMOVE, 
	GET
} OP_TYPE;

typedef enum {
	EMPTY,
	INCOMING,
	FINISHED,
	ERROR
} OP_STATUS;

template <typename K, typename T>
struct Operation {
	OP_TYPE type;
	OP_STATUS status;
	K key;
	std::optional<T> value;
};

template <typename K, typename T>
struct SH_MEM {
	size_t ret_tail;
	size_t op_tail;
	size_t op_head;
	std::mutex tail_lock;
	std::mutex head_lock;
	Operation<K, T> op_ports[];
};

template <typename K, typename T>
size_t shared_mem_size(size_t port_size) {
	return sizeof(Operation<K, T>) * port_size + sizeof(size_t) * 3 + sizeof(std::mutex) * 2;
}

