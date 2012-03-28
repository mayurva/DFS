//This file contains main code for the master server.
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
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
	char *hostname;
	int len;
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
                printf("%s: config file problem\n",__func__);
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
	if(gethostname(&hostname,len)!=-1){
		printf("length is %d\n",len);
//		printf("hostname %s\n",hostname);
		populateIp(&master,hostname);
	}
	else{
		populateIp(&master,"192.168.2.4");
	}

	#ifdef DEBUG
		printf("ip address is %s\n",master.ip_addr);
	#endif
	client_listen_socket = createSocket();
	bindSocket(client_listen_socket, CLIENT_LISTEN_PORT, master.ip_addr);	
	listenSocket(client_listen_socket);
	chunkserver_listen_socket = createSocket();
	bindSocket(chunkserver_listen_socket, CHUNKSERVER_LISTEN_PORT, master.ip_addr);	
	listenSocket(chunkserver_listen_socket);
	return 0;
}
int find_chunkserver(host h)
{
	return 0;
}

int main()
{
	int retval;
	pthread_t client_listen_thread, chunkserver_listen_thread;

	retval = master_init();
	if (retval == -1) {
		printf("%s : Failed to initialize master\n", __func__);
		return -1;
	}

	if((pthread_create(&client_listen_thread,NULL,listenClient,NULL))!=0){
		printf("%s: Failed to create listener thread for client\n",__func__);
	}
	if((pthread_create(&chunkserver_listen_thread,NULL,connectChunkServer,NULL))!=0){
		printf("%s: Failed to create listener thread for chunk server\n",__func__);
	}

	if((pthread_join(chunkserver_listen_thread,NULL))!=0){
		printf("%s: Failed to join listener thread for chunkserver\n",__func__);
	}
	if((pthread_join(client_listen_thread,NULL))!=0){
		printf("%s: Failed to join listener thread for client\n",__func__);
	}

	return 0;
}

//this threads waits for chunk servers to connect and forks a new thread if a chunkserver joins
void* connectChunkServer(void* ptr)
{
	int i,j;
	host h;
	int soc;
	#ifdef DEBUG
		printf("Waiting for chunk servers to join\n");
	#endif
	for(i=0;i<NUM_CHUNKSERVERS;i++){
		soc = acceptConnection(chunkserver_listen_socket);
		//send a reply ack msg
		//chunkserver sends back his ip and port. save it in h
		//find if this chunkserver present in configured list. return the index if so.
		if((j=find_chunkserver(h))!=-1){	
			chunk_servers[j].conn_socket = soc;
			chunk_servers[j].is_up = 1;
			#ifdef DEBUG
				printf("Chunkserver %d joined\n",j);
			#endif
			if((pthread_create(&chunk_servers[j].thread,NULL,listenChunkServer,(void*)&j))!=0){
        	        	printf("%s: Failed to create corresponding thread for chunk server %d\n",__func__,j);
        		}
		}
	}

	#ifdef DEBUG
		printf("All chunkservers joined\n");
	#endif
	for(i=0;i<NUM_CHUNKSERVERS;i++){
		if((pthread_join(chunk_servers[i].thread,NULL))!=0){
                	printf("%s: Failed to join thread for chunkserver %d\n",__func__,i);
        	}
	}
}

void* listenClient(void* ptr){
	int soc;
	struct msghdr msg;

	#ifdef DEBUG
		printf("this thread listens connection requests from clients\n");
	#endif
	
	while(1){
		soc = acceptConnection(client_listen_socket);
		#ifdef DEBUG
			printf("connected to client\n");
		#endif
		recvmsg(soc, &msg, 0);
		#ifdef DEBUG
			printf("received message from client\n");
		#endif
		//extract the message type
		//and reply to client appropriately
	}
}

void* listenChunkServer(void* ptr)
{
	int index = *(int*)ptr;
	struct msghdr msg;
	#ifdef DEBUG
		printf("this thread listens heartbeat messages from chunkserver %d\n",index);
	#endif
	while(1){
		recvmsg(chunk_servers[index].conn_socket, &msg, 0);
		#ifdef DEBUG
			printf("Received heartbeat message from chunkserver %d\n",index);
		#endif
		//reply to heartbeat
	}
}
