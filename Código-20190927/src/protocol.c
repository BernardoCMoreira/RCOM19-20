#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "protocol.h"



int svision_state_machine(char byte_received, char controlField, int state)
{

    switch (state)
    {

    case START:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }

        break;

    case FLAG_RCV:

        if (byte_received == 0x03)
        {
            state = A_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case A_RCV:
        if (byte_received == controlField)
        {
            state = C_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case C_RCV:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        if (byte_received == (0x03 ^ 0x03))
        {
            state = BCC_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case BCC_RCV:
        if (byte_received == 0x7E)
        {
            state = STOP_S;
        }
        else
        {
            state = START;
        }
    }

    return state;
}

int info_state_machine(char byte_received, char* buffer, int* res, int* rByte, int state)
{

    unsigned char controlByte;
    if(byte_received != START || state == A_RCV){
	printf("%x\n",byte_received);}
	
    switch (state)
    {

    case START:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START; 
        }
        break;
	
    case FLAG_RCV:
        if (byte_received == 0x03)
        {
            state = A_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV; 
        }
	else
	{
	    state = START;
	}
        break;
    
    case A_RCV:
        if (byte_received == 0x00)
        {
            //quando recebe o control com s = 0
	    controlByte = byte_received;
	    *rByte = 0;
	    state = C_RCV;
        }
        else if (byte_received == 0x40)
        {
            //quando recebe o control com s = 1 
	    controlByte = byte_received;
	    *rByte = 2;
	    state = C_RCV;
        }
	else if (byte_received==0x7E)
	{
	    state = FLAG_RCV;
	}
	else
	{
	    state = START;
	}
        break;

    case C_RCV:
        if (byte_received == (0x03 ^ controlByte))
        {
            state = BCC_RCV;
        }
	else
	{
	    state = START;
	}
        break;

    case BCC_RCV:
	if (byte_received == ESC){
	    state = ESC_S;
	    printf("chavetao\n");
	}

	else if (byte_received == FLAG){
	    state = STOP_S;
	}
	

	else{
            buffer = (char *)realloc(buffer, ++(*res));

            buffer[*res - 1] = byte_received;
	    printf("MESSAGE IS: %s AND last byte is: %x \n", buffer, byte_received);
	}
        break;

    case ESC_S: 
	if(byte_received == 0x5E){
		buffer = (char*)realloc(buffer, ++(*res));
		printf("Imprimindo um } : %x\n", byte_received);
		buffer[*res-1] = 0x7E;
	}
	
	else if(byte_received == 0x5D){
		buffer = (char*)realloc(buffer, ++(*res));
		printf("Imprimindo um }\n");
		buffer[*res-1] = 0x7D;
	}
	else{

		printf("Found invalid character after the escape character");
	}
	state = BCC_RCV;
	break;
	
    
  

    }
    return state;  	
		 
}


void ua_state_machine(char conf[], int state)
{
    switch (state)
    {

    case START:
        if (conf[0] == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }

        break;

    case FLAG_RCV:

        if (conf[0] == 0x03)
        {
            state = A_RCV;
        }
        else if (conf[0] == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case A_RCV:
        if (conf[0] == 0x07)
        {
            state = C_RCV;
        }
        else if (conf[0] == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case C_RCV:
        if (conf[0] == 0x7E)
        {
            state = FLAG_RCV;
        }
        if (conf[0] == (0x03 ^ 0x07))
        {
            state = BCC_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case BCC_RCV:
        if (conf[0] == 0x7E)
        {
            state = STOP_S;
        }
        else
        {
            state = START;
        }
    }

   
}

int disc_state_machine(char byte_received, int state)
{
     switch (state)
     {
    case START:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }

        break;

    case FLAG_RCV:

        if (byte_received == 0x03)
        {
            state = A_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case A_RCV:
        if (byte_received == 0x0B)
        {
            state = C_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case C_RCV:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        if (byte_received == (0x03 ^ 0x0B))
        {
            state = BCC_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case BCC_RCV:
        if (byte_received == 0x7E)
        {
            state = STOP_S;
        }
        else
        {
            state = START;
        }
    }

    return state;
}


void rr_state_machine(char byte_received, char controlBuf, int state)
{
    switch (state)
    {

    case START:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }

        break;

    case FLAG_RCV:

        if (byte_received == 0x03)
        {
            state = A_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case A_RCV:
        if ((byte_received == RR0 || byte_received == REJ0) && controlBuf == 0x00)
        {
            state = C_RCV;
        } 
	if ((byte_received == RR1 || byte_received == REJ1) && controlBuf == 0x40)
        {
            state = C_RCV;
        }
        else if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case C_RCV:
        if (byte_received == 0x7E)
        {
            state = FLAG_RCV;
        }
        if (byte_received == (0x03 ^ 0x07))
        {
            state = BCC_RCV;
        }
        else
        {
            state = START;
        }
        break;

    case BCC_RCV:
        if (byte_received == 0x7E)
        {
            state = STOP_S;
        }
        else
        {
            state = START;
        }
    }

    
}

void send_disc_message(int fd){

    char buf[255];
    buf[0] = 0x7E;

    buf[1] = 0x03;

    buf[2] = 0x0B;              

    buf[3] = buf[1] ^ buf[2];

    buf[4] = 0x7E;

    int res = write(fd, buf, 5);

}


void send_set_message(int fd)
{
    printf("Trying to send set message\n");
    char buf[255];
    buf[0] = 0x7E;

    buf[1] = 0x03;

    buf[2] = 0x03;

    buf[3] = buf[1] ^ buf[2];

    buf[4] = 0x7E;

    int res = write(fd, buf, 5);

    printf("Message set sent:  %2s:%d\n", buf, res);
}

void send_UA_Message(int fd)
{

    printf("Trying to send message confirmation.\n");

    char buf[255];
    int res;
    buf[0] = 0x7E;
    buf[1] = 0x03;
    buf[2] = 0x07;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = 0x7E;

    res = write(fd, buf, 5);

    printf("Message confirmation sent:  %2s:%d\n", buf, res);
}



void send_rr_message(int r, int fd)
{
    printf("Trying to send rr message\n");
    char buf[255];
    buf[0] = 0x7E;

    buf[1] = 0x03;

    if(r=0)
    	buf[2] = RR0;
    else 
	buf[2] = RR1;



    buf[3] = buf[1] ^ buf[2];

    buf[4] = 0x7E;

    int res = write(fd, buf, 5);

    printf("Message rr sent:  %2s:%d\n", buf, res);
}



void send_rej_message(int r, int fd)
{
    printf("Trying to send rej message\n");
    char buf[255];
    buf[0] = 0x7E;

    buf[1] = 0x03;

    if(r=0)
    	buf[2] = REJ0;
    else 
	buf[2] = REJ1;



    buf[3] = buf[1] ^ buf[2];

    buf[4] = 0x7E;


    int res = write(fd, buf, 5);

    printf("Message rej sent:  %2s:%d\n", buf, res);
}





int set_save_port_settings(int fd, struct termios *oldtio)
{
    struct termios newtio;

    if (fd < 0)
    {
        return -1;
    }
    if (tcgetattr(fd, oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    return 0;
}


int checkBcc2(char *buffer, int res)
{
  char bcc2 = buffer[0];

  for (int i = 1; i < res - 1; i++)
  {
    bcc2 ^= buffer[i];
  }

  return (bcc2 == buffer[res - 1]);
  
}

char calculateBcc2(char *buffer, int length)
{
  char bcc2 = buffer[0];
  
  for (int i = 1; i < length; i++)
  {
    bcc2 ^= buffer[i];
  }

  return bcc2;
}



