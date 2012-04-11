//This file contains main code for the storage server
#include"gfs.h"
#include"chunkserver.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
#include<errno.h>

#define MAX_DATA_SZ 1000

char *chunk_path;
host chunkserver;
host master;
int heartbeat_port;
int client_listen_socket,master_socket;
pthread_mutex_t	seq_mutex;

int chunkserver_init(int argc, char *argv[])
{

	/* Set path of chunkserver */
	chunk_path = (char*)malloc(sizeof(char)*(strlen(argv[1])+1));	
	if(chunk_path == NULL){
		printf("%s: Failed to allocate memory\n",__func__);
		return -1;	
	}	
	strcpy(chunk_path,argv[1]);
	#ifdef DEBUG
		printf("path is %s\n", chunk_path);
	#endif
	
	/* Set ip addr and port of master */
	strcpy(master.ip_addr, argv[2]);
	master.port = MASTER_LISTEN;
	#ifdef DEBUG
		printf("Master ip address = %s port = %d\n", master.ip_addr, master.port);
	#endif

	/* Set heartbeat port of chunkserver */
	heartbeat_port = atoi(argv[3]);	
	#ifdef DEBUG
		printf("Chunkserver heartbeat port is %d\n", heartbeat_port);
	#endif

	/* Set ip addr of chunkserver */
	strcpy(chunkserver.ip_addr, argv[4]);
	#ifdef DEBUG
		printf("Chunkserver ip address = %s\n", chunkserver.ip_addr);
	#endif

	/* Set client port of chunkserver */
	chunkserver.port = atoi(argv[5]);	
	#ifdef DEBUG
		printf("Chunkserver client port is %d\n", chunkserver.port);
	#endif

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
        if((bindSocket(master_socket, heartbeat_port, chunkserver.ip_addr))==-1){
		printf("%s: Failed bind to the port\n",__func__);
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

	if(argc != 6){
		printf("%s: Incorrect command line arguments\n",__func__);
		printf("Usage: ./chunkserver <path> <master_ip> <heartbeat_port> <chunkserver_ip> <client_listen_port>\n");
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

        if((pthread_join(client_listen_thread,NULL))!=0){
                printf("%s: Failed to join listener thread for client\n",__func__);
        }
	if((pthread_join(master_listen_thread,NULL))!=0){
                printf("%s: Failed to join listener thread for master\n",__func__);
        }

        return 0;
}

void* handle_client_request(void *arg)
{	
        struct msghdr *msg;
        int soc = (int)arg;
        char * data = (char *) malloc(MAX_DATA_SZ);
        prepare_msg(0, &msg, data, MAX_DATA_SZ);

        recvmsg(soc, msg, 0);

        dfs_msg *dfsmsg;
        dfsmsg = (dfs_msg*)msg->msg_iov[0].iov_base;
#ifdef DEBUG
        printf("received message from client\n");
#endif
        //extract the message type
        print_msg(msg);

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
        struct msghdr *msg;
        char * data = (char *) malloc(MAX_DATA_SZ);
        prepare_msg(0, &msg, data, MAX_DATA_SZ);

	#ifdef DEBUG
		printf("this thread listens connection requests from clients\n");
	#endif

	while(1){
		soc = acceptConnection(client_listen_socket);
		if (soc == -1 ) {
			printf("Accept failed");
			exit(0);
		}
		#ifdef DEBUG
			printf("connected to client\n");
		#endif
		recvmsg(soc, msg, 0);
		#ifdef DEBUG
			printf("received message from client\n");
		#endif
		//extract the message type
		//and reply to client appropriately
	}
}

void* listenMaster(void* ptr)
{
	int soc;
	struct msghdr *msg;
	int index;
	int id = 0;
	char buf[200];
	#ifdef DEBUG
		printf("This thread receives periodic heartbeat messages from master\n");
	#endif
	while(1) {
		char buf1[10];
		prepare_msg(HEARTBEAT, &msg, &index, sizeof(index));
		int retval = recvmsg(master_socket, msg, 0);
		//int retval = recv(master_socket, buf1, 3, 0);
		if (retval == -1) {
			sprintf(buf, "Failed to receive heartbeat from master - %d\n", errno);
			write(1, buf, strlen(buf));
		} else {	
		#ifdef DEBUG
			sprintf(buf, "\nreceived heartbeat message-%d from master\n", ++id);
			write(1, buf, strlen(buf));
		#endif
		}

		retval = sendmsg(master_socket, msg, 0);
		if (retval == -1) {
			sprintf(buf, "Failed to send heartbeat ACK to master - %d\n", errno);
			write(1, buf, strlen(buf));
		} else {	
		#ifdef DEBUG
			sprintf(buf, "Sent heartbeat ACK-%d to master\n", id);
			write(1, buf, strlen(buf));
		#endif
		}
	}	
}
