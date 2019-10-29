#pragma once
#include "definitions.h"

int power(int base, int exp);
char *removeHeader(char* buffer, int bufferLength, int* dataLength);
int reachedEnd(char* packet);
