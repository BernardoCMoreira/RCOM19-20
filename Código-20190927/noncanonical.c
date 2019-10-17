/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BAUDRATE B9600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4
#define STOP_S 5


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int state = 0;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0)
  	      (strcmp("/dev/ttyS2", argv[1])!=0))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


    while (STOP==FALSE) {       /* loop for input */

      
      printf("TEST1");
      res = read(fd,buf,255);   /* returns after 5 chars have been input */
      printf("TEST1");



      switch(state){

		case START:
			if(buf[res]==0x7E)
				state=FLAG_RCV;
			else
				state=START;

			break;

		case FLAG_RCV:
			if(buf[res] == 0x03)
				state=A_RCV;
			else if(buf[res] == 0x7E)
				state=FLAG_RCV;
			else
				state=START;
			break;

		case A_RCV:
			if(buf[res] == 0x03)
				state = C_RCV;
			else if(buf[res] == 0x7E)
				state=FLAG_RCV;
			else
				state=START;
			break;

		case C_RCV:
			if(buf[res] == 0x7E)
				state=FLAG_RCV;
			if((buf[res] == 0x03) ^ (buf[res]==0xFE))
				state=BCC_RCV;
			else
				state=START;
			break;

		case BCC_RCV:
			if(buf[res] == 0x7E)
				state=STOP_S;
			else
				state=START;


		}


    printf("State: %d", state);

      if(state == STOP_S){              /* so we can printf... */
        printf("%02hhX:%d\n", buf, res);
        STOP = TRUE;
      }

    }



    printf("Trying to send message confirmation sent.\n");

	   write(fd,buf,strlen(buf));

    printf("Message confirmation sent.\n");

  /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
  */


    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
