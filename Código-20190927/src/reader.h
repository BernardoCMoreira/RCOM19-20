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


void llopen(int fd);

unsigned char *llread(int fd, int *bufferSize);

void llclose(int fd);//

int control_state_machine(int fd, unsigned char controlByte); 

void write_ctrl_frame(int fd, unsigned char controlByte); 

int checkBcc2(unsigned char *buffer, int res); 

unsigned char *removeHeader(unsigned char *buffer, int bufferLength, int *dataLength); 

int reachedEnd(unsigned char *end); 

