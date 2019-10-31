#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "definitions.h"
#include "linklayer.h"
#include "linklayer.c"


void llopen(int fd);

unsigned char *llread(int fd, int *bufferSize);

void llclose(int fd);


