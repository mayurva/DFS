#include"gfs.h"
#include"master.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
struct msghdr *msg;

int failover_array[6][3]={	{0,2,3},	//0,1
				{0,1,3},	//0,2
				{0,1,2},	//0,3
				{0,0,3},	//1,2
				{0,0,2},	//1,3
				{0,0,1}		//2,3
			};

int add_tochunklist(chunk_info *chunk_ptr,int index)
{
	int cid = chunk_ptr->chunkserver_id[index];
	chunklist_node *ptr = (chunklist_node*)malloc(sizeof(chunklist_node));
	if(ptr == NULL){
		return -1;
	}
	ptr->chunk_ptr = chunk_ptr;	
	ptr->other_cs = chunk_ptr->chunkserver_id[!index];
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

int write_block(int cs_id,char *chunk_handle,char *chunk_data)
{
	write_data_req *data_ptr = (write_data_req*) malloc (sizeof(write_data_req));
        memcpy(data_ptr->chunk,chunk_data,CHUNK_SIZE);
        memcpy(&(data_ptr->chunk[CHUNK_SIZE]),chunk_handle,64);
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
	if((sendmsg(chunk_servers[cs_id].conn_socket,msg,0))==-1){
               	printf("%s: message sending failed\n",__func__);
		free_msg(msg);
		free(data_ptr);
		return -1;
        } else {
		printf("%s: Sent write request to secondary chunkserver\n",__func__);
	}
		
        /* Reply from secondary chunkserver */
        if((recvmsg(chunk_servers[cs_id].conn_socket,msg,0))==-1){
               	printf("%s: message receipt failed\n",__func__);
		free_msg(msg);
		free(data_ptr);
		return -1;
        } else {
		printf("%s: Received write reply from secondary chunkserver\n",__func__);
	}
}

int read_block(int cs_id,char *chunk_handle,char chunk_data[])
{
	int size_read=0;
	char buf[CHUNK_SIZE+100];
	dfs_msg *dfsmsg;
	read_data_req *data_ptr;
	free_msg(msg);
	strcpy(data_ptr->chunk_handle,chunk_handle);

	prepare_msg(READ_DATA_REQ, &msg, &data_ptr, sizeof(read_data_req));
	print_msg(msg->msg_iov[0].iov_base);

	/* Send read-data request to chunkserver */
	if((sendmsg(chunk_servers[cs_id].conn_socket,msg,0))==-1){
               	printf("%s: read request sending to chunkserver failed\n",__func__);
		return -1;
        } else {
               	printf("%s: Success: Sent read request\n",__func__);
	}

        /* Receive read-data reply from chunkserver */
	free_msg(msg);
	read_data_resp *resp = (read_data_resp*) malloc(sizeof(read_data_resp));
	prepare_msg(READ_DATA_RESP, &msg, resp, sizeof(read_data_resp));
        if((recvmsg(chunk_servers[cs_id].conn_socket,msg,0))==-1){
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
		return size_read;
	}
	chunk_data = buf;
	return -1;
}

void re_replicate(int index)
{
	int new_cs;
	char chunk_data[CHUNK_SIZE];
	chunklist_node *ptr = chunk_servers[index].head;	
	if(ptr==NULL)	return;
	while(ptr->next){
		chunklist_node *ptr1 = (chunklist_node*)malloc(sizeof(chunklist_node));
		ptr1->chunk_ptr = ptr->chunk_ptr;
		ptr1->other_cs = ptr->other_cs;
		new_cs = failover_array[index + ptr->other_cs + !index][failover_array[index + ptr->other_cs + !index][0]+1];
		failover_array[index + ptr->other_cs + !index][0] = !failover_array[index + ptr->other_cs + !index][0];	
		ptr->moved_cs = new_cs;
		ptr1->moved_cs = -1;

		if((ptr1->chunk_ptr)->chunkserver_id[0] == index)	ptr1->chunk_ptr->chunkserver_id[0] = new_cs;
		else	ptr1->chunk_ptr->chunkserver_id[1] = new_cs;

		read_block(ptr->other_cs,ptr1->chunk_ptr->chunk_handle,chunk_data);
		#ifdef DEBUG
			int i;
			printf("Data read is - \n");
			for (i = 0; i < CHUNK_SIZE; i++)
				printf("%c", chunk_data[i]);
			printf("\n");
		#endif
		write_block(new_cs,ptr1->chunk_ptr->chunk_handle,chunk_data);
	}
}
