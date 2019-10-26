#pragma once
#include "definitions.h"

void alarm_handler();
int llopen(char *porta, int flag);
int set_save_port_settings(int fd, struct termios *oldtio);
int llwrite(int fd, char * buffer, int length);
int info_state_machine(char byte_received, char* buffer, int* res);
int llclose(int fd);
void ack_disc_message(char conf[255]);
int svision_state_machine(char byte_received,char controlField);
void send_svision_message(char controlField);
