#pragma once
#include "definitions.h"

void alarm_handler();
int llopen(char *porta, int flag);
int llwrite(int fd, char * buffer, int length);
int llclose(int fd);
void ack_disc_message(char conf[255]);

