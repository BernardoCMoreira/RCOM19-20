#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>



#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


//general frame fields

#define FLAG_RCV 0x7E
#define A_RCV 0x03
#define SET 0x03
#define UA 0x07
#define DISC 0x0B

//control frame fields

#define C10 0x00
#define C11 0x40
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81

//control packets
#define C_END 0x03

//byte stuffing
#define ESC 0x7D


