#include"gfs.h"
#include"client.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

int create_mkdir_req(mkdir_req *ptr, char *path, mode_t mode)
{
	strcpy(ptr->path,path);
	ptr->mode = mode;
	return 0;
}

int create_open_req(open_req *ptr, char *path, int flags)
{
	strcpy(ptr->path,path);
	ptr->flags = flags;
	printf(" create flags - %d\n", flags);
	return 0;
}

int create_read_req(char *ptr,char *path, int chunk_index, int offset, int size)
{
	sprintf(ptr,"%s:%d:%d:%d:",path,chunk_index,offset,size);
	return 0;
}

int create_read_data_req(char *ptr,char *chunk_handle, int offset, int size)
{
	sprintf(ptr,"%s:%d:%d:",chunk_handle,offset,size);
	return 0;
}

int create_write_req(char *ptr,char *path, int chunk_index, int offset, int size)
{
	sprintf(ptr,"%s:%d:%d:%d:",path,chunk_index,offset,size);
	return 0;
}

int create_write_data_req(char *ptr, char *chunk_handle, char *buf, int offset, int size)
{
	int len;
	printf("chunk handle %s\n",chunk_handle);
	memset(ptr,0,MAX_BUF_SZ);
	sprintf(ptr,"%s:%d:%d:",chunk_handle,offset,size);
	len = strlen(ptr);
	memcpy(ptr+len,buf,size);
	return 0;
}

int client_init(int argc,char* argv[])
{	
	pthread_mutex_init(&seq_mutex, NULL);
	strcpy(master.ip_addr,argv[1]);
	master.port = MASTER_LISTEN;
	return 0;
}
