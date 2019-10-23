#pragma once
#include "link.h"

int llopen(char *porta, int flag);
int llwrite(int fd, char *buffer, int length);
