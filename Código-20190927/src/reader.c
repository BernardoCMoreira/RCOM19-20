/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include "application.h"
#include "application.c"
#include "reader.h"

int readerVAR = TRUE;

int main(int argc, char** argv)
{
	int fd;
	fd=llopen(argv[1],READER);

  	int blobSize = 0;
  	char* fileBlob;


	printf("\n\nnew code\n\n");

	int startPacketSize;
	char *startPacket = (unsigned char *)malloc(0);
		

  	startPacketSize = llread(fd, startPacket);

	
	printf("\n\nnew code\n\n");

	int digitNumber = startPacket[2];

  	char str[startPacket[2]];

	for(int i=0; i < digitNumber; i++){
		
		str[i]=startPacket[3+i];

	}

	printf("\n\nSTring de file size: %s\n\n", str);

	blobSize=atoi(str);

	

	printf("\n\nInteiro de file size: %d\n\n", blobSize);

	//nameLength nao esta a ser lido como devia
	int nameLength = 11;
	char *fileName = (char *)malloc(nameLength + 1);


	printf("Name lenght: %c \n",startPacket[2+digitNumber+1]);

	for (int i = 0; i < nameLength; i++){

	  fileName[i] = startPacket[2+digitNumber+2 + i];
	  //printf("FILE NAME[%d]: %c\n", i, fileName[i]);

	}
	
	fileName[nameLength] = '\0';

	printf("File name: %s \n",fileName);
 

	fileBlob = (char *)malloc(blobSize);


	
	char *tmpBuffer = (char *)malloc(sizeof(char*));
	int res = 0;
	int newRes = 4;
	off_t offset = 0;
	


	while (1)
	  {

	    printf("\n\nTENTA LER\n\n");
	    res = llread(fd, tmpBuffer);


	    printf("RES: %d", res);
	    printf("\ntmpBuffer[0]: %c\n", tmpBuffer[0]);
	    
	    for(int i=0; i < res; i++){
		
		printf("tmpBuffer[%d]: %c\n", i, tmpBuffer[i]);

	    }
	    


	    printf("\n\n CONSEGUE LER\n\n");

	    if (reachedEnd(tmpBuffer))
	   	break;
	    
	    printf("\n\n CONSEGUE LER\n\n");
	    
	    tmpBuffer = removeHeader(tmpBuffer, res, &newRes);
	    
	    tmpBuffer = (char *)malloc(newRes);

	    
	    printf("\n\n CONSEGUE LER\n\n");

	    memcpy(fileBlob + offset, tmpBuffer, newRes);

	    offset += newRes;


	  }
	FILE *f = fopen((char *)fileName, "wb+");
	fwrite((void *)fileBlob, 1 , blobSize, f);
	fclose(f);
	printf("CREATED FILE :%s \n",fileName);
	llclose(fd);






    return 0;
}


char *removeHeader(char* buffer, int bufferLength, int* dataLength){


  *dataLength = bufferLength - 4;
  char *noHeader = (char *)malloc(*dataLength);
  
  for (int i = 0; i < *dataLength; i++){
    noHeader[i] = buffer[i+4];
  }

  return noHeader;

}




int reachedEnd(char* packet){
  
  if(packet[0] == END_PACKET)
	return TRUE;
  else
	return FALSE;

	

}



int power(int base, int exp){
	
	int res=1;

	while (exp != 0){
	res *= base;
	--exp;
	}

	return res;

}
