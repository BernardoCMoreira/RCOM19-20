#include "definitions.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

void llopen(int fd);

unsigned char *llread(int fd, int *bufferSize);

void llclose(int fd); 

int control_state_machine(int fd, unsigned char controlByte);

void write_ctrl_frame(int fd, unsigned char controlByte);

int checkBcc2(unsigned char *buffer, int res);

unsigned char *removeHeader(unsigned char *buffer, int bufferLength, int *dataLength);

int reachedEnd(unsigned char *end);

int randomnessGenerator(int errorProbability);

unsigned char simulateErrorInHeader(int errorProbability);

int checkBcc1(unsigned char byte, unsigned char controlByte);
