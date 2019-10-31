/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "definitions.h"
#include "application.h"
#include "application.c"
#include "writer.h"

int writerVAR = TRUE;
int packetNumber = 0;

int main(int argc, char** argv)
{

	FILE *file;
	struct stat fileInfo;
	char *buffer;
	int *fileSize = (int*)malloc(sizeof(int*));
	char* fileName = argv[2];

	if ((file = fopen(fileName, "rb")) == NULL){
	    perror("error opening file!");
	    exit(-1);
	}

	stat((char *)fileName, &fileInfo);
	
	(*fileSize) = fileInfo.st_size;
	
	buffer = (char *)malloc(*fileSize);

	fread(buffer, sizeof(char), *fileSize, file);

	

	
	//sending start packet

	int fd;
	fd=llopen(argv[1],WRITER);
	int startRes = 0;

	int fileNameLength = strlen(argv[2]);
	char* startPacket = getControlPacket(START_PACKET, *fileSize, fileName, fileNameLength, &startRes);

	llwrite(fd, startPacket, startRes);


	//sending data packets

	
	int packetSize = 100;
	int index = 0;
	
	while(index < *fileSize){


	   char* packet = getPacket(buffer, &index, &packetSize,  *fileSize);
	

	   int finalPacketSize = packetSize;
	   char* finalPacket = addHeader(packet, *fileSize, &finalPacketSize);

	

	   printf("INDEX: %d  PACKETSIZE: %d\n", index, finalPacketSize);
	   llwrite(fd, finalPacket, finalPacketSize);



	}

	printf("\n\nBLABLABLA\n\n");
	
	llclose(fd);
	return 0;
}


char* getControlPacket(char controlField, off_t fileSize, char* fileName, int fileNameLength, int* res){

	
	char* packet = (char *)malloc(*res);

	packet[0] = controlField;
	packet[1] = PACK_SIZE;

	int digitNumber = 0;
	int n = fileSize;

	while(n!=0){
		
	   n/=10;
    	   digitNumber++;

	}

	packet[2]= digitNumber;

	char str[digitNumber];

	sprintf(str, "%ld", fileSize);	

	for(int i=0; i < digitNumber; i++){
	   
	   packet[3+i] = str[i];

	}

	packet[digitNumber + 3] = PACK_NAME;
	packet[digitNumber + 4] = fileNameLength;

	
	for (int j = 0; j < fileNameLength; j++){

	    packet[digitNumber + 5 + j] = fileName[j];

	}


	
	*res = fileNameLength + digitNumber + 5;

	//Confirmação de nome e size
/*	
	char* name = (char*)malloc(sizeof(char)*1);
	name[0] = packet[17]; 

	printf("\n\nname 0: %c \n\n", packet[10]);

	name[1] = packet[10]; 
	name[2] = packet[11]; 
	name[3] = packet[12]; 
	name[4] = packet[13]; 
	name[5] = packet[14]; 
	name[6] = packet[15]; 
	name[6] = packet[16]; 
	name[7] = packet[17]; 
	name[8] = packet[18]; 
	name[9] = packet[19]; 
	name[10] = packet[20]; 
	name[11] = packet[21];
 
	name[12] = packet[22]; 

	char* size = (char*)malloc(sizeof(char)*5);
	size[0] = packet[3]; 
	size[1] = packet[4]; 
	size[2] = packet[5]; 
	size[3] = packet[6]; 
	size[4] = packet[7];



	printf("\n\nName of file: %s\nSize of file: %s\n\n",name, size);	

*/


	return packet;

}


char* getPacket(char* buffer, int* index, int* packetSize, int fileSize){


	int j = *index;


	if(fileSize < *index + *packetSize){

	   *packetSize = fileSize - *index;

	}


	
	char* packet = (char*)malloc(*packetSize *sizeof(char));

	
	for(int i=0; i < *packetSize; i++){

	   packet[i] = buffer[j];
	   j++;

	}


	*index = j;

	return packet;

}




char* addHeader(char* buffer, int fileSize, int* packetSize){



	char* finalPacket = (char*)malloc(fileSize + 4);


	finalPacket[0] = DATA_PACKET;
	finalPacket[1] = packetNumber % 255;
	finalPacket[2] = (*packetSize / 256);
	finalPacket[3] = (*packetSize % 256);

	memcpy(finalPacket + 4, buffer, *packetSize);


	*packetSize += 4;
	packetNumber++;

	return finalPacket;

}












 




