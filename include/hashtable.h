#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <algorithm>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <vector>

template<typename K, typename T>
struct Hashentry {
	K key;
	T value;
};

template<typename K, typename T>
struct Hashbucket {
	std::shared_mutex rw_lock;
	std::list<Hashentry<K, T>> entries;

	void insert(K key, T value) {
		std::unique_lock<std::shared_mutex> lock(rw_lock);

		for (auto &entry: entries) {
			if (entry.key == key) {
				entry.value = value;
				return;
			}
		}

		entries.push_back({key, value});
	};

	bool remove(K key) {
		std::unique_lock<std::shared_mutex> lock(rw_lock);

		return entries.remove_if([key](Hashentry<K, T> e){ return e.key == key; });
	};

	std::optional<T> get(K key) {
		std::shared_lock<std::shared_mutex> lock(rw_lock);

		const auto v = std::find_if(entries.begin(), entries.end(), [key](Hashentry<K, T> e){ return e.key == key; });
		if (v != entries.end())
			return (*v).value;
		return {};
	}
};

template<typename K, typename T>
class Hashtable {
	std::vector<Hashbucket<K, T>> buckets;

public:

	Hashtable(size_t bucket_number) {
		buckets = std::vector<Hashbucket<K, T>>(bucket_number);
	};

	void insert(K key, T value) {
		buckets[std::hash<K>{}(key) % buckets.size()].insert(key, value);
	};


	bool remove(K key) {
		return buckets[std::hash<K>{}(key) % buckets.size()].remove(key);	
	};

	std::optional<T> get(K key) {
		return buckets[std::hash<K>{}(key) % buckets.size()].get(key);	
	};
};

#endif
