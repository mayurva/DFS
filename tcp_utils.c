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
