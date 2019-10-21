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
int state =0;

void Send_UA_Message(int fd){

    printf("Trying to send message confirmation.\n");

	char buf[255];
	buf[0] = 0x7E;
	buf[1] = 0x03;
	buf[2] = 0x07;
	buf[3] = buf[1]^buf[2];
	buf[4] = 0x7E;
	
	write(fd,buf,5);

	
    printf("Message confirmation sent.\n");

}

int updateStateMachine(char byte_received){

	 switch(state){

		case START:
			if(byte_received ==0x7E){
				state=FLAG_RCV;
				printf("from start to flag: %d", state);
			}
			else{
				state=START;
				printf("from start to start");
			}

			break;

		case FLAG_RCV:

			if(byte_received== 0x03){
				state=A_RCV;
				printf("from Flag to A");
			}
			else if(byte_received == 0x7E){
				state=FLAG_RCV;
				printf("from flag to flag");
			}
			else{
				state=START;
				printf("from FLag to start");
			}
			break;

		case A_RCV:
			if(byte_received == 0x03){
				state = C_RCV;
				printf("from A to C");
			}
			else if(byte_received == 0x7E){
				state=FLAG_RCV;
				printf("from A to flag");
			}
			else{
				state=START;
				printf("from A to start");
			}
			break;

		case C_RCV:
			if(byte_received == 0x7E){
				state=FLAG_RCV;
				printf("from C to flag");
			}
			if((byte_received == (0x03^0x03))){
				state=BCC_RCV;
				printf("from C to BCC");
			}
			else{
				state=START;
				printf("from C to start");
			}
			break;

		case BCC_RCV:
			if(byte_received== 0x7E){
				state=STOP_S;
				printf("from BCC to STOP");
			}
			else{
				state=START;
				printf("from BCC to START");
			}
	 }
	 
	return state;


}


int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS2", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

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

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


    while (STOP==FALSE) {    
	
		res = read(fd,buf,1);   /* returns after 5 chars have been input */
		
		printf(" TEST: %x index: %d\n", buf[0], res);

		state = updateStateMachine(buf[0]);	

		printf("\nSTATE: %d\n",state);

		if(state == STOP_S){           
			printf("%02hhX:%d\n", buf, res);
			STOP = TRUE;
		}

    }

	Send_UA_Message(fd);

    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
	
}

