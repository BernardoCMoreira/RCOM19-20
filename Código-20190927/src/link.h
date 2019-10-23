#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "definitions.h"

void alarm_handler();
int set_state_machine(char byte_received);
void send_set_message();
void Send_UA_Message(int fd);
void ua_state_machine(char conf[]);
int set_save_port_settings(int fd, struct termios *oldtio);
char calculateBcc2(char *buffer, int length);
