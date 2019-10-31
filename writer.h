#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "definitions.h"
#include "linklayer.c"
#include "linklayer.h"


int llopen(int fd);
int llwrite(int fd, unsigned char *buffer, int size);
void llclose(int fd);



