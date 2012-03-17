//This file contains main code for the client
#define FUSE_USE_VERSION 26
#include"gfs.h"
#include"client.h"
#include<fuse.h>
#include<stdio.h>

static int gfs_getattr(const char *path, struct stat *stbuf)
{
	int ret;
	//strcpy(filepath,rootpath);
	//strcat(filepath,filename);
	//send a message to master
	//reply from master
	//if failure return -errno
	//copy the data into stbuf (if required)
	return 0;
/*	
//client side code goes here

		send(sock,tcp_buf,strlen(tcp_buf),0);
		recv(sock,(char *)&temp_stbuf,sizeof(struct stat),0);

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
        //strcpy(filepath,rootpath);
        //strcat(filepath,filename);
        //send a message to master
        //reply from master
        //if failure return -errno
        return 0;
/*

	//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);

        return res;*/
}

static int gfs_open(const char *path, struct fuse_file_info *fi)
{
        int ret;
        //strcpy(filepath,rootpath);
        //strcat(filepath,filename);
        //send a message to master
        //reply from master
        //if failure return -errno
        return 0;

/*  int ret=0;

        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);


        return ret;*/
}

static int gfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
	int ret;
	//strcpy(filepath,rootpath);
        //strcat(filepath,filename);
        //send a message to master
        //reply from master
	//send a read request to chunkserver
	//reply from chunkserver
        //if failure return -errno
	return 0;
/*  printf("Inside read. Path is: %s buf is %s",path,buf);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"READ\n%s",path);

	(void)fi;
	char cacheFile[100];
	char *tempBuf;
	FILE *fd;
	int cacheflag=0,exitflag=0;
	int ret=0,tempsize,res;
	int recvflag,retval=0;
	strcpy(cacheFile,".");
	strcat(cacheFile,path);
	strcat(cacheFile,".cache");
	fd=fopen(cacheFile,"ab+");
       
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
	
        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	
	if(strcmp(tcp_buf,"success")==0)
	  {
	    int nsize,noff;
	    noff=(int)offset/BLOCKSIZE;
	    nsize=(int)size/BLOCKSIZE+1;
	    tempBuf=(char *)malloc(sizeof(char)*(nsize*BLOCKSIZE));
	    strcpy(tempBuf,""); 
	    int i;
	    for(i=1;i<nsize;i++)
	      {
*/	/*************************cached data*******************************/
/*		if(searchFile(noff+1,fd)==1)
		  {
		    printf("\n\nfound!!!!!\n\n");
		    
		    cacheflag=1;
		    memset(tcp_buf,0,MAXLEN);
		    
		    FILE *fp;

  
		    blocks temp;  
		    fseek(fd,0,SEEK_SET);
		    while(fread(&temp,sizeof(blocks),1,fd))
		      {
		      
		      if(temp.blockNumber==noff+1)
			{
			tempsize=strlen(temp.blockData);
 printf("%s\n\n<3<3<3<3<3%d:: %d",temp.blockData,(int)size,tempsize);fflush(stdout);
			strcat(tempBuf,temp.blockData);
			ret=ret+tempsize;
			retval=retval+tempsize;
			printf("This happens");fflush(stdout);

			break;
		      }
		    }
		    if(tempsize<BLOCKSIZE)
		      {
			send(sock,"cache",strlen("cache"),0);
			printf("next\ni is %d and nsize is %d\n",i,nsize);
			exitflag=1;
			break;
		      }
		    if(i!=nsize)
		      {
			send(sock,"ok",strlen("ok"),0);
			printf("ok\ni is %d and nsize is%d\n",i,nsize);
		      }
		    else
		      {
			send(sock,"done",strlen("done"),0);
			printf("next\ni is %d and nsize is %d\n",i,nsize);
			exitflag=1;
			break;
		      }

		    fflush(stdout);

		  }
*/
		/*********************************if uncached***************************************************/
