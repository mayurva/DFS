INC = chunkserver.h client.h master.h dfs.h
OBJ = chunkserver.o client.o master.o
SRC = chunkserver.c client.c master.c
OUT = chunkserver client master 
CC = cc

FLAGS = #-g pkg-config fuse --cflags --libs'


all: $(OBJ) client master chunkserver

$(OBJ):$(SRC)

client:
	$(CC) -o client client.o 

master:
	$(CC) -o master master.o
	
chunkserver:
	$(CC) -o chunkserver chunkserver.o

.PHONY: clean 
clean:
	rm -f $(OUT) $(OBJ)

