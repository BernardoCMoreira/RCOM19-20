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
	off_t *fileSize = (off_t*)malloc(sizeof(off_t*));
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


char* getControlPacket(char controlField, off_t fileSize, char* fileName, int fileNameLength, int* res){

	
	*res = fileNameLength + 9 * sizeof(char);
	char* packet = (char *)malloc(*res);

	packet[0] = controlField;
	packet[1] = PACK_SIZE;
	packet[2] = PS_LENGTH;
	packet[3] = (fileSize >> 24) & 0xFF;
	packet[4] = (fileSize >> 16) & 0xFF;
	packet[5] = (fileSize >> 8) & 0xFF;
	packet[6] = fileSize & 0xFF;
	packet[7] = PACK_NAME;
	packet[8] = fileNameLength;

	
	for (int i = 0; i < fileNameLength; i++){

	    packet[i+9] = fileName[i];

	}

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













