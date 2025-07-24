# Hasht-Server

Server-side implementation for providing a hashtable to the client program via POSIX shared memory.
The client-side implementation can be found [here](https://github.com/corvulpex/hasht-client).

## Building:

You can build this program either by using CMake or build it directly by using the included Makefile.
Note that this program relies on POSIX functionality so it won't build on Windows. 

## How to use:

### Starting the server

Start the server by specifying the name of the shared memory object and the number of hashtable buckets.
```
./hasht-server [name_of_shared_memory] [number_of_buckets]
```

Optionally you can change the number of requests that can be queued at the same time. By default this number will be 30.
```
./hasht-server [name_of_shared_memory] [number_of_buckets] [number_of_queue_slots]
```
> [!WARNING]
> The number of queue slots has to be the same in the client program or the queue will break.

### Using the running server

After the server is running you can use the hashtable with the client side program.<br>
You can quit it by entering 'q'.


## Notes

This specific implementation lets you insert <int, int> pairs into the hashmap. The underlying logic is capable of using arbitrary types of a fixed size that have a std::hash implementation.
