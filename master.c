//This file contains main code for the master server.
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include"gfs.h"
#define _GNU_SOURCE
#define __USE_GNU
#include"master.h"
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEBUG

#define  MAX_DATA_SZ 1000

struct hsearch_data *file_list;
host master;

int file_inode =0;
unsigned chunk_id = 0;
int secondary_count = 1;

int client_request_socket;
int heartbeat_socket;

chunkserver chunk_servers[NUM_CHUNKSERVERS];
pthread_mutex_t seq_mutex;
pthread_t	threads[MAX_THR];
int thr_id = 0;

char* itoa(int n)
{
}

int master_init(char *master_ip)
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

	/* Read config file and prepare chunkserver table */
	FILE * config_file;
        config_file = fopen(".master.config","r");
        if(config_file == 0) {
                printf("%s: config file problem\n",__func__);
                return -1;
        }
	int i = 0;
	while (!feof(config_file)) {
		fscanf(config_file, "%s %d %d\n", chunk_servers[i].ip_addr, &chunk_servers[i].heartbeat_port, &chunk_servers[i].client_port);
		chunk_servers[i].is_up = 0;
		#ifdef DEBUG
			printf("chunk server %d: ip address is: %s and heartbeat port is: %d and client port is %d\n"
					,i, chunk_servers[i].ip_addr, chunk_servers[i].heartbeat_port, chunk_servers[i].client_port);
		#endif
		i++;
	}


	/* Set master ip address */
	strcpy(master.ip_addr, master_ip);	
	#ifdef DEBUG
		printf("ip address is %s\n",master.ip_addr);
	#endif

	/* Initialize sequence number mutex */
	pthread_mutex_init(&seq_mutex, NULL);

	/* Create, bind and listen socket for client requests */
	client_request_socket = createSocket();
	bindSocket(client_request_socket, CLIENT_LISTEN_PORT, master.ip_addr);	
	listenSocket(client_request_socket);

	/* Create, bind and listen socket for heartbeats */
	heartbeat_socket = createSocket();
	bindSocket(heartbeat_socket, CHUNKSERVER_LISTEN_PORT, master.ip_addr);	
	listenSocket(heartbeat_socket);
	return 0;
}

