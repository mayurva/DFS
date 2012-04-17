#ifndef GFS_H
#define GFS_H
#define DEBUG
//this file contains common data structures for the overall file system 
#define	MAX_FILENAME	(1024)
#define CHUNK_SIZE 	(1024)
#define MAX_CLIENTS 10

#include<sys/types.h>
#include<sys/socket.h>

typedef struct host_{
	char ip_addr[20];
	int port;
}host;

enum msg_type {

	HEARTBEAT,

	MAKE_DIR_REQ,
	MAKE_DIR_RESP,

	OPEN_REQ,
	OPEN_RESP,

	CREATE_REQ,
	CREATE_RESP,

	GETATTR_REQ,
	GETATTR_RESP,

	READDIR_REQ,
	READDIR_RESP,

	READ_REQ,
	READ_RESP,
	READ_DATA_REQ,
	READ_DATA_RESP,

	WRITE_REQ,
	WRITE_RESP,
	WRITE_DATA_REQ,
	WRITE_DATA_RESP,
};

typedef struct dfs_msg_ {
	int	seq;
	int 	status;
	int	msg_type;
	int	len;
	void	*data;
}dfs_msg;

/*Data structures for message types*/
typedef struct mkdir_req_{
	char	path[MAX_FILENAME];
	mode_t	mode;
}mkdir_req;

typedef struct open_req_{
	char    path[MAX_FILENAME];
	int flags;
}open_req;

/* Read data structures */

typedef struct read_req_ {
	char	filename[MAX_FILENAME];
	int	chunk_index;			
}read_req;

typedef struct read_resp_ {
	char	ip_address[20];
	int 	port;
	char	chunk_handle[64];
}read_resp;

typedef struct read_data_req_ {
	char	chunk_handle[64];
	/* Always do data tranfer in chunks of 2MB
	If start offset and end offset do not fall within chunk boundaries
	Allocate extra 2MB buffers for the first and last chunk 
	and then do a extra copy to user buffer */
}read_data_req;	

typedef struct read_data_resp_ {
	char	chunk[CHUNK_SIZE];
	/* Use recvmsg() and sendmsg() API to separate seq and data - and allow use of user buffer pointer */
}read_data_resp;

/* Write data structures */

typedef struct write_req_ {
	char	filename[MAX_FILENAME];
	int	chunk_index;			
}write_req;

typedef struct write_resp_ {
	/* For now, write full chunk, just validate that the new chunk is the last chunk */
	char	ip_address_primary[64];
	char	ip_address_secondary[64];
	char	chunk_handle[64];
	int 	port_primary;
	int 	port_secondary;
}write_resp;

typedef struct write_data_req_ {
	char	chunk[CHUNK_SIZE];
	char	chunk_handle[64];
	/* For now, write full chunk, just validate that the new chunk is the last chunk */
}write_data_req;	

typedef struct write_data_resp_ {
}write_data_resp;

#endif
