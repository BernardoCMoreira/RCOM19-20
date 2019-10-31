#pragma once
#include "definitions.h"



char* getControlPacket(char controlField, off_t fileSize, char* fileName, int fileNameLength, int* res);
char* getPacket(char* buffer, int* index, int* packetSize, int fileSize);
char* addHeader(char* buffer, int fileSize, int* packetSize);
