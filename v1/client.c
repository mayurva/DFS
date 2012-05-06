//This file contains main code for the client
#define FUSE_USE_VERSION 26
#include"gfs.h"
#include"client.h"
#include<fuse.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<dirent.h>
pthread_mutex_t seq_mutex;
host master;
#define DEBUG
static int gfs_getattr(const char *path, struct stat *stbuf)
{
	struct msghdr *msg;
	int master_soc;
	int ret;
	char * buf = (char*) malloc (1000);
	strcpy(buf, path);
	prepare_msg(GETATTR_REQ, &msg, buf, 1000);
	print_msg(msg->msg_iov[0].iov_base);

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
		free(buf);
                return -1;
        }

        if(createConnection(master,master_soc) == -1){
                printf("%s: can not connect to the master server - error-%d\n",__func__, errno);
		free(buf);
                return -1;
        }

	/* send a message to master */
	if((sendmsg(master_soc,msg,0))==-1){
		printf("%s: message sending failed - %d\n",__func__, errno);
		free(buf);
		return -1;
	} else {
	#ifdef DEBUG
		printf("%s: Getattr request sent\n",__func__);
	#endif
	}
	free(msg);
	prepare_msg(GETATTR_REQ, &msg, buf, 1000);
	/* reply from master */
	if((recvmsg(master_soc,msg,0))==-1){
		printf("%s: message receipt failed - %d\n",__func__, errno);
		free(buf);
		return -1;
	} else {
	#ifdef DEBUG
		printf("%s: Getattr response received from server\n",__func__);
	#endif
	}
	close(master_soc);

	/* if failure return -errno */
	dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
	if(dfsmsg->status != 0){
	#ifdef DEBUG
		printf("%s: Getattr status is - %d\n",__func__, dfsmsg->status);
	#endif
		free(buf);
		return dfsmsg->status;
	}

	/* TODO : Only size and inode nummer is received from master right now */
	char *str = msg->msg_iov[1].iov_base;
	#ifdef DEBUG
		printf("%s: Getattr str - %s\n",__func__, str);
	#endif
	char ino[10], size[10];
	int i = 0, j = 0;
	while (str[i] != ' ') {
		ino[j++] = str[i++];
	}
	ino[j] = '\0';
	i++;
	j = 0;
	while (str[i] != ' ') {
		size[j++] = str[i++];
	}
	size[j] = '\0';
	lstat("./client.c", stbuf);
	stbuf->st_ino = atol(ino);	
	stbuf->st_size = atol(size);	
	
	#ifdef DEBUG
		printf("%s: Getattr status is - %d ino = %llu file size = %llu\n",__func__
			, dfsmsg->status, stbuf->st_ino, stbuf->st_size);
	#endif
	free(buf);
	return 0;
}

