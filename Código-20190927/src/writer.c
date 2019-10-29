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
#include <math.h>
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
	
	(*fileSize) = fileInfo.st_size; //segmentation fault
	
	buffer = (char *)malloc(*fileSize);

	fread(buffer, sizeof(char), *fileSize, file);
	

	int fd;
	fd=llopen(argv[1],WRITER);
	int startRes = 0;

	int fileNameLength = strlen(argv[2]);
	char* startPacket = getControlPacket(START_PACKET, *fileSize, fileName, fileNameLength, &startRes);

	printf("GOT HERE 1\n");

	llwrite(fd, startPacket, startRes);


	printf("Start Packet sent\n");

	int packetSize = 100;
	off_t index = 0;

	while (index < *fileSize){
		
		char *packet = getPacket(buffer, &index, &packetSize, *fileSize);
		printf("Data packet sent\n");

		int finalPacketSize = packetSize;
		char *finalPacket = addHeader(packet, *fileSize, &finalPacketSize);

		llwrite(fd, finalPacket, finalPacketSize);
			
	}


	int endRes = 0;
	char *endPacket = getControlPacket(END_PACKET, *fileSize, fileName, fileNameLength, &endRes);
  	llwrite(fd, endPacket, endRes);
	printf("End Packet sent\n");

	llclose(fd);
	return 0;
}


char* getControlPacket(char controlField, int fileSize, char* fileName, int fileNameLength, int* res){

	
	*res = fileNameLength + 9 * sizeof(char);
	char* packet = (char *)malloc(*res);

	packet[0] = controlField;
	packet[1] = PACK_SIZE;


	int digitNumber=0;
	int n = fileSize;

	while(n != 0){
		n /= 10;
		++digitNumber;
	}


	packet[2] = digitNumber;

	
  	char str[digitNumber];

  	sprintf(str, "%d", fileSize);

	
	for(int i=0; i < digitNumber; i++){
		
		packet[3+i]=str[i];

	}


	packet[2+digitNumber+1] = PACK_NAME;
	packet[2+digitNumber+2] = fileNameLength;

	
	
	for (int j = 0; j < fileNameLength; j++){

	    packet[2+digitNumber+2+j] = fileName[j];

	}

	printf("\n\n\n Length of file name: %c \nSize of File packets: %s\n packet 1: %c\n packet 2: %c\n packet 3: %c\n packet 4: %c\n packet 5: %c\n PACK_NAME: %x\n\n",packet[2+digitNumber+2],str,packet[3],packet[4],packet[5],packet[6],packet[7],packet[8]);



	return packet;

}



char *getPacket(char *buffer, off_t *index, int *packetSize, off_t fileSize){
	
	char *packet;
	off_t j = *index;

	if (fileSize < *index + *packetSize){
	  *packetSize = fileSize - *index;
	}

	packet = (char *)malloc(*packetSize);

	for (int i = 0; i < *packetSize; i++){
		
		packet[i] = buffer[j];
		j++;
	
	}

	*index = j;
	
	//indexfinal =indexinicial + i  em vez do =j

	return packet;

}


char *addHeader(char *buffer, off_t fileSize, int *packetSize){
	
	char *finalPacket = (char *)malloc(fileSize + 4);

	finalPacket[0] = DATA_PACKET;
	finalPacket[1] = packetNumber % 255;
	finalPacket[2] = (int)fileSize / 256;
	finalPacket[3] = (int)fileSize % 256;

	memcpy(finalPacket + 4, buffer, *packetSize);

	*packetSize += 4;
	packetNumber++;

	return finalPacket;

}













