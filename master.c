//This file contains main code for the master server.
#include<stdio.h>
#include<stdlib.h>
#include"gfs.h"
#define _GNU_SOURCE
#define __USE_GNU
#include"master.h"

struct hsearch_data *file_list;
host master;
int client_listen_socket;
int chunkserver_listen_socket;
chunkserver chunk_servers[NUM_CHUNKSERVERS];

int master_init()
{
	/* Create hashtable */
	file_list = (struct hsearch_data*)calloc(1, sizeof(struct hsearch_data)); 
	if (file_list == NULL) {
		printf("%s : Not enough memory\n", __func__);
		return -1;
	}
	int retval = hcreate_r(100, file_list);
	if (retval == 0) {
		printf("%s : Failed to create master hashtable\n", __func__);
		return -1;
	}

	/* Read config file */
	FILE * config_file;
        config_file = fopen(".master.config","r");
        if(config_file == 0)
        {
                printf("config file problem\n");
                return -1;
        }
	int i = 0;
	while (!feof(config_file)) {
		fscanf(config_file, "%s %d\n", chunk_servers[i].h.ip_addr, &chunk_servers[i].h.port);
		chunk_servers[i].is_up = 0;
		#ifdef DEBUG
			printf("chunk server %d: ip address is: %s and port is: %d\n",i,chunk_servers[i].h.ip_addr,chunk_servers[i].h.port);
		#endif
		i++;
	}

	/* Create master listen socket */
	populateIp(&master,"localhost");
	#ifdef DEBUG
		printf("ip address is %s\n",master.ip_addr);
	#endif
	client_listen_socket = createSocket();
	bindSocket(client_listen_socket, CLIENT_LISTEN_PORT, master.ip_addr);	
	//chunkserver_listen_socket = createSocket();
	//bindSocket(chunkserver_listen_socket, CHUNKSERVER_LISTEN_PORT, master.ip_addr);	
	
	return 0;
}

int main()
{
	int retval;

	retval = master_init();

		
	if (retval == -1) {
		printf("\n %s : Failed to initialize master", __func__);
		return -1;
	}
	return 0;
}
