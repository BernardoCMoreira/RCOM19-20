#pragma once
// MACROS TO MAKE CODE CLEANER

//Emissor / Recetor
#define EMISSOR 1
#define RECETOR 0

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
