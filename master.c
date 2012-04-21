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

int failover_array[6][3]={	{0,2,3},	//0,1
				{0,1,3},	//0,2
				{0,1,2},	//0,3
				{0,0,3},	//1,2
				{0,0,2},	//1,3
				{0,0,1}		//2,3
			};


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

int add_tochunklist(int cid,int other_id, char* chunk_handle)
{
	chunklist_node *ptr = (chunklist_node*)malloc(sizeof(chunklist_node));
	if(ptr == NULL){
		return -1;
	}
	strcpy(ptr->chunk_handle,chunk_handle);
	ptr->other_cs = other_id;
	ptr->moved_cs = -1;
	ptr->next = NULL;
	if(chunk_servers[cid].head == NULL){
		chunk_servers[cid].head = chunk_servers[cid].tail = ptr;
	} else {
		chunk_servers[cid].tail->next = ptr;
		chunk_servers[cid].tail = chunk_servers[cid].tail->next;		
	}
	return 0;
}

void re_replicate(int index)
{
	int new_cs;
	chunklist_node *ptr = chunk_servers[index].head;	
	if(ptr==NULL)	return;
	while(ptr->next){
		chunklist_node *ptr1 = (chunklist_node*)malloc(sizeof(chunklist_node));
		strcpy(ptr1->chunk_handle,ptr->chunk_handle);
		ptr1->other_cs = ptr->other_cs;
		new_cs = failover_array[index + ptr->other_cs + !index][failover_array[index + ptr->other_cs + !index][0]+1];
		failover_array[index + ptr->other_cs + !index][0] = !failover_array[index + ptr->other_cs + !index][0];	
		ptr->moved_cs = new_cs;
		ptr1->moved_cs = -1;
		//e.key = ptr1->chunk_handle;
		//find the chunk and update the new chunk server
		//read a block from chunk server
		//write the block to other chunk server
	}
}

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
	//strcpy(master.ip_addr, master_ip);	
	getSelfIp(&master);
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

