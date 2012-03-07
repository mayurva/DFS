//This file contains main code for the client
#define FUSE_USE_VERSION 26
#include"gfs.h"
#include"client.h"
#include<fuse.h>
#include<stdio.h>

static int gfs_getattr(const char *path, struct stat *stbuf)
{
}

static int gfs_mkdir(const char *path, mode_t mode)
{
/*        int res;
	//client side code goes here
        printf("Inside mkdir Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"MKDIR\n%s\n",path);

	//tcp code goes here
        send(sock,tcp_buf,strlen(tcp_buf),0);
        recv(sock,tcp_buf,MAXLEN,0);

        send(sock,(char*)&mode,sizeof(mode_t),0);

        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);
        printf("Received message: %s\n",tcp_buf);

        a = strtok(tcp_buf,"\n");
        a = strtok(NULL,"\n");
        res = atoi(a);
        printf("End of mkdir\n");
        return res;*/
}

static int gfs_open(const char *path, struct fuse_file_info *fi)
{
/*  int ret=0;
  printf("Inside open Path is: %s\n",path);
        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"OPEN\n%s",path);

//tcp code goes here

        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        recv(sock,tcp_buf,MAXLEN,0);

        memset(tcp_buf,0,MAXLEN);
        sprintf(tcp_buf,"%d",fi->flags);
        send(sock,tcp_buf,strlen(tcp_buf),0);
        memset(tcp_buf,0,MAXLEN);
        int res=recv(sock,tcp_buf,MAXLEN,0);
        if(res<0){
          printf("\nError receiving flags");
          exit(1);
        }
        tcp_buf[res]='\0';

        if(strcmp(tcp_buf,"failed")==0)
          {
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
          }

        //      if(strcmp(tcp_buf,"success")!=0)
        printf("\n%s\n",tcp_buf);
        printf("End of open\n");
        return ret;*/
}

static int gfs_read(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{
}

static int gfs_write(const char *path, const char *buf, size_t size,off_t offset, struct fuse_file_info *fi)
{
}

static int gfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi)
{

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

