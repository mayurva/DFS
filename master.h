//Common data structures required for master
#ifndef MASTER_H
#define MASTER_H
#include<sys/types.h>
#include<sys/stat.h>
#include<search.h>

#define CLIENT_LISTEN_PORT 5000
#define CHUNKSERVER_LISTEN_PORT 6000
#define MAX_THR 100
#define NUM_CHUNKSERVERS 2
typedef struct chunkserver_{
	char		ip_addr[20];
	int		heartbeat_port;
	int		client_port;
	int		is_up;
	pthread_t	thread;
	int		conn_socket;
}chunkserver;

typedef struct file_info_ {
	struct hsearch_data *chunk_list;
	int num_of_chunks;
	struct stat filestat;
}file_info;

typedef struct chunk_info_ {
	char chunk_handle[64];
	int chunkserver_id[2];
	int last_read;
}chunk_info;

void* connect_chunkserver_thread(void*);
void* client_request_listener(void*);
void* heartbeat_thread(void*);

#endif
