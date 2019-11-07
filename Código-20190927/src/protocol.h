#pragma once
#include "definitions.h"

int svision_state_machine(char byte_received, char controlField, int state);
int info_state_machine(char byte_received, char* buffer, int* res, int* rByte, int state);
void ua_state_machine(char conf[], int state);
int disc_state_machine(char byte_received, int state);
void rr_state_machine(char byte_received, char controlBuf, int state);
void send_disc_message(int fd);
void send_set_message(int fd);
void send_UA_Message(int fd);
void send_rr_message(int r, int fd);
void send_rej_message(int r, int fd);
int set_save_port_settings(int fd, struct termios *oldtio);
int checkBcc2(char *buffer, int res);
char calculateBcc2(char *buffer, int length);
