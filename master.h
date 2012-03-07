//Common data structures required for master
#include <stat.h>

#define NUM_CHUNKSERVERS 4

struct chunk_server {
	char	ip[20];
	int	port;
};	

struct file_info {
	struct hsearch_data	chunk_list;
	struct stat		filestat;
};

struct	chunk_info {
	char	chunk_handle[64];
	int	chunksever_id[2];
};
	
