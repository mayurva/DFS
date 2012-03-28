#ifndef CHUNKSERVER_H
#define CHUNKSERVER_H

#define MASTER_LISTEN 6000
//Common data structures required for fileserver
void* sendHeartbeat(void*);
void* listenClient(void*);
void* listenMaster(void*);
#endif
