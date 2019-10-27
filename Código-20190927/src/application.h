#pragma once
#include "definitions.h"

void alarm_handler();
int llopen(char *porta, int flag);
int set_save_port_settings(int fd, struct termios *oldtio);
void send_set_message(void);
void ua_state_machine(char conf[]);
void Send_UA_Message(int fd);
int svision_state_machine(char byte_received, char controlField);
int llwrite(int fd, char * buffer, int length);
int info_state_machine(char byte_received, char* buffer, int* res, int* rByte);
int llclose(int fd);
void send_disc_message(void);
int disc_state_machine(char byte_received);
void ack_disc_message(char conf[255]);
void send_rr_message(int r);
void send_rej_message(int r);
void rr_state_machine(char byte_received, char controlBuf);
