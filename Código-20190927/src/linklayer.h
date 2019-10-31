#pragma once
#include "definitions.h"

int control_state_machine(int fd, unsigned char controlByte); 

void write_ctrl_frame(int fd, unsigned char controlByte); 

int checkBcc2(unsigned char *buffer, int res); 

unsigned char *removeHeader(unsigned char *buffer, int bufferLength, int *dataLength); 

int reachedEnd(unsigned char *end); 