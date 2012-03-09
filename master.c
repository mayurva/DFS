//This file contains main code for the master server.
#include<stdio.h>
#include"gfs.h"
#include"master.h"

struct hsearch_data *file_list;
int master_listen_socket;
chunk_server	chunk_servers[NUM_CHUNKSERVERS];


int master_init()
{
	/* Create hashtable */
	file_list = (struct hsearch_data*) calloc(1, sizeof(struct hsearch_data));
	if (file_list == NULL) {
		printf("\n %s : Not enough memory", __func__);
		return -1;
	}
	int retval = hcreate_r(100, file_list);
	if (retval == 0) {
		printf("\n %s : Failed to create master hashtable", __func__);
		return -1;
	}

	/* Read config file */
	FILE * config_file;
        config_file = fopen(".master.config","r");
        if(config_file == 0)
        {
                printf("\nconfig file problem\n");
                return -1;
        }
	int i = 0;
	while (!feof(config_file)) {
		fscanf(config_file, "%s %d\n", chunk_servers[i].ip, chunk_servers[i].port);
		i++;
	}

	/* Create master listen socket */
	int optval = 1;
	master_listen_socket = createSocket();
	setsockopt(master_listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	bindSocket(master_listen_socket, LISTEN_PORT, MY_IP_ADDRESS);	
	return 0;
}

int main()
{
	int retval;

	retval = master_init();

		
	if (retval == 0) {
		printf("\n %s : Failed to create master hashtable", __func__);
		return -1;
	}
	return 0;
}
