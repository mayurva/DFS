//Common data structures required for master
#ifndef MASTER_H
#define MASTER_H
#include<sys/types.h>
#include<sys/stat.h>
#include<search.h>

#define CLIENT_LISTEN_PORT 5000
#define CHUNKSERVER_LISTEN_PORT 6000

#define NUM_CHUNKSERVERS 4
typedef struct chunkserver_{
	host h;
	int is_up;
}chunkserver;

typedef struct file_info_ {
	struct hsearch_data	*chunk_list;
	struct stat		filestat;
}file_info;

typedef struct chunk_info_ {
	char	chunk_handle[64];
	int	chunksever_id[2];
}chunk_info;
#endif
