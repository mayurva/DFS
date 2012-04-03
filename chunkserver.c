//This file contains main code for the storage server
#include"gfs.h"
#include"chunkserver.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>

char *chunk_path;
host chunkserver;
host master;
int client_listen_socket,master_socket;
pthread_mutex_t	seq_mutex;

int chunkserver_init(int argc, char *argv[])
{
	chunk_path = (char*)malloc(sizeof(char)*(strlen(argv[1])+1));	
	if(chunk_path == NULL){
		printf("%s: Failed to allocate memory\n",__func__);
		return -1;	
	}	
	strcpy(chunk_path,argv[1]);	//THis is the path where chunks are stored
	#ifdef DEBUG
		printf("path is %s\n", chunk_path);
	#endif

	if((populateIp(&master,argv[2]))==-1){	//Ip address of master.. its listen port is known
		printf("%s: Failed to Connect to master\n",__func__);
		return -1;	
	}
	master.port = MASTER_LISTEN;

	if((populateIp(&chunkserver,"localhost"))==-1){
		printf("%s: Failed to Initiate chunk server\n",__func__);
		return -1;	
	}	

	chunkserver.port = atoi(argv[3]);	//This is the port where chunkserver will listen for clients
	if((client_listen_socket = createSocket())==-1){
		printf("%s: Failed to create socket\n",__func__);
		return -1;	
	}
        if((bindSocket(client_listen_socket, chunkserver.port, chunkserver.ip_addr))==-1){
		printf("%s: Failed bind to the port\n",__func__);
		return -1;	
	}
        if((listenSocket(client_listen_socket))==-1){
		printf("%s: Could not start the listener\n",__func__);
		return -1;	
	}

	if((master_socket = createSocket())==-1){
                printf("%s: Failed to create socket\n",__func__);
                return -1;
        }
	if(createConnection(master,master_socket)==-1){
		printf("%s: Could not connect to the master\n",__func__);
		return -1;	
	}

	pthread_mutex_init(&seq_mutex, NULL);
	//recv an ack
	//send IP and Port details
		
	return 0;
}

int main(int argc, char * argv[])
{
	pthread_t client_listen_thread, master_listen_thread, heartbeat_thread;

	if(argc != 4){
		printf("%s: Incorrect command line arguments\n",__func__);
		printf("Usage: ./chunkserver <path> <master_ip> <chunkserver_listen_port>\n");
		exit(-1);
	}

	if(chunkserver_init(argc,argv)!=0){
		printf("%s: Failed to initiate chunk Server\n",__func__);
		exit(-1);
	}
	#ifdef DEBUG
		printf("Chunkserver initialized\n");
	#endif

	if((pthread_create(&client_listen_thread,NULL,listenClient,NULL))!=0){
                printf("%s: Failed to create listener thread for client\n",__func__);
        }
        if((pthread_create(&master_listen_thread,NULL,listenMaster,NULL))!=0){
                printf("%s: Failed to create listener thread for master\n",__func__);
        }
        if((pthread_create(&heartbeat_thread,NULL,sendHeartbeat,NULL))!=0){
                printf("%s: Failed to create heart beat thread \n",__func__);
        }

        if((pthread_join(client_listen_thread,NULL))!=0){
                printf("%s: Failed to join listener thread for client\n",__func__);
        }
	if((pthread_join(master_listen_thread,NULL))!=0){
                printf("%s: Failed to join listener thread for master\n",__func__);
        }
	if((pthread_join(heartbeat_thread,NULL))!=0){
                printf("%s: Failed to join heartbeat thread\n",__func__);
        }

        return 0;
}

void* handle_client_request(void *arg)
{	
	struct msghdr msg;
        int soc = (int)arg;

        recvmsg(soc, &msg, 0);

        dfs_msg *dfsmsg;
        dfsmsg = (dfs_msg*)msg.msg_iov[0].iov_base;
#ifdef DEBUG
        printf("received message from client\n");
#endif
        //extract the message type
        print_msg(&msg);

        switch (dfsmsg->msg_type) {

                case HEARTBEAT:
                        break;

                case READ_DATA_REQ:
                        break;

                case WRITE_DATA_REQ:
                        break;
        }
}
	

void* listenClient(void* ptr)
{
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

void* listenMaster(void* ptr)
{
	#ifdef DEBUG
		printf("This server listens for control command from master\n");
	#endif
	while(1){
		//recv message from master
		//decide the action to be taken
	}	
}

void* sendHeartbeat(void* ptr)
{
	#ifdef DEBUG
		printf("This thread sends periodic heartbeat messages to master\n");
	#endif
	while(1){
		sleep(2);
		//send heartbeat message to the master
	}	
}
