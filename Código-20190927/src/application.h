#pragma once
/*
*Porta : (Com1, Com2 etc..)
*Flag : Indica se é Emissor ou Recetor
*Retorna o identificador da ligação de dados, ou < 0 em caso de erro
*/

// MACROS TO MAKE CODE CLEANER

//Emissor / Recetor
#define WRITER 1
#define READER 0

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define MODEMDEVICE0 "/dev/ttyS0"
#define MODEMDEVICE1 "/dev/ttyS1"
#define MODEMDEVICE2 "/dev/ttyS2"

#define FALSE 0
#define TRUE 1

//States
#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4
#define STOP_S 5

void alarm_handler();
int llopen(char *porta, int flag);
int set_save_port_settings(int fd, struct termios *oldtio);
void send_set_message(void);
void ua_state_machine(char conf[]);
void Send_UA_Message(int fd);
int set_state_machine(char byte_received);
