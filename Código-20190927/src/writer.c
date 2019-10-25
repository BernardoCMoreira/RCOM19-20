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
#include "application.h"
#include "application.c"

int writerVAR = TRUE;


int main(int argc, char** argv)
{

	FILE *file = NULL;
	long file_size=0;
	int size_fileName=0;
	long pack_size = 0, auxiliar_size = 0;
	int counter = 0;
	unsigned char* vec, data, start, end; // data- 0x01, start 0x02, end 0x03

	llopen(argv[1],WRITER);
	//int fd = open(argv[1], O_RDWR | O_NOCCTY);

	if(fd<0){
		printf("Error opening port");
		return -1
	}
	(void)signal(SIGALARM, alarm_handler); //alarme instalado!


	file = fopen(argv[2],"r"); //use "rb" for non text-files


	//size of file name;
	size_fileName = strlen([argv[2]]);
	//criar o nome do ficheiro.
	unsigned char *file_name = (unsigned char*)malloc(size_fileName);
	file_name = (unsigned char*)argv[2];

	//size of file;
	fseek(file,0,SEEK_END);
	file_size = ftell(file);

	auxiliar_size = file_size;

	while(auxiliar_size >=256){
		auxiliar_size /= 256;
		counter++;
	}
	auxiliar_size = file_size;

	for(int j=i , j>0; j++){
		vec[j-1] = (unsigned char)(auxiliar_size%256);
		auxiliar_size /= 256;
	}

	//start packet
	start[0] = 0x02;
	start[1] = 


    return 0;
}

