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



int llopen(int fd);
int llwrite(int fd, unsigned char *buffer, int size);
void llclose(int fd);
void ua_state_machine( unsigned char *conf, int *state);
void write_ctrl_frame(int fd, unsigned char controlByte);
unsigned char read_ctrl_frame(int fd); 
unsigned char calculateBcc2(unsigned char *buffer, int length);
unsigned char *getControlPacket(unsigned char controlField, off_t fileSize, unsigned char *fileName, int fileNameLength, int *res);
unsigned char *addHeader(unsigned char *mensagem, off_t sizeFile, int *sizePacket);
unsigned char *getPacket(unsigned char *buffer, off_t *index, int *sizePacket, off_t fileSize);


