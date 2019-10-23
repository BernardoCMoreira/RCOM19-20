#pragma once
#include "definitions.h"

void alarm_handler();
int llopen(char *porta, int flag);
int set_save_port_settings(int fd, struct termios *oldtio);
void send_set_message(void);
void ua_state_machine(char conf[]);
void Send_UA_Message(int fd);
int set_state_machine(char byte_received);
