#pragma once
/*
*Porta : (Com1, Com2 etc..)
*Flag : Indica se é Emissor ou Recetor
*Retorna o identificador da ligação de dados, ou < 0 em caso de erro
*/
void alarm_handler();
int llopen(char *porta, int flag);
int set_save_port_settings(int fd, struct termios *oldtio);
void send_set_message(void);
void ua_state_machine(char conf[]);
void Send_UA_Message(int fd);
int set_state_machine(char byte_received);