/*		else
		  {
		    
		    memset(tcp_buf,0,MAXLEN);

		      sprintf(tcp_buf,"%d",(noff*BLOCKSIZE));
		    send(sock,tcp_buf,strlen(tcp_buf),0);
		    
		    memset(tcp_buf,0,MAXLEN);
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
		    retval=retval+recvflag;
			strcat(tempBuf,tcp_buf);

		    if(strcmp(temp.blockData,"file_is_finished")==0 || recvflag<BLOCKSIZE){
		      send(sock,"next",strlen("next"),0);
		      printf("next\ni is %d and nsize is %d\n",i,nsize);		  
			exitflag=1;
			printf("Break from here");fflush(stdout);
		      break;
		    }

		    memset(tcp_buf,0,MAXLEN);
		    if(i!=nsize)
		      {
			send(sock,"ok",strlen("ok"),0);
			printf("ok\ni is %d and nsize is%d\n",i,nsize);
		      }
		    else
		      {
			send(sock,"next",strlen("next"),0);
			printf("next\ni is %d and nsize is %d\n",i,nsize);
		      }
		    fflush(stdout);

		  }
		printf("hi pathav ki");fflush(stdout);
		recv(sock,tcp_buf,MAXLEN,0);
		noff++;
	      }
	}
	else{

	    printf("sending errno");fflush(stdout);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    memset(tcp_buf,0,MAXLEN);
	    res=recv(sock,tcp_buf,MAXLEN,0);
	    if(res<0){
	      printf("\nError receiving flags");
	      exit(1);
	    }
	    tcp_buf[res]='\0';
	    ret=atoi(tcp_buf);

	    return ret;


	}

	
	if(exitflag==1)
	  recv(sock,tcp_buf,MAXLEN,0);

	fclose(fd);
	printf("End of read\n");
	printf("print");fflush(stdout);
	memcpy(buf,tempBuf+((int)offset%BLOCKSIZE),(int)size);
	buf[(int)size]='\0';
	printf("\n\nbuf,%s",buf);fflush(stdout);
	free(tempBuf);
	memset(tcp_buf,0,MAXLEN);
	printf("return %d:: %d",strlen(buf),(int)size);
        return (int)size;

*/
}

static int gfs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
        int ret;
        //strcpy(filepath,rootpath);
        //strcat(filepath,filename);
        //send a message to master
        //reply from master
        //send a write (append) request to chunkserver
        //reply from chunkserver
        //if failure return -errno
        return 0;

/*	printf("Inside write. Path is: %s buf is %s",path,buf);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"WRITE\n%s",path);

	char wrtFile[100];
	char *tempBuf;
	tempBuf=(char *)malloc(sizeof(char)*(int)size);
	FILE *fd;
	int recvflag;
	strcpy(wrtFile,".");
	strcat(wrtFile,path);
	strcat(wrtFile,".wrt");
	fd=fopen(wrtFile,"ab+");
	//	printf("cacheFile %s",cacheFile);fflush(stdout);
//tcp code goes here
       
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	
        memset(tcp_buf,0,MAXLEN);
	sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
	memset(tcp_buf,0,MAXLEN);
	recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd (this should be success)\n%s\n",tcp_buf);
	
	if(strcmp(tcp_buf,"success")==0)
	  {

	    memset(tcp_buf,0,MAXLEN);
	    sprintf(tcp_buf,"%d",(int)offset);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    strcpy(tempBuf,"");
	    int nsize,noff;
	    noff=(int)offset/BLOCKSIZE;
	    nsize=(int)size/BLOCKSIZE;
	    recv(sock,tcp_buf,MAXLEN,0);
		printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	    sprintf(tcp_buf,"%d",(int)size);
	    send(sock,tcp_buf,MAXLEN,0);
	    recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	    // sprintf(tcp_buf,"%d",(int)size);
	    send(sock,buf,strlen(buf),0);
	    recv(sock,tcp_buf,MAXLEN,0);
	printf("recvd\n%s\n",tcp_buf);
	    memset(tcp_buf,0,MAXLEN);
	  }
	else
	  {
	    int ret,res;
	  printf("\n%s\n",tcp_buf);
	    send(sock,tcp_buf,strlen(tcp_buf),0);
	    memset(tcp_buf,0,MAXLEN);
	    res=recv(sock,tcp_buf,MAXLEN,0);
	    if(res<0){
	      printf("\nError receiving flags");
	      exit(1);
	    }
	    tcp_buf[res]='\0';
	    ret=atoi(tcp_buf);
	    // send(sock,"next",strlen("next"),0);
	    return ret;

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

int main(int argc, char *argv[])
{
        int i;
	/*initialize the client data structures here*/
        for(i=1;i<argc;i++)
                argv[i] = argv[i+1];
        argc--;
        umask(0);
        return fuse_main(argc, argv, &gfs_oper, NULL);
}

