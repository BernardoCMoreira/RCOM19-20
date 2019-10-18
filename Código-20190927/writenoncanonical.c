/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyS0"
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

int flag=1, conta=1;

void send_set_message(int fd){

	char buf[255];
	buf[0] = 0x7E;

        buf[1] = 0x03;

        buf[2] = 0x03;

        buf[3] = buf[1]^buf[2];

        buf[4] = 0x7E;

        int res = write(fd,buf,5);

        printf("%d bytes written\n", res);
}

void alarm_handler()                  			 // atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
	
}


int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    //char buf[255];
    int state=0;
    int i, sum = 0, speed = 0;


    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          	(strcmp("/dev/ttyS2", argv[1])!=0) )) {
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

    send_set_message(fd);
    //Start timeout verification	
    (void) signal(SIGALRM,  alarm_handler);    
	

    char conf[255];
   
    while (STOP==FALSE || conta<4) {       /* loop for input */
	  if(flag){
      		alarm(3);                 // activa alarme de 3s
      		flag=0;
		send_set_message(fd);
   	}
	
	printf("1\n");
        res = read(fd,conf,1);   /* returns after 5 chars have been input */
	printf("2\n");

	printf(" TEST: %x index: %d\n", conf[0], res);

      //verify UA message
      switch(state){
	
		case START:
			if(conf[0]==0x7E){
				state=FLAG_RCV;
				printf("from start to flag");
			}
			else{
				state=START;
				printf("from start to start");
			}

			break;

		case FLAG_RCV:

			if(conf[0] == 0x03){
				state=A_RCV;
				printf("from Flag to A");
			}
			else if(conf[0] == 0x7E){
				state=FLAG_RCV;
				printf("from flag to flag");
			}
			else{
				state=START;
				printf("from FLag to start");
			}
			break;

		case A_RCV:
			if(conf[0] == 0x07){
				state = C_RCV;
				printf("from A to C");
			}
			else if(conf[0] == 0x7E){
				state=FLAG_RCV;
				printf("from A to flag");
			}
			else{
				state=START;
				printf("from A to start");
			}
			break;

		case C_RCV:
			if(conf[0] == 0x7E){
				state=FLAG_RCV;
				printf("from C to flag");
			}
			if((conf[0] == (0x03^0x07))){
				state=BCC_RCV;
				printf("from C to BCC");
			}
			else{
				state=START;
				printf("from C to start");
			}
			break;

		case BCC_RCV:
			if(conf[0] == 0x7E){
				state=STOP_S;
				printf("from BCC to STOP");
			}
			else{
				state=START;
				printf("from BCC to START");
			}

		}	

      if(state == STOP_S){              /* so we can printf... */
        printf("\nConfirmation message was obtained succesfully\n");
        STOP = TRUE;
	alarm(0);
	
      }	
  }


  /*
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
    o indicado no gui�o
  */

    sleep(1);
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }



    close(fd);
    return 0;

}

