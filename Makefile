INC = chunkserver.h client.h master.h dfs.h
OBJ = chunkserver.o master.o tcp_utils.o
SRC = chunkserver.c master.c tcp_utils.c
OUT = chunkserver client master 
CC = cc

FLAGS = `pkg-config fuse --cflags --libs`


all: $(OBJ) client master chunkserver

$(OBJ):$(SRC)

client:
	$(CC) -g $(FLAGS) -o client client.c tcp_utils.c

master:
	$(CC) -o master master.o tcp_utils.o 
	
chunkserver:
	$(CC) -o chunkserver chunkserver.o tcp_utils.o

.PHONY: clean 
clean:
	rm -f $(OUT) $(OBJ) 