/*	if (argc != 2) {
		printf("run as : ./master <master-ip-address>\n");
		return 0;
	}*/

	/* Initialize master */
	retval = master_init();
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
			chunk_servers[j].head = chunk_servers[j].tail = NULL;
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
	FILE *filelist;
	struct msghdr *msg;
	int soc  = (int)arg;
	ENTRY e,*ep,*ep_temp;
	struct timeval tv;
	int retval = 0;

	open_req *open_req_obj;
	write_req *write_req_obj;
	read_req *read_req_obj;

	char * data = (char *) malloc(MAX_BUF_SZ);
	prepare_msg(0, &msg, data, MAX_BUF_SZ);
	recvmsg(soc, msg, 0);

	dfs_msg *dfsmsg;
	dfsmsg = (dfs_msg*)msg->msg_iov[0].iov_base;	
	#ifdef DEBUG
	printf("received message from client\n");
	#endif
	
	switch (dfsmsg->msg_type) {

		case CREATE_REQ:
			#ifdef DEBUG
			printf("received create request from client\n");
			#endif
			free_msg(msg);
			break;

		case OPEN_REQ:
			#ifdef DEBUG
			printf("received open request from client\n");
			#endif
			open_req_obj = (open_req*) msg->msg_iov[1].iov_base;
			e.key = open_req_obj->path;
			#ifdef DEBUG
				printf("received open request for file - %s flags - %d, check - %d\n", e.key, open_req_obj->flags, open_req_obj->flags & O_CREAT);
			#endif

			/* Search for file in master file list */
			if(hsearch_r(e,FIND,&ep,file_list) == 0) {

				/* File is being created */
				if(open_req_obj->flags & O_CREAT) {
					
					/* Create new object for thus file */
					file_info *new_file = (file_info*)malloc(sizeof(file_info));
					/* Initialize hash table for chunks of this file */
					new_file->chunk_list = (struct hsearch_data*)calloc(1, sizeof(struct hsearch_data));
					if (new_file->chunk_list == NULL) {
						printf("%s : Not enough memory\n", __func__);
						retval = -1;
						break;
					}
					int retval = hcreate_r(10,new_file->chunk_list);
					if (retval == 0) {
						printf("%s : Failed to create file hashtable\n", __func__);
						retval = -1;
						break;
					}

					/* Initialize file stats */
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
					if(hsearch_r(e, ENTER, &ep, file_list) == 0){
						printf("Error creating file %s\n",open_req_obj->path);
						retval = -1;
					} else {
						filelist = fopen(".filelist","ab+");
						fwrite(new_file,sizeof(file_info),1,filelist);
						#ifdef DEBUG
							printf("written to file list\n");
						#endif
						retval = 0;			
						fclose(filelist);
					}
				/* File is being opened */
				} else {
					#ifdef DEBUG	
						printf("File not found at master\n");
					#endif
					retval = -1;
				}

			/* File exists */
			} else {
				/* File to be created already exists - failure */
				if(open_req_obj->flags & O_CREAT){
					if ((open_req_obj->flags & O_RDONLY) || (open_req_obj->flags & O_RDWR) || (open_req_obj->flags & O_APPEND)) {
						#ifdef DEBUG	
						printf("File opened for read/write\n");
						#endif
						retval = 0;
					} else {
						#ifdef DEBUG	
						printf("File already present\n");
						#endif
						retval = -1;
					}
				/* File to to opened is found at master - success */
				} else {
					retval = 0;
				}
			}

			/* Send reply to client */
			dfsmsg->status = retval;
			dfsmsg->msg_type = OPEN_RESP;
			sendmsg(soc, msg, 0);
			free_msg(msg);
			break;

		case GETATTR_REQ:
			#ifdef DEBUG
			printf("received getattr request from client\n");
			#endif
			e.key = (char*)msg->msg_iov[1].iov_base;
			#ifdef DEBUG
			printf("file is : %s\n", e.key);
			#endif

			/* File not found */
			if(hsearch_r(e,FIND,&ep,file_list) == 0) {
				#ifdef DEBUG
				printf("File not present - %s\n", e.key);
				#endif
				retval = -ENOENT;
			/* File found */
			} else {
				char str[200];
				sprintf(str,"%lu %lu ", ((file_info*)ep->data)->filestat.st_ino, ((file_info*)ep->data)->filestat.st_size);
				msg->msg_iov[1].iov_base = str; 
				msg->msg_iov[1].iov_len = 200;
				printf("ino size = %s\n", (char*)msg->msg_iov[1].iov_base);	
				retval = 0;
			}

			/* Send reply to client */
			dfsmsg->status = retval;
			dfsmsg->msg_type = GETATTR_RESP;
			sendmsg(soc, msg, 0);
			free_msg(msg);
			break;

		case READDIR_REQ:
			#ifdef DEBUG
			printf("received readdir request from client\n");
			#endif
			free_msg(msg);
			break;

		case READ_REQ:
			#ifdef DEBUG
			printf("received read request from client\n");
			#endif
			read_req_obj = msg->msg_iov[1].iov_base;
			e.key = read_req_obj->filename;

			/* File not found */
			if(hsearch_r(e,FIND,&ep,file_list) == 0) {
				#ifdef DEBUG
				printf("File not present\n");
				#endif
				retval = -ENOENT;
			/* File found */
			} else {
				/* TODO : check if the read size of within the chunk size.. current size variable must be maintained within chunk_info */
				if(((file_info*)ep->data)->num_of_chunks <= read_req_obj->chunk_index){
					#ifdef DEBUG
					printf("Read error - no data available - no. of chunks = %d requested index = %d\n",
					((file_info*)ep->data)->num_of_chunks, read_req_obj->chunk_index);
					#endif
					retval = -ENOENT;
				} else {

					/* Prepare chunk object */	
					char temp[10];
					sprintf(temp, "%d", read_req_obj->chunk_index);
					e.key = temp;

					/* Chunk not found in hashtable */
					if (hsearch_r(e, FIND, &ep_temp, ((file_info*)ep->data)->chunk_list) == 0) {
						#ifdef DEBUG
						printf("Read error - chunk does not exist\n");
						#endif
						retval = -ENODATA;
					/* Chunk found in hashtable */
					} else {
						read_resp * resp = (read_resp*) malloc(sizeof(read_resp));
						chunk_info *c = (chunk_info*)ep_temp->data;
						c->last_read = (c->last_read + 1) % 2;	
						/* Prepare response */
						strcpy(resp->ip_address, chunk_servers[c->chunkserver_id[c->last_read]].ip_addr);
						resp->port = chunk_servers[c->chunkserver_id[c->last_read]].client_port;
						strcpy(resp->chunk_handle, c->chunk_handle);
						msg->msg_iov[1].iov_base = resp;
						msg->msg_iov[1].iov_len = sizeof(read_resp);
						retval = 0;
					}
				}

			}

			/* Send reply to client */
			dfsmsg->status = retval;
			dfsmsg->msg_type = READ_RESP;
			sendmsg(soc, msg, 0);
			free_msg(msg);
			break;

		case WRITE_REQ:
			#ifdef DEBUG
				printf("received write request from client\n");
			#endif
			write_req_obj = msg->msg_iov[1].iov_base;
			e.key = write_req_obj->filename;

			/* File not found */
			if(hsearch_r(e,FIND,&ep,file_list) == 0){
				#ifdef DEBUG
				printf("File not present\n");
				#endif
				retval = -ENOENT;
			/* File found */
			} else {
				/* For now, Only next chunk can be appended */
				/* TODO : Allow append operation within the same chunk - in this case no need to create a new chunk
				          Also, a current size variable must be maintained within chunk_info */
				if(((file_info*)ep->data)->num_of_chunks != write_req_obj->chunk_index){
					#ifdef DEBUG
					printf("Error not an append operation\n");
					#endif
					retval = -EPERM;
				} else {

					/* Prepare chunk object */	
					pthread_mutex_lock(&seq_mutex);
					chunk_id++;
					pthread_mutex_unlock(&seq_mutex);
					chunk_info *c = (chunk_info*)malloc(sizeof(chunk_info));
					sprintf(c->chunk_handle, "%d", chunk_id);
					char temp[10];
					sprintf(temp, "%d", write_req_obj->chunk_index);
					e.key = temp;

					//TODO: what to do if a chunkserver is down - then it cannot be the primary or secondary for the new chunk

					/* Assign primary chunkserver */
					c->chunkserver_id[0] = chunk_id % NUM_CHUNKSERVERS;				

					/* Assign secondary chunkserver */
					c->chunkserver_id[1] = (c->chunkserver_id[0] + (secondary_count))%NUM_CHUNKSERVERS;
					pthread_mutex_lock(&seq_mutex);
					secondary_count = (secondary_count+1)%(NUM_CHUNKSERVERS-1)+1;
					pthread_mutex_unlock(&seq_mutex);
					c->last_read = 1;
					add_tochunklist(c->chunkserver_id[0],c->chunkserver_id[1],c->chunk_handle);
					add_tochunklist(c->chunkserver_id[1],c->chunkserver_id[0],c->chunk_handle);

					/* Enter into hashtable */
					e.data = c;
					((file_info*)ep->data)->num_of_chunks++;
					((file_info*)ep->data)->filestat.st_size += CHUNK_SIZE;
					hsearch_r(e,ENTER,&ep_temp,((file_info*)ep->data)->chunk_list);

					/* Prepare response */
					write_resp * resp = (write_resp*) malloc(sizeof(write_resp));
					strcpy(resp->ip_address_primary, chunk_servers[c->chunkserver_id[0]].ip_addr);
					resp->port_primary = chunk_servers[c->chunkserver_id[0]].client_port;
					strcpy(resp->ip_address_secondary, chunk_servers[c->chunkserver_id[1]].ip_addr);
					resp->port_secondary = chunk_servers[c->chunkserver_id[1]].client_port;
					strcpy(resp->chunk_handle, c->chunk_handle);
					msg->msg_iov[1].iov_base = resp;
					msg->msg_iov[1].iov_len = sizeof(write_resp);

					retval = 0;
				}

			}

			/* Send reply to client */
			dfsmsg->status = retval;
			sendmsg(soc, msg, 0);
			dfsmsg->msg_type = WRITE_RESP;
			free_msg(msg);
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

		#ifdef DEBUG1
			sprintf(buf, "Sending heartbeat message to chunkserver %d to socket %d\n", index, chunk_servers[index].conn_socket);
			write(1, buf, strlen(buf));
		#endif
		/* Send heartbeat request to chunkserver */
		retval = sendmsg(chunk_servers[index].conn_socket, msg, 0);
		//retval = send(chunk_servers[index].conn_socket, "Hi", 3, 0);
		if (retval == -1) {
			chunk_servers[index].is_up = 0;
			re_replicate(index);
			sprintf(buf, "Chunkserver-%d is down errno = %d\n",index, errno);
			write(1, buf, strlen(buf));
			break;
		} else {
		#ifdef DEBUG1
			sprintf(buf, "\nSent heartbeat message-%d to Chunkserver-%d\n", ++id, index);
			write(1, buf, strlen(buf));
		#endif
		}	

		/* Wait for heartbeat reply from chunkserver */
		retval = recvmsg(chunk_servers[index].conn_socket, msg, 0);
		if (retval == -1) {
			chunk_servers[index].is_up = 0;
			re_replicate(index);
			sprintf(buf, "Chunkserver-%d is down\n",index);
			write(1, buf, strlen(buf));
			break;
		} else {
		#ifdef DEBUG1
			sprintf(buf, "Received heartbeat ACK-%d from chunkserver-%d\n", id, index);
			write(1, buf, strlen(buf));
		#endif
		}

		/* Heartbeat is exchanged every 5 sec */
		sleep(5);
	}
}
