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
struct msghdr *msg;
host master;
int soc;
pthread_mutex_t seq_mutex;

static int gfs_getattr(const char *path, struct stat *stbuf)
{
	int ret;
	prepare_msg(GETATTR_REQ, msg, path, strlen(path));
	if(createConnection(master,soc) == -1){
		printf("%s: can not connect to the master server\n",__func__);
		return -1;
	}

	//send a message to master
	if((sendmsg(soc,msg,0))==-1){
		printf("%s: message sending failed\n",__func__);
		return -1;
	}
	//reply from master
	if((recvmsg(soc,msg,0))==-1){
		printf("%s: message receipt failed\n",__func__);
		return -1;
	}
	dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
	//if failure return -errno
	if(dfsmsg->status!=0){
		return -*(int*)dfsmsg->data;
	}
	//copy the data into stbuf (if required)
	stbuf = (struct stat*) dfsmsg->data;
	return 0;
/*	
//client side code goes here

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

	//TODO: create a data structure for mkdir req
        prepare_msg(MAKE_DIR_REQ, msg, path, strlen(path));
        if(createConnection(master,soc) == -1){
                printf("%s: can not connect to the master server\n",__func__);
                return -1;
        }

	//send a message to master
        if((sendmsg(soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }
        //reply from master
        if((recvmsg(soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        //if failure return -errno
        if(dfsmsg->status!=0){
                return -*(int*)dfsmsg->data;
        }
        return 0;
}

static int gfs_open(const char *path, struct fuse_file_info *fi)
{
        int ret;
	//TODO: create a data structure for open req
	prepare_msg(OPEN_REQ, msg, path, strlen(path));
        if(createConnection(master,soc) == -1){
                printf("%s: can not connect to the master server\n",__func__);
                return -1;
        }
	//send a message to master
        if((sendmsg(soc,msg,0))==-1){
                printf("%s: message sending failed\n",__func__);
                return -1;
        }
        //reply from master
        if((recvmsg(soc,msg,0))==-1){
                printf("%s: message receipt failed\n",__func__);
                return -1;
        }
        dfs_msg *dfsmsg =  msg->msg_iov[0].iov_base;
        //if failure return -errno
        if(dfsmsg->status!=0){
                return -*(int*)dfsmsg->data;
        }
        return 0;
}

static int gfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	int ret;
	int start_block = offset/CHUNK_SIZE;
	int last_block = (offset+size)/CHUNK_SIZE;
	int i;
	prepare_msg(OPEN_REQ, msg, path, strlen(path));
	if(createConnection(master,soc) == -1){
		printf("%s: can not connect to the master server\n",__func__);
		return -1;
	}
	
	//read first block
	for(i=start_block;i<last_block;i++){
		if((sendmsg(soc,msg,0))==-1){
                	printf("%s: message sending failed\n",__func__);
	                return -1;
        	}
        	//reply from master
        	if((recvmsg(soc,msg,0))==-1){
                	printf("%s: message receipt failed\n",__func__);
	                return -1;
        	}
		close(soc);
				
	}
	//read last block
	
	/*
	find the number of blocks and corresponding block numbers to be read.
	for each block to be read.. {
	        send a message to master with file name and block number
		reply from master with chunk id and chunk server details (ip and port I suppose)
		send a read request to chunkserver
		reply from chunkserver... if first/last block... then do extra overhead
	}
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
        /*strcpy(filepath,rootpath);
        strcat(filepath,filename);
	find the block number where to write.
        send a message to master with filename and block number
        reply from master.. failure if the write request is not an append to the last block...
	if successful server returns 2 chunckservers to write to
        send a write (append) request to each chunkserver
        if successful reply from both chunkservers write successful.
	There are multiple other options with write... to make sure of atomicity...
	1. Either server returns only 1 primary chunk server... and somehow notifies the primary tht xyz is secondary.. 
or primary after receiving write request checks with master regarding who is secondary... 
	Then primary forwards the request to secondary and on success replies to client tht write is successful.. ensures ATOMIC write
	2. Else.. client gets both chunk servers from master... while sending write request to primary.. the client also sends the details pof secondary.. 
	Then primary forwards the request to secondary and on success replies to client tht write is successful.. ensures ATOMIC write
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

int client_init(int argc,char* argv[])
{	
	pthread_mutex_init(&seq_mutex, NULL);
	populateIp(&master,argv[1]);
	if((soc = createSocket())==-1){
		printf("%s: Error creating socket\n",__func__);
		return -1;
	}
	master.port = MASTER_LISTEN;
	return 0;
}

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

