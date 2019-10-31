#pragma once
#include "definitions.h"

int control_state_machine(int fd, unsigned char controlByte); 

void write_ctrl_frame(int fd, unsigned char controlByte); 

int checkBcc2(unsigned char *buffer, int res); 

unsigned char *removeHeader(unsigned char *buffer, int bufferLength, int *dataLength); 

int reachedEnd(unsigned char *end); 

void ua_state_machine( unsigned char *conf, int *state);

unsigned char read_ctrl_frame(int fd); 

unsigned char calculateBcc2(unsigned char *buffer, int length);

unsigned char *getControlPacket(unsigned char controlField, off_t fileSize, unsigned char *fileName, int fileNameLength, int *res);

unsigned char *addHeader(unsigned char *mensagem, off_t sizeFile, int *sizePacket);

unsigned char *getPacket(unsigned char *buffer, off_t *index, int *sizePacket, off_t fileSize);