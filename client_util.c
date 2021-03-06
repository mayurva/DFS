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

int create_read_req(read_req *ptr,char *path, int chunk_index)
{
	strcpy(ptr->filename,path);
	ptr->chunk_index = chunk_index;
	return 0;
}

int create_read_data_req(read_data_req *ptr,char *chunk_handle)
{
	strcpy(ptr->chunk_handle,chunk_handle);
	return 0;
}

int create_write_req(write_req *ptr,char *path, int chunk_index)
{
	strcpy(ptr->filename,path);
	ptr->chunk_index = chunk_index;
	return 0;
}

int create_write_data_req(write_data_req *ptr, char *chunk_handle,char *buf)
{
	//strcpy(ptr->chunk_handle,chunk_handle);
	printf("chunk handle %s\n",chunk_handle);
	memcpy(ptr->chunk,buf,CHUNK_SIZE);
	memcpy(&(ptr->chunk[CHUNK_SIZE]),chunk_handle,64);
	return 0;
}
int client_init(int argc,char* argv[])
{	
	pthread_mutex_init(&seq_mutex, NULL);
	strcpy(master.ip_addr,argv[1]);
	master.port = MASTER_LISTEN;
	return 0;
}