static int gfs_mkdir(const char *path, mode_t mode)
{
	struct msghdr *msg;
	int master_soc;
       	int ret;
	mkdir_req data_ptr;
	create_mkdir_req(&data_ptr,path,mode);
        prepare_msg(MAKE_DIR_REQ, &msg, &data_ptr, sizeof(mkdir_req)); 

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }


        if(createConnection(master,master_soc) == -1){
		printf("%s: can not connect to the master server\n",__func__);
		return -1;
	}
	print_msg(msg->msg_iov[0].iov_base);

	/* send a message to master */
        if((sendmsg(master_soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }

        /* reply from master */
        if((recvmsg(master_soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
	close(master_soc);

        /* if failure return -errno */
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        if(dfsmsg->status != 0){
                return dfsmsg->status;
        }
        return 0;
}

static int gfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	struct msghdr *msg;
	int master_soc;
        int ret;
	open_req data_ptr;
	create_open_req(&data_ptr,path,fi->flags | O_CREAT);
	prepare_msg(OPEN_REQ, &msg, &data_ptr, sizeof(open_req));

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }

        if(createConnection(master,master_soc) == -1){
                printf("%s: can not connect to the master server\n",__func__);
                return -1;
        }
	#ifdef DEBUG
	print_msg(msg->msg_iov[0].iov_base);
	printf("flags are ------ %d\n", ((open_req*)msg->msg_iov[1].iov_base)->flags);
	#endif
	
	/* send a message to master */
        if((sendmsg(master_soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }

        /* reply from master */
        if((recvmsg(master_soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
	close(master_soc);
        
	/* if failure return -errno */
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        if(dfsmsg->status!=0){
                return dfsmsg->status;
        }
        return 0;
}

static int gfs_open(const char *path, struct fuse_file_info *fi)
{
	struct msghdr *msg;
	int master_soc;
        int ret;
	open_req data_ptr;
	create_open_req(&data_ptr,path,fi->flags);
	prepare_msg(OPEN_REQ, &msg, &data_ptr, sizeof(open_req));

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }

        if(createConnection(master,master_soc) == -1){
                printf("%s: can not connect to the master server\n",__func__);
                return -1;
        }
	#ifdef DEBUG
	print_msg(msg->msg_iov[0].iov_base);
	#endif
	
	/* send a message to master */
        if((sendmsg(master_soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }

        /* reply from master */
        if((recvmsg(master_soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
	close(master_soc);

        /* if failure return -errno */
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        if(dfsmsg->status!=0){
                return dfsmsg->status;
        }
        return 0;
}

static int gfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	struct msghdr *msg;
	int master_soc;
	int chunk_soc;
	host chunk_server;
	int ret;
	int i;
	int curr_offset;
	read_req read_ptr;
	read_data_req data_ptr;
	char chunk_handle[64];
	dfs_msg *dfsmsg;
	size_t size_read = 0;
	int chunk_size, chunk_offset;

	int start_block = offset/CHUNK_SIZE;
	int last_block = (offset+size-1)/CHUNK_SIZE;
	
	#ifdef DEBUG
	printf("start block = %d last block = %d size = %u offset = %llu\n", start_block, last_block, size, offset);
	#endif

	for(i=start_block;i<=last_block;i++) {
		#ifdef DEBUG
		printf("Chunk - %d\n", i);
		#endif

		if((master_soc = createSocket())==-1){
			printf("%s: Error creating socket\n",__func__);
			return -1;
		}

		if(createConnection(master,master_soc) == -1){
			printf("%s: can not connect to the master server\n",__func__);
			return -1;
		}

		/* Send metadata request to master */
		if ( i == start_block) {
			chunk_offset = offset % CHUNK_SIZE;
		} else {
			chunk_offset = 0;
		}
		if ((size - size_read + chunk_offset) > CHUNK_SIZE) {
			chunk_size = CHUNK_SIZE - chunk_offset;
		} else {
			chunk_size = size - size_read;
		}
		create_read_req(&read_ptr,path,i, chunk_offset, chunk_size);
		prepare_msg(READ_REQ, &msg, &read_ptr, sizeof(read_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(master_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}

        	/* reply from master - chunk metadata and location info */
        	if((recvmsg(master_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	} else {
			dfsmsg =  msg->msg_iov[0].iov_base;
			/* No metadata for this chunk */
			if (dfsmsg->status != 0)
				break;
		}
		close(master_soc);

		/* extract chunkserver details */
		strcpy(chunk_server.ip_addr,((read_resp*)dfsmsg -> data) ->ip_address);
		chunk_server.port = ((read_resp*)dfsmsg->data) ->port;
		strcpy(chunk_handle,((read_resp*)dfsmsg->data) ->chunk_handle);

		/* conect to the chunk server */
	        if((chunk_soc = createSocket())==-1){
                	printf("%s: Error creating socket\n",__func__);
                	return -1;
        	}
		if(createConnection(chunk_server,chunk_soc) == -1){
			printf("%s: can not connect to the chunk server\n",__func__);
			return -1;
		}
		/* Preapare read-data request */
		create_read_data_req(&data_ptr,chunk_handle, chunk_offset, chunk_size);
		free_msg(msg);
		prepare_msg(READ_DATA_REQ, &msg, &data_ptr, sizeof(read_data_req));
		print_msg(msg->msg_iov[0].iov_base);

		/* Send read-data request to chunkserver */
		if((sendmsg(chunk_soc,msg,0))==-1){
                	printf("%s: read request sending to chunkserver failed\n",__func__);
	                return -1;
        	} else {
                	printf("%s: Success: Sent read request\n",__func__);
		}

        	/* Receive read-data reply from chunkserver */
		free_msg(msg);
		read_data_resp * resp = (read_data_resp*) malloc(sizeof(read_data_resp));
		prepare_msg(READ_DATA_RESP, &msg, resp, sizeof(read_data_resp));
        	if((recvmsg(chunk_soc,msg,0))==-1){
                	printf("%s: read reply from chunkserver failed\n",__func__);
			free(resp);
	                return -1;
        	} else {
                	printf("%s: Success: Received read reply from chunkserver\n",__func__);
		}

		/*TODO: process received data */
        	dfsmsg =  msg->msg_iov[0].iov_base;
		if(dfsmsg->status == 0) {
			/* Update number of bytes read */
			resp = msg->msg_iov[1].iov_base;
			memcpy(buf+size_read, resp->chunk, resp->size); 
			#ifdef DEBUG
			int j;
			printf("Data read is - \n");
			for (j = 0; j < CHUNK_SIZE; j++) 
				printf("%c", buf[j+size_read]);
			printf("\n");
			#endif
			size_read += resp->size;
			if (resp->size < chunk_size) {
				break;
			}
		}
		free(resp);
		free_msg(msg);
		close(chunk_soc);
	}

	/* Return number of bytes written */
	#ifdef DEBUG
	printf("No. of bytes Read successfully - %d\n", size_read);
	#endif
	return size_read;

}

static int gfs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
	struct msghdr *msg;
	int master_soc;
        int ret;
	int i;
	int chunk_soc;
	write_req write_ptr;
	host chunk_server[2];
	char chunk_handle[64];
	size_t write_size = 0;	
	int start_block = offset/CHUNK_SIZE;
	int last_block = (offset+size-1)/CHUNK_SIZE;
	int chunk_size, chunk_offset;

	for(i=start_block;i<=last_block;i++){
		if((master_soc = createSocket())==-1){
			printf("%s: Error creating socket\n",__func__);
			return -1;
		}

		if(createConnection(master,master_soc) == -1){
			printf("%s: can not connect to the master server\n",__func__);
			return -1;
		}

		/* Send metadat request to master */
		if ( i == start_block) {
			chunk_offset = offset % CHUNK_SIZE;
		} else {
			chunk_offset = 0;
		}
		if ((size - write_size + chunk_offset) > CHUNK_SIZE) {
			chunk_size = CHUNK_SIZE - chunk_offset;
		} else {
			chunk_size = size - write_size;
		}
		create_write_req(&write_ptr, path, i, chunk_offset, chunk_size);
		prepare_msg(WRITE_REQ, &msg, &write_ptr, sizeof(write_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(master_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}

        	/* Matadata reply from master */
        	if((recvmsg(master_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		close(master_soc);

		/* Extract primary and secondary chunkserver details */
	 	dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
		if (dfsmsg->status != 0) {
                	printf("%s: Error creating chunk on master\n",__func__);
	                return -1;
        	}
		write_resp * resp = msg->msg_iov[1].iov_base;
		strcpy(chunk_server[0].ip_addr,resp->ip_address_primary);
		chunk_server[0].port = resp->port_primary;
		strcpy(chunk_server[1].ip_addr,resp->ip_address_secondary);
		chunk_server[1].port = resp->port_secondary;
		strcpy(chunk_handle, resp->chunk_handle);
		#ifdef DEBUG	
		printf("Write response - primary-%s:%d secondary-%s:%d chunkhandle-%s\n", 
		resp->ip_address_primary, resp->port_primary, resp->ip_address_secondary, resp->port_secondary, resp->chunk_handle);	
		#endif

		/* Create connection with secondary chunkserver */
	        if((chunk_soc = createSocket())==-1){
                	printf("%s: Error creating socket\n",__func__);
	                return -1;
        	}

		if(createConnection(chunk_server[1],chunk_soc) == -1){
			printf("%s: can not connect to the chunk server\n",__func__);
			return -1;
		}

		/* Prepare write data request */
		write_data_req *data_ptr = (write_data_req*) malloc (sizeof(write_data_req));
		create_write_data_req(data_ptr, resp->chunk_handle, buf+ write_size, chunk_offset, chunk_size);
		#ifdef DEBUG
		int j;
		printf("Data to be written is - \n");
		for (j = 0; j < CHUNK_SIZE+64; j++) 
			printf("%c", data_ptr->chunk[j]);
//		printf("\nhandle - %s\n", data_ptr->chunk+CHUNK_SIZE);
		printf("\n");
		#endif
		free_msg(msg);
		prepare_msg(WRITE_DATA_REQ, &msg, data_ptr, sizeof(write_data_req));
		print_msg(msg->msg_iov[0].iov_base);

		/* Send write-data request to secondary chunkserver */
		if((sendmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
			free_msg(msg);
			free(data_ptr);
	                return -1;
        	} else {
			printf("%s: Sent write request to secondary chunkserver\n",__func__);
		}
		
        	/* Reply from secondary chunkserver */
        	if((recvmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
			free_msg(msg);
			free(data_ptr);
	                return -1;
        	} else {
			printf("%s: Received write reply from secondary chunkserver\n",__func__);
		}
		close(chunk_soc);

		/* If secondary chunkserver failed to write data - report failure to client */
	 	dfsmsg =  msg->msg_iov[0].iov_base;
		if (dfsmsg->status != 0) {
                	printf("%s: Error writing data to secondary chunserver\n",__func__);
			free_msg(msg);
			free(data_ptr);
	                return -1;
		}

		/* create connection with primary chunk server */
	        if((chunk_soc = createSocket())==-1){
                	printf("%s: Error creating socket\n",__func__);
			free_msg(msg);
			free(data_ptr);
                	return -1;
		}
		if(createConnection(chunk_server[0],chunk_soc) == -1){
			printf("%s: can not connect to the chunk server\n",__func__);
			free_msg(msg);
			free(data_ptr);
			return -1;
		}

		/* Send write-data to primary chunkserver */
		free_msg(msg);
		prepare_msg(WRITE_DATA_REQ, &msg, data_ptr, sizeof(write_data_req));
		if((sendmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
			free_msg(msg);
			free(data_ptr);
	                return -1;
        	} else {
			printf("%s: Sent write request to primary chunkserver\n",__func__);
		}

        	/* Reply from primary chunkserver */
        	if((recvmsg(chunk_soc,msg,0))==-1) {
                	printf("%s: message receipt failed\n",__func__);
			free_msg(msg);
			free(data_ptr);
	                return -1;
        	} else {
			printf("%s: Received write reply from primary chunkserver\n",__func__);
		}

		/* If Primary chunkserver failed to write data - report failure to client and secondary chunkserver */
	 	dfsmsg =  msg->msg_iov[0].iov_base;
		if (dfsmsg->status != 0) {
                	printf("%s: Error writing data to primary chunserver\n",__func__);
			free_msg(msg);
			/* TODO : send rollback to secondary chunkserver */
			if((chunk_soc = createSocket())==-1){
				printf("%s: Error creating socket\n",__func__);
				return -1;
			}
			if(createConnection(chunk_server[1],chunk_soc) == -1){
				printf("%s: can not connect to the chunk server\n",__func__);
				return -1;
			}
			prepare_msg(ROLLBACK_REQ, &msg, data_ptr, sizeof(write_data_req));
			if((sendmsg(chunk_soc,msg,0))==-1){
				printf("%s: message sending failed\n",__func__);
				free_msg(msg);
				free(data_ptr);
				return -1;
			} else {
				printf("%s: Sent write request to secondary chunkserver\n",__func__);
			}
			/* Reply from secondary chunkserver */
			if((recvmsg(chunk_soc,msg,0))==-1) {
				printf("%s: message receipt failed\n",__func__);
				free_msg(msg);
				free(data_ptr);
				return -1;
			} else {
				printf("%s: Received write reply from secondary chunkserver\n",__func__);
			}
			free(data_ptr);
			return -1;
		}
		if((master_soc = createSocket())==-1){
			printf("%s: Error creating socket\n",__func__);
			return -1;
		}

		if(createConnection(master,master_soc) == -1){
			printf("%s: can not connect to the master server\n",__func__);
			return -1;
		}
		create_write_req(&write_ptr, path, i, chunk_offset, chunk_size);
		prepare_msg(WRITE_COMMIT_REQ, &msg, &write_ptr, sizeof(write_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(master_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}

        	/* Matadata reply from master */
        	if((recvmsg(master_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		close(master_soc);
	 	dfsmsg =  msg->msg_iov[0].iov_base;
		if (dfsmsg->status != 0) {
                	printf("%s: Error commiting write on master\n",__func__);
	                return -1;
        	}
		write_size += chunk_size;
		close(chunk_soc);
		free_msg(msg);
		free(data_ptr);
	}
	/* Return number of bytes written */
#ifdef DEBUG
	printf("No. of bytes written successfully - %d\n", write_size);
	#endif
	return write_size;
}

static int gfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{
	struct msghdr *msg;
	int master_soc;
	struct stat st;
	int ret;
	char * tempbuf = (char*) malloc (1000);
	char *file_name = (char*) malloc(1000);
	strcpy(tempbuf, path);
	prepare_msg(READDIR_REQ, &msg, tempbuf, 1000);
	print_msg(msg->msg_iov[0].iov_base);

        if((master_soc = createSocket())==-1){
		printf("%s: Error creating socket\n",__func__);
		return -1;
        }

        if(createConnection(master,master_soc) == -1){
                printf("%s: can not connect to the master server - error-%d\n",__func__, errno);
                return -1;
        }

	/* send a message to master */
	if((sendmsg(master_soc,msg,0))==-1){
		printf("%s: message sending failed - %d\n",__func__, errno);
		return -1;
	} else {
	#ifdef DEBUG
		printf("%s: readdir request sent\n",__func__);
	#endif
	}

	/* reply from master */
	while(1){
		//free_msg(msg);
		//prepare_msg(READDIR_RESP, &msg, &st, sizeof(struct stat));
		if((recv(master_soc,file_name,1000,0))==-1){
			printf("%s: message receipt failed - %d\n",__func__, errno);
			return -1;
		} else {
		#ifdef DEBUG
			printf("%s: received filename %s\n",__func__,file_name);
		#endif
		}	
//		if(((dfs_msg*)(msg->msg_iov[0]).iov_base)->status == -1)
		if(strcmp(file_name,"END")==0)
			break;
		//dptr = &d;
		free_msg(msg);
		prepare_msg(READDIR_REQ, &msg, tempbuf, 1000);
		if((sendmsg(master_soc,msg,0))==-1){
		printf("%s: message sending failed - %d\n",__func__, errno);
			return -1;
		} else {
		#ifdef DEBUG
			printf("%s: request sent for stat\n",__func__);
		#endif
		}
		//memcpy(&st,msg->msg_iov[1].iov_base, sizeof(struct stat));
		//free_msg(msg);
		//memset(file_name,0,MAX_BUF_SZ);
		//prepare_msg(READDIR_RESP, &msg, file_name, sizeof(struct stat));
		if((recv(master_soc,&st,sizeof(struct stat),0))==-1){
			printf("%s: message receipt failed - %d\n",__func__, errno);
			return -1;
		} 
		//memcpy(file_name,msg->msg_iov[1].iov_base, MAX_BUF_SZ);
		#ifdef DEBUG
			printf("%s: received stat\n",__func__);
		//	printf("status is %d\n",((dfs_msg*)(msg->msg_iov[0]).iov_base)->status);
		#endif
	
		if (filler(buf, file_name+1, &st, 0)){	}

		free_msg(msg);
		prepare_msg(READDIR_REQ, &msg, tempbuf, 1000);
		if((sendmsg(master_soc,msg,0))==-1){
		printf("%s: message sending failed - %d\n",__func__, errno);
			return -1;
		} else {
			#ifdef DEBUG
				printf("%s: readdir request sent for next record\n",__func__);
			#endif
		}
		

	}
	close(master_soc);
}

static int gfs_unlink(const char *path)
{
	struct msghdr *msg;
	int master_soc;
        int ret;
	open_req data_ptr;
	create_open_req(&data_ptr,path, 0);
	prepare_msg(UNLINK_REQ, &msg, &data_ptr, sizeof(open_req));

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }

        if(createConnection(master,master_soc) == -1){
                printf("%s: can not connect to the master server\n",__func__);
                return -1;
        }
	#ifdef DEBUG
	print_msg(msg->msg_iov[0].iov_base);
	#endif
	
	/* send a message to master */
        if((sendmsg(master_soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }

        /* reply from master */
        if((recvmsg(master_soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
	close(master_soc);
        
	/* if failure return -errno */
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        if(dfsmsg->status!=0){
                return dfsmsg->status;
        }
        return 0;
}

static int gfs_chmod(const char *path, mode_t mode)
{
        return 0;
}

static int gfs_chown(const char *path, uid_t uid, gid_t gid)
{
        return 0;
}
static int gfs_utimens(const char *path, const struct timespec ts[2])
{
        return 0;
}


static struct fuse_operations gfs_oper = {
	.getattr = (void *)gfs_getattr,
	//.mknod = (void *)gfs_mknod,
	.mkdir = (void *)gfs_mkdir,
	.open = (void *)gfs_open,
	.create = (void *)gfs_create,
	.read = (void *)gfs_read,
	.write = (void *)gfs_write,
	.readdir = (void *)gfs_readdir,
	//.access = (void *)dfs_access,
	.chmod = (void *)gfs_chmod,
	.chown = (void *)gfs_chown,
	//.rmdir = (void *)gfs_rmdir,
	//.rename = (void *)gfs_rename,
	//.flush = (void*)dfs_flush, 
	.utimens = (void*)gfs_utimens,
	//.getxattr = (void*)dfs_getxattr,
	//.setxattr = (void*)dfs_setxattr,
};

int main(int argc, char *argv[])
{
	int i;
	if((client_init(argc,argv))==-1){
		printf("Error initializing client..exiting..\n");
		exit(-1);
	}
        for(i=1;i<argc;i++)
                argv[i] = argv[i+1];
        argc--;
        umask(0);

        return fuse_main(argc, argv, &gfs_oper, NULL);
}

