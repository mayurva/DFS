#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<ifaddrs.h>
#include<net/if.h>
#include<sys/time.h>
#include"gfs.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>

unsigned long long seq_num = 0;
extern pthread_mutex_t seq_mutex;

void print_msg(dfs_msg *msg)
{
		
	switch(msg->msg_type)
	{

	        case HEARTBEAT:
			break;

        	case MAKE_DIR_REQ:
			printf("MAKE_DIR_REQ: path is %s\n",((mkdir_req*)msg->data)->path);
			break;

	        case MAKE_DIR_RESP:
			break;

	        case OPEN_REQ:
			printf("OPEN_REQ: path is %s flags %d\n",((open_req*)msg->data)->path, ((open_req*)msg->data)->flags);
			break;

		case OPEN_RESP:
			break;

	        case CREATE_REQ:
			break;

		case CREATE_RESP:
			break;

		case GETATTR_REQ:
			printf("GETATTR_REQ: path is %s\n",((char*)msg->data));
			break;

		case GETATTR_RESP:
			break;

		case READDIR_REQ:
			break;

		case READDIR_RESP:
			break;

		case READ_REQ:
			break;

		case READ_RESP:
			break;

		case READ_DATA_REQ:
			break;

		case READ_DATA_RESP:
			break;

		case WRITE_REQ:
			break;

		case WRITE_RESP:
			break;

		case WRITE_DATA_REQ:
			break;

		case WRITE_DATA_RESP:
			break;
	}

}

int getRandom(int lower,int upper)
{
	struct timeval tv;
	int seed;
	gettimeofday(&tv,NULL);
	seed = tv.tv_sec+tv.tv_usec;
	srand(seed);
	return ((rand()%(upper-lower))+lower);
}

int populateIp(host *h,char *hostname)
{
	struct hostent *lh = gethostbyname(hostname);
	if(lh){
		strcpy(h->ip_addr,inet_ntoa( *( struct in_addr*)( lh -> h_addr_list[0])));
		return 0;
	}
	return -1;
}

int listenSocket(int soc)
{
	#ifdef DEBUG
		printf("Listen for connection\n");
	#endif

        if(listen(soc,MAX_CLIENTS) == -1) {
                printf("listen error\n");
                return -1;
        }

	#ifdef DEBUG
		printf("Out of Listen\n");
	#endif
	return 0;
}

int acceptConnection(int soc)
{
	int conn_port;
	struct sockaddr_in sock_client;
	int slen = sizeof(sock_client);
	
	if ((conn_port = accept(soc, (struct sockaddr *) &sock_client, &slen)) == -1) {
		return -1;
	}
	
	#ifdef DEBUG
		printf("Connection accepted\n");
	#endif
	
	return conn_port;
}

int createConnection(host h,int conn_socket)
{
        struct sockaddr_in sock_client;
	int slen = sizeof(sock_client);
        int ret;

        memset((char *) &sock_client, 0, sizeof(sock_client));
	
	#ifdef DEBUG
		printf("Connecting to a server\nIP Addr: %s\nport: %d\n",h.ip_addr,h.port);
		printf("Connection socket is %d\n",conn_socket);
	#endif

        sock_client.sin_family = AF_INET;
        sock_client.sin_port = htons(h.port);
	ret = inet_pton(AF_INET, h.ip_addr, (void *) &sock_client.sin_addr);

	ret = connect(conn_socket, (struct sockaddr *) &sock_client, slen);
	if (ret == -1) {
		return -1;
	}
	
	#ifdef DEBUG
		printf("Connected to the server\n");
	#endif
	return 0;
}

int createSocket()
{
	int soc;
	if ((soc= socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}
	#ifdef DEBUG
		printf("Socket created\n");
	#endif
	return soc;
}

int bindSocket(int soc, int listen_port, char ip_addr[])
{
        int optval = 1;
	struct sockaddr_in sock_server;	
	memset((char *) &sock_server, 0, sizeof(sock_server));
	sock_server.sin_family = AF_INET;
	sock_server.sin_port = htons(listen_port);
	sock_server.sin_addr.s_addr = inet_addr(ip_addr);
        setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	while (bind(soc, (struct sockaddr *) &sock_server, sizeof(sock_server)) == -1) {
		listen_port = getRandom(64500,65636);
		sock_server.sin_port = htons(listen_port);
	}

	#ifdef DEBUG
		printf("Player listens on port %d\n",listen_port);
		printf("Listen Socket created\n");
	#endif

	return listen_port;
}


void prepare_msg(int msg_type, struct msghdr **msg, void * data_ptr, int data_len)
{
        dfs_msg *dfsmsg = (dfs_msg*) malloc(sizeof(dfs_msg));
        *msg = (struct msghdr*) malloc(sizeof(struct msghdr));
	struct iovec *iov = (struct iovec*) malloc(sizeof(struct iovec) * 2);
	memset(*msg, 0, sizeof(struct msghdr));
	memset(dfsmsg, 0, sizeof(dfs_msg));
	memset(iov, 0, sizeof(iov));

	pthread_mutex_lock(&seq_mutex);
        dfsmsg->seq = seq_num++;
	pthread_mutex_unlock(&seq_mutex);

        dfsmsg->msg_type = msg_type;
        dfsmsg->len =  data_len;
        dfsmsg->data = data_ptr;

        (*msg)->msg_iov = iov;
        iov[0].iov_base = dfsmsg;
        iov[0].iov_len = sizeof(dfs_msg);
        iov[1].iov_base = data_ptr;
        iov[1].iov_len = data_len;
        (*msg)->msg_iovlen = 2;

}

void free_msg(struct msghdr *msg)
{
        free(((dfs_msg*)msg->msg_iov[0].iov_base)->data);
        free(msg->msg_iov[0].iov_base);
        free(msg->msg_iov);
        free(msg);
}

