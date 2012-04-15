#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAX_LENGTH 100
#define CHUNK_SIZE 1024

int read_file(char *filename)
{
	FILE *fd;
	char *ptr = (char*)malloc(CHUNK_SIZE+1);
	int n;
	if((fd = fopen(filename,"r+"))==NULL){
		printf("Error opening file\n");
		return -1;
	}
	printf("Enter which block to be read: ");
	scanf("%d",&n);
	//Read the nth block if exists throw error if not...	
	fseek(fd,(n-1)*CHUNK_SIZE,SEEK_SET);
	if(fread(ptr,CHUNK_SIZE,1,fd) == 0){
		printf("Read failure\n");
		fclose(fd);
		return -1;
	}
	printf("Data is\n%s\n",ptr);
	fclose(fd);
	return 0;
} 

int write_file(char *filename)
{
	int n,i;
	char *ptr = (char*)malloc(CHUNK_SIZE+1);
	FILE *fd;
	if((fd = fopen(filename,"a+"))==NULL){
		printf("Error opening file\n");
		return -1;
	}
	printf("Enter number of blocks to be written: ");
	scanf("%d",&n);

	//write n blocks of random data each of size CHUNK_SIZE
	for(i=0;i<n;i++){
		memset(ptr,'a'+i,CHUNK_SIZE);
		ptr[CHUNK_SIZE] = '\0';
		if(fwrite(ptr,CHUNK_SIZE,1,fd) == 0){
			printf("Write failure\n");
			fclose(fd);
			return -1;
		}
	}	
	printf("Write successful\n");
	fclose(fd);
	return 0;
}

int main()
{
	FILE *fd;
	char filename[MAX_LENGTH];
	int ch;
	while(1){
		printf("Choose and option\n");
		printf("0 : Exit\n");
		printf("1 : Create\n");
		printf("2 : Open for reading\n");
		printf("3 : Open for writing\n");
		printf("Enter a choice: ");
		scanf("%d",&ch);
		if(ch>0 && ch <4){
			printf("Enter filename: ");
			scanf("%s",filename);
		}
		switch(ch){
			case 0:
				exit(0);
			case 1:
				if((fd = fopen(filename,"w+")) == NULL){
					printf("Error creating file\n");
				}
				else{
					fclose(fd);
				}
				break;
			case 2:
				read_file(filename);
				break;
			case 3:
				write_file(filename);
				break;
			default:
				printf("Incorrect choice\n");
		}
	}
	return 0;
}
