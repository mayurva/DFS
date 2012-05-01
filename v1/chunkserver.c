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

char *chunk_path;
host chunkserver;
host master;
int heartbeat_port;
int client_listen_socket,master_socket;
pthread_mutex_t	seq_mutex;
#define MAX_THR 100
pthread_t       threads[MAX_THR];
int thr_id = 0;
#define DEBUG
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
//	strcpy(chunkserver.ip_addr, argv[4]);
	getSelfIp(&chunkserver);
	#ifdef DEBUG
		printf("Chunkserver ip address = %s\n", chunkserver.ip_addr);
	#endif

	/* Set client port of chunkserver */
	chunkserver.port = atoi(argv[4]);	
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

	if(argc != 5){
		printf("%s: Incorrect command line arguments\n",__func__);
		printf("Usage: ./chunkserver <path> <master_ip> <heartbeat_port> <client_listen_port>\n");
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

int chunk_read(read_data_req * req, read_data_resp *resp)
{
	char	path[256];
	
	strcpy(path, chunk_path);
	strcat(path, req->chunk_handle);

	#ifdef DEBUG
	printf("opening chunk for read- %s req_size = %d req_offset = %d\n", path, req->size, req->offset);
	#endif
	FILE * chunk_fd = fopen(path, "r");
	if (chunk_fd == NULL) {
	#ifdef DEBUG
		printf("cannot open chunk file for reading - %s\n", path);
	#endif
		return -1;
	} else {
	#ifdef DEBUG
		printf("opened chunk file for read - %s\n", path);
	#endif
	}

	fseek(chunk_fd, req->offset, SEEK_SET);
	size_t retval = fread(resp->chunk, 1, req->size, chunk_fd);
	if (retval == 0) {
	#ifdef DEBUG
		printf("failure: chunkfile not read\n");
	#endif
                return -1;
	} else {
	#ifdef DEBUG
		printf("Read chunkfile- %s\n", path);
		int i;
		for (i = 0; i < req->size; i++)
			printf("%c", resp->chunk[i]);
		printf("\n");
	#endif
		resp->size = retval;
	}
	return 0; 
}


int chunk_write(write_data_req *req)
{
	char	path[256];
	
	#ifdef DEBUG
	printf("opening chunk for write- %s req_size = %d req_offset = %d\n", &(req->chunk[CHUNK_SIZE]), req->size, req->offset);
	#endif
	strcpy(path, chunk_path);
	strcat(path, &(req->chunk[CHUNK_SIZE]));
	#ifdef DEBUG
	printf("creating chunk for write - %s\n", path);
	#endif

	FILE * chunk_fd = fopen(path, "a");
	if (chunk_fd == NULL) {
	#ifdef DEBUG
		printf("cannot open chunk file for writing - %s\n", path);
	#endif
		return -1;
	} else {
	#ifdef DEBUG
		printf("opened chunk file for write- %s\n", path);
	#endif
	}
	
	#ifdef DEBUG
	int i;
	for (i = 0; i < req->size; i++)
		printf("%c", req->chunk[i]);
	printf("\n");
	#endif

	//fseek(chunk_fd, req->offset, SEEK_SET);
	size_t retval = fwrite(req->chunk, req->size, 1, chunk_fd);
	fclose(chunk_fd);
	if (retval != 1) {
	#ifdef DEBUG
		printf("failure: write incomplete\n");
	#endif
                return -1;
	} else {
	#ifdef DEBUG
		printf("Written chunkfile successfully- %s\n", path);
	#endif
	}
	return 0; 
}

void* handle_client_request(void *arg)
{	
        struct msghdr *msg;
        int soc = (int)arg;
        char * data = (char *) malloc(MAX_BUF_SZ);
        prepare_msg(0, &msg, data, MAX_BUF_SZ);
	read_data_resp * resp;
	dfs_msg *dfsmsg;

        int retval = recvmsg(soc, msg, 0);
	if (retval == -1) {
		printf("failed to receive message from client - errno-%d\n", errno);
	} else {
		dfsmsg = (dfs_msg*)msg->msg_iov[0].iov_base;
		#ifdef DEBUG
        	printf("received message from client - %d\n", dfsmsg->msg_type);
		#endif
	}


        //extract the message type
        print_msg(dfsmsg);

        switch (dfsmsg->msg_type) {

                case HEARTBEAT:
                        break;

                case READ_DATA_REQ:
			resp = (read_data_resp*) malloc(sizeof(read_data_resp));
			dfsmsg->status = chunk_read(msg->msg_iov[1].iov_base, resp);
			msg->msg_iov[1].iov_base = resp;
			msg->msg_iov[1].iov_len = sizeof(read_data_resp);
			dfsmsg->msg_type = READ_DATA_RESP;
			retval = sendmsg(soc, msg, 0); 
			if (retval == -1) {
				printf("failed to send read reply to client - errno-%d\n", errno);
			} else {
				#ifdef DEBUG
				printf("sent read reply to client\n");
				#endif
			}
			break;

                case WRITE_DATA_REQ:
			dfsmsg->status = chunk_write(msg->msg_iov[1].iov_base);
			dfsmsg->msg_type = WRITE_DATA_RESP; 
			retval = sendmsg(soc, msg, 0); 
			if (retval == -1) {
				printf("failed to send write reply to client - errno-%d\n", errno);
			} else {
				#ifdef DEBUG
				printf("sent write reply to client\n");
				#endif
			}
			break;
	}
}
	

void* listenClient(void* ptr)
{
	int soc;
        struct msghdr *msg;
        char * data = (char *) malloc(MAX_BUF_SZ);
        prepare_msg(0, &msg, data, MAX_BUF_SZ);

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
                if (thr_id == MAX_THR) {
                        thr_id = 0;
                }
                if((pthread_create(&threads[thr_id++], NULL, handle_client_request, (void*)soc)) != 0) {
                        printf("%s: Failed to create thread to handle client requests %d\n", __func__, thr_id);
                }
	}
}

void* listenMaster(void* ptr)
{
        int soc;
//      struct msghdr *msg;
        int index;
        int id = 0;
        char buf[200];
        //char msg[MAX_BUF_SZ];
        #ifdef DEBUG
                printf("This thread receives periodic heartbeat messages from master\n");
        #endif
        while(1) {
                char buf1[10];
                //prepare_msg(HEARTBEAT, &msg, &index, sizeof(index));
                int retval = recv(master_socket, &id, sizeof(int), 0);
                //int retval = recv(master_socket, buf1, 3, 0);
                if (retval == -1) {
                        sprintf(buf, "Failed to receive heartbeat from master - %d\n", errno);
                        write(1, buf, strlen(buf));
                } else {
                #ifdef DEBUG1
                        sprintf(buf, "\nreceived heartbeat message-%d from master\n", ++id);
                        write(1, buf, strlen(buf));
                #endif
                }

                retval = send(master_socket,&id,sizeof(int),0);
//              retval = sendmsg(master_socket, msg, 0);
                if (retval == -1) {
                        sprintf(buf, "Failed to send heartbeat ACK to master - %d\n", errno);
                        write(1, buf, strlen(buf));
                } else {
                #ifdef DEBUG1
                        sprintf(buf, "Sent heartbeat ACK-%d to master\n", id);
                        write(1, buf, strlen(buf));
                #endif
                }
        }
}

