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
#define ESC_S 6
#define DATA_S 7


//control fields
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81

//packet fields
#define PACK_SIZE 0x00
#define PACK_NAME 0x01
#define PS_LENGTH 0x04
#define START_PACKET 0x02
#define END_PACKET 0x03
#define DATA_PACKET 0x01




#define FLAG 0x7E
#define ESC 0x7D
#define ESC_OR 0x20




















