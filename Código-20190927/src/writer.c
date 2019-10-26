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
	int fd;
	fd=llopen(argv[1],WRITER);
	llwrite(fd,argv[2],strlen(argv[2]));
    return 0;
}

