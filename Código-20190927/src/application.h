#pragma once
#include "definitions.h"

void alarm_handler();
int llopen(char *porta, int flag);
int set_save_port_settings(int fd, struct termios *oldtio);
void send_set_message(void);
void ua_state_machine(char conf[]);
void Send_UA_Message(int fd);
int set_state_machine(char byte_received);
int llwrite(int fd, char * buffer, int length);
int info_state_machine(char byte_received, char* buffer, int* res);
int llclose(int fd);
void send_disc_message(void);
int disc_state_machine(char byte_received);
void ack_disc_message(char conf[255]);
