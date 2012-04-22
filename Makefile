INC = chunkserver.h client.h master.h dfs.h
OBJ = chunkserver.o master.o tcp_utils.o replication.o
SRC = chunkserver.c master.c tcp_utils.c replication.c
OUT = chunkserver client master 
CC = cc -g

FLAGS = `pkg-config fuse --cflags --libs`


all: $(OBJ) client master chunkserver

$(OBJ):$(SRC)

client:
	$(CC)  $(FLAGS) -o client client.c tcp_utils.c client_util.c -g

master:
	$(CC) -o master master.o tcp_utils.o replication.o -lpthread -g
	
chunkserver:
	$(CC) -o chunkserver chunkserver.o tcp_utils.o -lpthread -g

.PHONY: clean 
clean:
	rm -f $(OUT) $(OBJ) 

