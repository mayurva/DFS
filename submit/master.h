//Common data structures required for master
#ifndef MASTER_H
#define MASTER_H
#include<sys/types.h>
#include<sys/stat.h>
#include<search.h>
#include<pthread.h>

#define CLIENT_LISTEN_PORT 5000
#define CHUNKSERVER_LISTEN_PORT 6000
#define MAX_THR 100
#define NUM_CHUNKSERVERS 4

struct chunk_info_;

typedef struct chunklist_node_ {
	int			other_cs;
	int			moved_cs;
	struct chunk_info_ 	*chunk_ptr;
	struct chunklist_node_	*next;
}chunklist_node;

typedef struct chunkserver_ {
	char		ip_addr[20];
	int		heartbeat_port;
	int		client_port;
	int		is_up;
	pthread_t	thread;
	int		conn_socket;
	chunklist_node	*head,*tail;
}chunkserver;

typedef struct file_info_ {
	char file_name[MAX_FILENAME];
	struct	hsearch_data *chunk_list;
	int	num_of_chunks;
	int 	write_in_progress;
	struct	stat filestat;
	struct file_info_ *next;
	int 	is_deleted;
}file_info;

typedef struct directory_{
	file_info *head,*tail;
}directory;

typedef struct chunk_info_ {
	char	chunk_handle[64];
	int	chunkserver_id[2];
	int	last_read;
	int     chunk_size;
}chunk_info;

extern int failover_array[6][3];
extern chunkserver chunk_servers[NUM_CHUNKSERVERS];
extern pthread_mutex_t msg_mutex;

void* connect_chunkserver_thread(void*);
void* client_request_listener(void*);
void* heartbeat_thread(void*);

#endif
