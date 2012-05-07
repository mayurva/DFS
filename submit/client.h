#ifndef CLIENT_H
#define CLIENT_H
//Common data structures required for client 
#define MASTER_LISTEN 5000
#include<pthread.h>
extern pthread_mutex_t seq_mutex;
extern host master;
extern int master_soc;
#endif
