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
	prepare_msg(GETATTR_REQ, &msg, path, strlen(path));
	print_msg(msg->msg_iov[0].iov_base);

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }

        if(createConnection(master,master_soc) == -1){
                printf("%s: can not connect to the master server - error-%d\n",__func__, errno);
                return -1;
        }

	//send a message to master
	if((sendmsg(master_soc,msg,0))==-1){
		printf("%s: message sending failed - %d\n",__func__, errno);
		return -1;
	} else {
	#ifdef DEBUG
		printf("%s: Getattr request sent\n",__func__);
	#endif
	}
	//reply from master
	if((recvmsg(master_soc,msg,0))==-1){
		printf("%s: message receipt failed - %d\n",__func__, errno);
		return -1;
	} else {
	#ifdef DEBUG
		printf("%s: Getattr response received from server\n",__func__);
	#endif
	}
	close(master_soc);
	dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
	//if failure return -errno
	if(dfsmsg->status != 0){
		return -1;
	}
	//copy the data into stbuf (if required)
	stbuf = (struct stat*) dfsmsg->data;
	return 0;
/*	
_//client side code goes here

	stbuf->st_dev = temp_stbuf.st_dev;
	stbuf->st_ino = temp_stbuf.st_ino;
	stbuf->st_mode = temp_stbuf.st_mode;
	stbuf->st_nlink = temp_stbuf.st_nlink;
	stbuf->st_uid = temp_stbuf.st_uid;
	stbuf->st_gid = temp_stbuf.st_gid;
	stbuf->st_rdev = temp_stbuf.st_rdev;
	stbuf->st_size = temp_stbuf.st_size;
	stbuf->st_blksize = temp_stbuf.st_blksize;
	stbuf->st_blocks = temp_stbuf.st_blocks;
	stbuf->st_atime = temp_stbuf.st_atime;
	stbuf->st_mtime = temp_stbuf.st_mtime;
	stbuf->st_ctime = temp_stbuf.st_ctime;
*/
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

	//send a message to master
        if((sendmsg(master_soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }
        //reply from master
        if((recvmsg(master_soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
	close(master_soc);
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        //if failure return -errno
        if(dfsmsg->status != 0){
                return -1;
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
	print_msg(msg->msg_iov[0].iov_base);
	
	//send a message to master
        if((sendmsg(master_soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }
        //reply from master
        if((recvmsg(master_soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
	close(master_soc);
        //if failure return -errno
        if(dfsmsg->status!=0){
                return -1;
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

	int start_block = offset/CHUNK_SIZE;
	int last_block = (offset+size)/CHUNK_SIZE;
	
	if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }
	
	if(createConnection(master,master_soc) == -1){
		printf("%s: can not connect to the master server\n",__func__);
		return -1;
	}
	
	for(i=start_block;i<=last_block;i++){
		create_read_req(&read_ptr,path,i);
		prepare_msg(READ_REQ, &msg, &read_ptr, sizeof(read_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(master_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}
        	//reply from master
        	if((recvmsg(master_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		//extract chunkserver details
        	dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
		strcpy(chunk_server.ip_addr,((read_resp*)dfsmsg -> data) ->ip_address);
		chunk_server.port = ((read_resp*)dfsmsg->data) ->port;
		strcpy(chunk_handle,((read_resp*)dfsmsg->data) ->chunk_handle);
		//conect to the chunk server
	        if((chunk_soc = createSocket())==-1){
                	printf("%s: Error creating socket\n",__func__);
                	return -1;
        	}

		if(createConnection(chunk_server,chunk_soc) == -1){
			printf("%s: can not connect to the chunk server\n",__func__);
			return -1;
		}
		create_read_data_req(&data_ptr,chunk_handle);
		prepare_msg(READ_DATA_REQ, &msg, &data_ptr, sizeof(read_data_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}
        	//reply from master
        	if((recvmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		/*TODO: process received data
		if first/last block... then do extra overhead*/
		close(chunk_soc);
	}
	close(master_soc);
	/*
        if failure return -errno*/
	return 0;

/*  printf("Inside read. Path is: %s buf is %s",path,buf);
        sprintf(tcp_buf,"READ\n%s",path);
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);
	
	NOTE: we had to send some arg separately in a new msg.. not sure why was that...
	sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
	recv(sock,tcp_buf,MAXLEN,0);
	
	NOTE:if file was opened correctly at master
	if(strcmp(tcp_buf,"success")==0)
	  {
	    int nsize,noff;
	    NOTE: some math to find block numbers
	    noff=(int)offset/BLOCKSIZE;
	    nsize=(int)size/BLOCKSIZE+1;
	    tempBuf=(char *)malloc(sizeof(char)*(nsize*BLOCKSIZE));
	    strcpy(tempBuf,""); 
	    int i;
	    for(i=1;i<nsize;i++)
	      {
		      sprintf(tcp_buf,"%d",(noff*BLOCKSIZE));
		    send(sock,tcp_buf,strlen(tcp_buf),0);
		    
		    recvflag=recv(sock,tcp_buf,BLOCKSIZE,0);
		    if(recvflag<0)
		      {
			printf("Receiving error");
			exit(0);
		      }
		    
		    blocks temp;
		    temp.blockNumber=noff;
		    strcpy(temp.blockData,tcp_buf);

		    writeToFile(fd,&temp);
		    NOTE: some awkward copy-paste from temp buffer to actual buffer
		    retval=retval+recvflag;
			strcat(tempBuf,tcp_buf);

		    NOTE: special handling if file is smaller thn no of bytes to be read
		    if(strcmp(temp.blockData,"file_is_finished")==0 || recvflag<BLOCKSIZE){
		      send(sock,"next",strlen("next"),0);
		      printf("next\ni is %d and nsize is %d\n",i,nsize);		  
			exitflag=1;
			printf("Break from here");fflush(stdout);
		      break;
		    }
		noff++;
	      }
	printf("End of read\n");
*/
}

static int gfs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
        int ret;
	int i;
	int chunk_soc;
	write_req write_ptr;
	write_data_req data_ptr;
	host chunk_server[2];
	char chunk_handle[64];
	
	int start_block = offset/CHUNK_SIZE;
	int last_block = (offset+size)/CHUNK_SIZE;

        if((master_soc = createSocket())==-1){
                printf("%s: Error creating socket\n",__func__);
                return -1;
        }

        if(createConnection(master,master_soc) == -1){
		printf("%s: can not connect to the master server\n",__func__);
		return -1;
	}
	
	for(i=start_block;i<=last_block;i++){
		create_write_req(&write_ptr);
		prepare_msg(WRITE_REQ, &msg, &write_ptr, sizeof(write_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(master_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}
        	//reply from master
        	if((recvmsg(master_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		//extract chunkserver details
	 	dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
		strcpy(chunk_server[0].ip_addr,((write_resp*)dfsmsg->data) ->ip_address_primary);
		chunk_server[0].port = ((write_resp*)dfsmsg->data) ->port_primary;
		strcpy(chunk_server[1].ip_addr,((write_resp*)dfsmsg->data) ->ip_address_secondary);
		chunk_server[0].port = ((write_resp*)dfsmsg->data) ->port_secondary;
		strcpy(chunk_handle,((write_resp*)dfsmsg->data) ->chunk_handle);
		
		//create connection with secondary chunk server
	        if((chunk_soc = createSocket())==-1){
                	printf("%s: Error creating socket\n",__func__);
	                return -1;
        	}

		if(createConnection(chunk_server[1],chunk_soc) == -1){
			printf("%s: can not connect to the chunk server\n",__func__);
			return -1;
		}
		create_write_data_req(&data_ptr,chunk_handle,buf+(i-start_block)*CHUNK_SIZE);
		prepare_msg(WRITE_DATA_REQ, &msg, &data_ptr, sizeof(write_data_req));
		print_msg(msg->msg_iov[0].iov_base);
		if((sendmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}
        	//reply from chunkserver
        	if((recvmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		close(chunk_soc);
		//create connection with primary chunk server
	        if((chunk_soc = createSocket())==-1){
                	printf("%s: Error creating socket\n",__func__);
                	return -1;
        	}

		if(createConnection(chunk_server[0],chunk_soc) == -1){
			printf("%s: can not connect to the chunk server\n",__func__);
			return -1;
		}
		if((sendmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}
        	//reply from chunkserver
        	if((recvmsg(chunk_soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		//TODO:Send a confirmation message to primary to commit the write
		close(chunk_soc);
			
	}
	close(master_soc);
	/*
        if failure return -errno*/
	return 0;

/*	printf("Inside write. Path is: %s buf is %s",path,buf);
        sprintf(tcp_buf,"WRITE\n%s",path);

	char wrtFile[100];
	char *tempBuf;
	tempBuf=(char *)malloc(sizeof(char)*(int)size);
	FILE *fd;
	int recvflag;
       
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);
	
        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"%d",fi->flags);
	NOTE: we had to send some arg separately in a new msg.. not sure why was that...
        send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd (this should be success)\n%s\n",tcp_buf);
	
	NOTE: File was open successfully
	if(strcmp(tcp_buf,"success")==0)
	  {
	    sprintf(tcp_buf,"%d",(int)offset);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    strcpy(tempBuf,"");
	    int nsize,noff;

	    NOTE: calculation of blocks
	    noff=(int)offset/BLOCKSIZE;
	    nsize=(int)size/BLOCKSIZE;

	    recv(sock,tcp_buf,MAXLEN,0);
		printf("recvd\n%s\n",tcp_buf);
	    sprintf(tcp_buf,"%d",(int)size);
	    send(sock,tcp_buf,MAXLEN,0);
	    recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	    send(sock,buf,strlen(buf),0);
	    recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	  }
	     
	fclose(fd);
	printf("End of write\n");
	send(sock,"total",strlen("total"),0);
	int rf=recv(sock,tcp_buf,MAXLEN,0);
	tcp_buf[rf]='\0';
	int ret=atoi(tcp_buf);
        return ret;
*/
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