/* Find the chunkserver index */
int find_chunkserver(host h)
{
	int i;
	for (i = 0; i < NUM_CHUNKSERVERS; i++) {
		if ((strcmp(h.ip_addr, chunk_servers[i].ip_addr) == 0) && (h.port == chunk_servers[i].heartbeat_port)) {
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[])
{
	int retval;
	pthread_t client_listen_thread, chunkserver_listen_thread;

	if (argc != 2) {
		printf("run as : ./master <master-ip-address>\n");
		return 0;
	}

	/* Initialize master */
	retval = master_init(argv[1]);
	if (retval == -1) {
		printf("%s : Failed to initialize master\n", __func__);
		return -1;
	}

	if((pthread_create(&client_listen_thread, NULL, client_request_listener, NULL)) != 0) {
		printf("%s: Failed to create listener thread for client\n",__func__);
	}
	if((pthread_create(&chunkserver_listen_thread, NULL, connect_chunkserver_thread, NULL)) != 0) {
		printf("%s: Failed to create listener thread for chunk server\n",__func__);
	}

	if((pthread_join(chunkserver_listen_thread, NULL)) != 0) {
		printf("%s: Failed to join listener thread for chunkserver\n",__func__);
	}
	if((pthread_join(client_listen_thread, NULL)) != 0) {
		printf("%s: Failed to join listener thread for client\n",__func__);
	}

	return 0;
}

/* this threads waits for chunk servers to connect and forks a new thread if a chunkserver joins */
void* connect_chunkserver_thread(void* ptr)
{
	int i,j;
	host h;
	int soc;
	char buf[200];
	#ifdef DEBUG
		printf("Waiting for chunk servers to join\n");
	#endif
	for(i = 0; i < NUM_CHUNKSERVERS; i++) {
		/* Accept connection from chunkserver */
		soc = acceptConnection(heartbeat_socket);

		/* Get the connection details of the chunkserver */
		struct sockaddr_in sockaddr;
		socklen_t addrlen = sizeof(struct sockaddr_in);
		if (getpeername(soc, (struct sockaddr*) &sockaddr, &addrlen) == 0) {
			strcpy(h.ip_addr, inet_ntoa(sockaddr.sin_addr));
			h.port = (unsigned)ntohs(sockaddr.sin_port);
			sprintf(buf, "chunk server ip = %s port = %d\n", h.ip_addr, h.port);
			write(1, buf, strlen(buf));
		} else {
			sprintf(buf, "Unable to obtain chunkserver ip and port\n");
			write(1, buf, strlen(buf));
			continue;
		}
 
		/* find if this chunkserver present in configured list. return the index if so */
		if((j = find_chunkserver(h)) != -1  ) {	
			chunk_servers[j].conn_socket = soc;
			chunk_servers[j].is_up = 1;
			#ifdef DEBUG
				printf("Chunkserver %d joined %d\n",j, soc);
			#endif
			/* Start a heartbeat thread for this chunkserver */
			if((pthread_create(&chunk_servers[j].thread, NULL, heartbeat_thread, (void*)j)) != 0) {
        	        	printf("%s: Failed to create corresponding thread for chunk server %d\n",__func__,j);
        		}
		}
	}

	for(i = 0; i < NUM_CHUNKSERVERS; i++) {
		if((pthread_join(chunk_servers[i].thread, NULL)) != 0) {
                	printf("%s: Failed to join thread for chunkserver %d\n",__func__,i);
        	}
	}

	#ifdef DEBUG
		printf("All chunkservers joined\n");
	#endif
}

void* handle_client_request(void *arg)
{
	struct msghdr *msg;
	int soc  = (int)arg;
	open_req *open_req_obj;
	write_req *write_req_obj;
	ENTRY e,*ep,*ep_temp;
	struct timeval tv;

	char * data = (char *) malloc(MAX_DATA_SZ);
	prepare_msg(0, &msg, data, MAX_DATA_SZ);
	recvmsg(soc, msg, 0);

	dfs_msg *dfsmsg;
	dfsmsg = (dfs_msg*)msg->msg_iov[0].iov_base;	
	#ifdef DEBUG
	printf("received message from client\n");
	#endif
	//extract the message type
	//print_msg(dfsmsg);
	
	switch (dfsmsg->msg_type) {

		case CREATE_REQ:
			#ifdef DEBUG
			printf("received create request from client\n");
			#endif
			break;

		case OPEN_REQ:
			#ifdef DEBUG
				printf("received open request from client\n");
			#endif
			open_req_obj = dfsmsg->data;
			e.key = open_req_obj->path;
			if(hsearch_r(e,FIND,&ep,file_list) ==0){
				if(open_req_obj->flags|O_CREAT){
					file_info *new_file = (file_info*)malloc(sizeof(file_info));
					hcreate_r(10,new_file->chunk_list);

					new_file->filestat.st_dev = 0;
					pthread_mutex_lock(&seq_mutex);
						new_file->filestat.st_ino = file_inode++;
					pthread_mutex_unlock(&seq_mutex);
					new_file->num_of_chunks = 0;

					new_file->filestat.st_mode = 00777;
					new_file->filestat.st_nlink = 0;
					new_file->filestat.st_uid = 0;
					new_file->filestat.st_gid = 0;
					new_file->filestat.st_rdev = 0;
					new_file->filestat.st_size = 0;
					new_file->filestat.st_blksize = CHUNK_SIZE;
					new_file->filestat.st_blocks = new_file->filestat.st_size/512;
					gettimeofday(&tv,NULL);
					new_file->filestat.st_atime = new_file->filestat.st_mtime = new_file->filestat.st_ctime = tv.tv_sec;
					e.data = new_file;
					if(hsearch_r(e,ENTER,&ep,file_list) == 0){
						printf("Error creating file %s\n",open_req_obj->path);
						//ret_val = -1;
					}
					else{			
						//TODO:Reply success to the client
					}
				}
				else{
					#ifdef DEBUG	
						printf("File not present\n");
					#endif
					//TODO:Reply error to the client
				}
			}
			else{
				if(open_req_obj->flags|O_CREAT){
					printf("File already present\n");
				}
				else{
					//TODO:IF it is open request
				}
			}
			break;

		case GETATTR_REQ:
			#ifdef DEBUG
			printf("received getattr request from client\n");
			#endif
			dfsmsg->status = -1;
			sendmsg(soc, msg, 0);
			break;

		case READDIR_REQ:
			#ifdef DEBUG
			printf("received readdir request from client\n");
			#endif
			break;

		case READ_REQ:
			#ifdef DEBUG
			printf("received read request from client\n");
			#endif
			break;

		case WRITE_REQ:
			#ifdef DEBUG
				printf("received write request from client\n");
			#endif
			write_req_obj = dfsmsg->data;
			e.key = write_req_obj->filename;
			if(hsearch_r(e,FIND,&ep,file_list)==0){
				printf("File not present\n");
				//TODO:send error message
			}
			else{
				chunk_info *c=(chunk_info*)malloc(sizeof(chunk_info));
				if(((file_info*)ep->data)->num_of_chunks>=write_req_obj->chunk_index){
					printf("Error not an append operation\n");
					//TODO: Send error message
				}
				
				strcpy(c->chunk_handle,itoa(chunk_id));
				//TODO: what to do if a chunkserver is down
				e.key = itoa(((file_info*)ep->data)->num_of_chunks);
				c->chunkserver_id[0] = chunk_id%NUM_CHUNKSERVERS;
				c->chunkserver_id[1] = (c->chunkserver_id[0] + (secondary_count))%NUM_CHUNKSERVERS;
				secondary_count = (secondary_count+1)%(NUM_CHUNKSERVERS-1)+1;
				e.data = c;
				((file_info*)ep->data)->num_of_chunks++;
				hsearch_r(e,ENTER,&ep_temp,((file_info*)ep->data)->chunk_list);
				//TODO: build response structure for client
				//Send a message to client
			}
			break;

	}
}

/* Thread to handle requests from clients */
void* client_request_listener(void* ptr){
	struct msghdr *msg;
	static int id = 0;
	int soc;

	#ifdef DEBUG
		printf("this thread listens connection requests from clients\n");
	#endif
	
	while(1) {

		soc = acceptConnection(client_request_socket);
		#ifdef DEBUG
			printf("connected to client id-%d\n", ++id);
		#endif
		if (thr_id == MAX_THR) {
			thr_id = 0;
		}
		if((pthread_create(&threads[thr_id++], NULL, handle_client_request, (void*)soc)) != 0) {
			printf("%s: Failed to create thread to handle client requests %d\n", __func__, thr_id);
		}
	}
}

/* Heartbeat thread */
void* heartbeat_thread(void* ptr)
{
	int index = (int)ptr, retval;

	struct msghdr *msg;
	prepare_msg(HEARTBEAT, &msg, &index, sizeof(index));
	char buf[200];
	int id = 0;	
	#ifdef DEBUG
		printf("this thread sends heartbeat messages to chunkserver %d\n",index);
	#endif

	while(1) {

		#ifdef DEBUG
			sprintf(buf, "Sending heartbeat message to chunkserver %d to socket %d\n", index, chunk_servers[index].conn_socket);
			write(1, buf, strlen(buf));
		#endif
		/* Send heartbeat request to chunkserver */
		retval = sendmsg(chunk_servers[index].conn_socket, msg, 0);
		//retval = send(chunk_servers[index].conn_socket, "Hi", 3, 0);
		if (retval == -1) {
			chunk_servers[index].is_up = 0;
			// TODO : handle failure of chunkserver by re-replication
			sprintf(buf, "Chunkserver-%d is down errno = %d\n",index, errno);
			write(1, buf, strlen(buf));
			break;
		} else {
		#ifdef DEBUG
			sprintf(buf, "\nSent heartbeat message-%d to Chunkserver-%d\n", ++id, index);
			write(1, buf, strlen(buf));
		#endif
		}	

		/* Wait for heartbeat reply from chunkserver */
		retval = recvmsg(chunk_servers[index].conn_socket, msg, 0);
		if (retval == -1) {
			chunk_servers[index].is_up = 0;
			// TODO : handle failure of chunkserver by re-replication
			sprintf(buf, "Chunkserver-%d is down\n",index);
			write(1, buf, strlen(buf));
			break;
		} else {
		#ifdef DEBUG
			sprintf(buf, "Received heartbeat ACK-%d from chunkserver-%d\n", id, index);
			write(1, buf, strlen(buf));
		#endif
		}

		/* Heartbeat is exchanged every 5 sec */
		sleep(5);
	}
}
