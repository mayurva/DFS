//this file contains common data structures for the overall file system 

#define	MAX_FILENAME	4096
#define CHUNK_SIZE 	(2 * 1024 * 1024)
enum msg_type {

	HEARTBEAT,

	CREATE_REQ,
	CREATE_RESP,

	OPEN_REQ,
	OPEN_RESP,

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

struct dfs_msg {
	int	seq;
	int 	status;
	int	msg_type;
	int	len;
	char	data[0];
};

/* Read data structures */

struct read_req {
	char	filename[MAX_FILENAME];
	int	chunk_index;			
};

struct read_resp {
	char	ip_address[20];
	int 	port;
	char	chunk_handle[64];
};

struct read_data_req {
	char	chunk_handle[64];
	/* Always do data tranfer in chunks of 2MB
	If start offset and end offset do not fall within chunk boundaries
	Allocate extra 2MB buffers for the first and last chunk 
	and then do a extra copy to user buffer */
};	

struct read_data_resp {
	char	chunk[CHUNK_SIZE];
	/* Use recvmsg() and sendmsg() API to separate seq and data - and allow use of user buffer pointer */
}

/* Write data structures */

struct write_req {
	char	filename[MAX_FILENAME];
	int	chunk_index;			
};

struct write_resp {
	/* For now, write full chunk, just validate that the new chunk is the last chunk */
	char	ip_address_primary[20];
	int 	port_primary;
	char	ip_address_secondary[20];
	int 	port_secondary;
	char	chunk_handle[64];
};

struct write_data_req {
	char	chunk_handle[64];
	char	chunk[CHUNK_SIZE];
	/* For now, write full chunk, just validate that the new chunk is the last chunk */
};	

struct write_data_resp {
}



