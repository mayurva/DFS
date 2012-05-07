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
	#ifdef DEBUG
		printf("added chunk handle %s to chunlist of %d \n",ptr->chunk_ptr->chunk_handle,cid);
	#endif
	return 0;
}

int write_block(int cs_id,chunk_info *chunk,char *chunk_data)
{
	int sock = createSocket();
	char *data_ptr = (char*) malloc (MAX_BUF_SZ);

	memset(data_ptr,0,MAX_BUF_SZ);
	sprintf(data_ptr,"%s:%d:%d:",chunk->chunk_handle,0,chunk->chunk_size);
	int len = strlen(data_ptr);
	memcpy(data_ptr+len,chunk_data,chunk->chunk_size);

	#ifdef DEBUG1
		int i;
		printf("Data to be written is - \n");
		for (i = 0; i < CHUNK_SIZE; i++) 
			printf("%c", ((char*)data_ptr+len)[i]);
//		printf("\nhandle - %s\n", data_ptr->chunk+CHUNK_SIZE);
		printf("\n");
	#endif
	//pthread_mutex_lock(&msg_mutex);

		free_msg(msg);
		prepare_msg(WRITE_DATA_REQ, &msg, data_ptr, strlen(data_ptr));
		print_msg(msg->msg_iov[0].iov_base);

		/* Send write-data request to secondary chunkserver */
		host h;
		strcpy(h.ip_addr,chunk_servers[cs_id].ip_addr);
		h.port = chunk_servers[cs_id].client_port;
		if(createConnection(h,sock) == -1){
			printf("Connect failed\n");
			return -1;
		}
		if((sendmsg(sock,msg,0))==-1){
        	       	printf("%s: message sending failed\n",__func__);
			free_msg(msg);
			free(data_ptr);
			return -1;
	        } else {
			printf("%s: Sent write request to new chunkserver %d\n",__func__,cs_id);
		}
		
	        /* Reply from secondary chunkserver */
        	if((recvmsg(sock,msg,0))==-1){
               		printf("%s: message receipt failed\n",__func__);
			free_msg(msg);
			free(data_ptr);
			return -1;
	        } else {
			printf("%s: Received write reply from secondary chunkserver\n",__func__,cs_id);
		}
		close(sock);
	//pthread_mutex_unlock(&msg_mutex);
}

int read_block(int cs_id,chunk_info * chunk,char **chunk_data)
{
//	printf("aat \n");
	int size_read=0;
	char *buf;// = (char*)malloc((CHUNK_SIZE+100)*sizeof(char));
	dfs_msg *dfsmsg;
	char *data_ptr = (char*)malloc(MAX_BUF_SZ);
	//*chunk_data = malloc((CHUNK_SIZE)*sizeof(char));
//	free_msg(msg);
	sprintf(data_ptr,"%s:%d:%d:",chunk->chunk_handle,0,chunk->chunk_size);
	#ifdef DEBUG
//		print_msg(msg->msg_iov[0].iov_base);
		printf("beginning of read %s\n",chunk->chunk_handle);
	#endif
	prepare_msg(READ_DATA_REQ, &msg, data_ptr, strlen(data_ptr));
	print_msg(msg->msg_iov[0].iov_base);

	int sock = createSocket();
//	pthread_mutex_lock(&msg_mutex);	
		/* Send read-data request to chunkserver */
		host h;
		strcpy(h.ip_addr,chunk_servers[cs_id].ip_addr);
		h.port = chunk_servers[cs_id].client_port;
		if(createConnection(h,sock) == -1){
			printf("Connect failed\n");
			return -1;
		}

		if((sendmsg(sock,msg,0))==-1){
               		printf("%s: read request sending to chunkserver failed\n",__func__);
			return -1;
        	} else {
               		printf("%s: Success: Sent read request\n",__func__);
		}

		/* Receive read-data reply from chunkserver */
		free_msg(msg);
		char *resp = (char*) malloc(MAX_BUF_SZ);
		prepare_msg(READ_DATA_RESP, &msg, resp, sizeof(read_data_resp));
		
        	if((recvmsg(sock,msg,0))==-1){
               		printf("%s: read reply from chunkserver failed\n",__func__);
			return -1;
        	} else {
       			printf("%s: Success: Received read reply from chunkserver\n",__func__);
		}
//	pthread_mutex_unlock(&msg_mutex);
		/*TODO: process received data */
	       	dfsmsg =  msg->msg_iov[0].iov_base;
		resp = msg->msg_iov[1].iov_base;
		if(dfsmsg->status == 0) {
		/* Update number of bytes read */
			int size = atoi(strtok_r(resp,":",&buf));
			size_read += size;
			#ifdef DEBUG1
				int i;
				printf("Data read is - \n");
				for (i = 0; i < resp->size; i++) 
					printf("%c", buf[i]);
				printf("\n");
			#endif
			*chunk_data = buf;
			return size_read;
		}
	close(sock);
	return -1;
}

void re_replicate(int index)
{
	printf("inside re replicate\n");
	int new_cs;
	char *chunk_data;//=(char*)malloc(CHUNK_SIZE*sizeof(char));
	chunklist_node *ptr = chunk_servers[index].head;	
	if(ptr==NULL){
		printf("chunk list is empty for chunkserver %d\n",index);
		return;
	}
	int count = 0;
	while(ptr){
		count++;
		ptr = ptr -> next;
	}
	printf("%d chunks in the list",count);
	ptr = chunk_servers[index].head;
	while(1){
		printf("inside while\n");
		chunklist_node *ptr1 = (chunklist_node*)malloc(sizeof(chunklist_node));
		ptr1->chunk_ptr = ptr->chunk_ptr;
		ptr1->other_cs = ptr->other_cs;
		int diff = index && ptr->other_cs;
		printf("diff is %d\n",diff);
		new_cs = failover_array[index + ptr->other_cs - !diff][failover_array[index + ptr->other_cs - !diff][0]+1];
		failover_array[index + ptr->other_cs - !diff][0] = !failover_array[index + ptr->other_cs - !diff][0];	
		ptr->moved_cs = new_cs;
		ptr1->moved_cs = -1;
	
		printf("re-replicating chunk %s from failed chunkserver %d.. copying from chunkserver %d to %d\n",ptr1->chunk_ptr->chunk_handle,index,ptr->other_cs,new_cs);		
		if((ptr1->chunk_ptr)->chunkserver_id[0] == index)	ptr1->chunk_ptr->chunkserver_id[0] = new_cs;
		else	ptr1->chunk_ptr->chunkserver_id[1] = new_cs;

		printf("before reading\n");
		int bytes_read = read_block(ptr->other_cs,ptr1->chunk_ptr,&chunk_data);
		if (bytes_read != ptr1->chunk_ptr->chunk_size) {
			printf("\n Error reading chunk data from failed chunkserver");
			return;
		}
		#ifdef DEBUG1
			int i;
			printf("Data read is - \n");
			for (i = 0; i < bytes_read; i++)
				printf("%c", chunk_data[i]);
			printf("\n");
		#endif
		write_block(new_cs,ptr1->chunk_ptr,chunk_data);
		printf("written\n");
		ptr = ptr -> next;
		if(!ptr)
			break;
		printf("next object\n");
	}
	printf("finished re-replicating\n");
}
