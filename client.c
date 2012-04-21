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
struct msghdr *msg;
host master;
int master_soc;
pthread_mutex_t seq_mutex;

static int gfs_getattr(const char *path, struct stat *stbuf)
{
	int ret;
	char * buf = (char*) malloc (MAX_BUF_SZ);
	strcpy(buf, path);
	prepare_msg(GETATTR_REQ, &msg, buf, MAX_BUF_SZ);
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
		printf("%s: Getattr request sent\n",__func__);
	#endif
	}

	/* reply from master */
	if((recvmsg(master_soc,msg,0))==-1){
		printf("%s: message receipt failed - %d\n",__func__, errno);
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
	return 0;
}

static int gfs_mkdir(const char *path, mode_t mode)
{
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
		create_read_req(&read_ptr,path,i);
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
		create_read_data_req(&data_ptr,chunk_handle);
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
	                return -1;
        	} else {
                	printf("%s: Success: Received read reply from chunkserver\n",__func__);
		}

		/*TODO: process received data */
        	dfsmsg =  msg->msg_iov[0].iov_base;
		if(dfsmsg->status == 0) {
			/* Update number of bytes read */
			size_read += CHUNK_SIZE;
			resp = msg->msg_iov[1].iov_base;
			memcpy(buf, resp->chunk, CHUNK_SIZE); 
			#ifdef DEBUG
			int i;
			printf("Data read is - \n");
			for (i = 0; i < CHUNK_SIZE; i++) 
				printf("%c", buf[i]);
			printf("\n");
			#endif
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
        int ret;
	int i;
	int chunk_soc;
	write_req write_ptr;
	host chunk_server[2];
	char chunk_handle[64];
	size_t write_size = 0;	
	int start_block = offset/CHUNK_SIZE;
	int last_block = (offset+size-1)/CHUNK_SIZE;

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
		create_write_req(&write_ptr, path, i);
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
		create_write_data_req(data_ptr,resp->chunk_handle,buf+(i-start_block)*CHUNK_SIZE);
		#ifdef DEBUG
		int i;
		printf("Data to be written is - \n");
		for (i = 0; i < CHUNK_SIZE+64; i++) 
			printf("%c", data_ptr->chunk[i]);
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
			free(data_ptr);
			/* TODO : send rollback to secondary chunkserver */
	                return -1;
		}
		write_size += CHUNK_SIZE;
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
	printf("This is read directory\n");
	return 0;

//client side code goes here
/*        printf("Inside getdir Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
    sprintf(tcp_buf,"GETDIR\n%s\n",path);


//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        int recFlag=recv(sock,tcp_buf,MAXLEN,0);
    if(recFlag<0){
      printf("Error while receiving");
      exit(1);
    }

        send(sock,"ok",strlen("ok"),0);
    int flag=1;
    while(1)
      {
        struct stat tempSt;
        recv(sock,(char*)&tempSt,sizeof(struct stat),0);
            send(sock,"ok",strlen("ok"),0);
        int recFlag=recv(sock,tcp_buf,MAXLEN,0);
        if(recFlag<0){
          printf("Error while receiving");
          exit(1);
        }
        tcp_buf[recFlag]='\0';
	if(strcmp(tcp_buf,"end")==0)
	  break;
        if (filler(buf, tcp_buf, &tempSt, 0))
          {
	    flag=0;
	    break;
          }
      }
//rest of the code goes here
        return 0;*/
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
	//.chmod = (void *)dfs_chmod,
	//.chown = (void *)dfs_chown,
	//.rmdir = (void *)gfs_rmdir,
	//.rename = (void *)gfs_rename,
	//.flush = (void*)dfs_flush, 
	//.utimens = (void*)dfs_utimens,
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

