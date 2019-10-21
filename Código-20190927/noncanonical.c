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
int fd;
int state = 0;

void send_ua_message(){
	char buff[255];
	buff[0]= 0x7E;
	buff[1]= 0x03;
	buff[2]= 0x07;
	//buff[3]= buff[1]^buff[2];
	buff[3] = (0x03 ^ 0x07);
	buff[4]= 0x7E;
	
	int res = write(fd,buff,5);
	printf("%d bytes written \n", res);
}

void set_state_machine(char buf[], int res){
 switch(state){
		case START:
			if(buf[0]==0x7E){
				state=FLAG_RCV;
				printf("from start to flag");
			}
			else{
				state=START;
				printf("from start to start");
			}

			break;

		case FLAG_RCV:

			if(buf[0] == 0x03){
				state=A_RCV;
				printf("from Flag to A");
			}
			else if(buf[0] == 0x7E){
				state=FLAG_RCV;
				printf("from flag to flag");
			}
			else{
				state=START;
				printf("from FLag to start");
			}
			break;

		case A_RCV:
			if(buf[0] == 0x03){
				state = C_RCV;
				printf("from A to C");
			}
			else if(buf[0] == 0x7E){
				state=FLAG_RCV;
				printf("from A to flag");
			}
			else{
				state=START;
				printf("from A to start");
			}
			break;

		case C_RCV:
			if(buf[0] == 0x7E){
				state=FLAG_RCV;
				printf("from C to flag");
			}
			if((buf[0] == (0x03^0x03))){
				state=BCC_RCV;
				printf("from C to BCC");
			}
			else{
				state=START;
				printf("from C to start");
			}
			break;

		case BCC_RCV:
			if(buf[0] == 0x7E){
				state=STOP_S;
				printf("from BCC to STOP");
			}
			else{
				state=START;
				printf("from BCC to START");
			}
		}	

      if(state == STOP_S){              /* so we can printf... */
        printf("%02hhX:%d\n", buf, res);
        STOP = TRUE;
	send_ua_message();
      }

    
}
int main(int argc, char** argv)
{
    int c, res;
    struct termios oldtio,newtio;
    char buf[255];
  

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

	int i=-1;
	unsigned char frame[255];
    while (STOP==FALSE) {       /* loop for input */

      
	
      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      i++;
	
	frame[i]=buf[0];
	
      printf(" TEST: %x index: %d\n", buf[0], res);
	
	set_state_machine(buf, res);
     }
      printf("Trying to send message confirmation sent.\n");
	
     //write(fd,frame,5);

    printf("Message confirmation sent.\n");
  /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o
  */


    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
