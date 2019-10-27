#pragma once
#include "definitions.h"

char* getControlPacket(char controlField, off_t fileSize, char* fileName, int fileNameLength, int* res);
char *addHeader(char *buffer, off_t fileSize, int *packetSize);
char *getPacket(char *buffer, off_t *index, int *packetSize, off_t fileSize);
