INC = chunkserver.h client.h master.h dfs.h
OBJ = chunkserver.o master.o
SRC = chunkserver.c master.c
OUT = chunkserver client master 
CC = cc

FLAGS = `pkg-config fuse --cflags --libs`


all: $(OBJ) client master chunkserver

$(OBJ):$(SRC)

client:
	$(CC) -g $(FLAGS) -o client client.c 

master:
	$(CC) -o master master.o
	
chunkserver:
	$(CC) -o chunkserver chunkserver.o

.PHONY: clean 
clean:
	rm -f $(OUT) $(OBJ) 

