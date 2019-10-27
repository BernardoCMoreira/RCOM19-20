#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "application.h"

volatile int STOP = FALSE;

int flag = 1, conta = 1, fd;
int state = 0;
int lastS=0;
int writerVar=0, readerVar=0;

void alarm_handler() // atende alarme
{
    printf("alarme # %d\n", conta);
    flag = 1;
    conta++;
    send_set_message();                 
}

void write_alarm_handler() // atende alarme
{
    printf("alarme # %d\n", conta);
    flag = 1;
    conta++;  
}



int llopen(char *porta, int flagE_R)
{
    printf("Trying to open\n");
    int c, res;
    struct termios oldtio;
    int i, sum = 0, speed = 0;
    char recepBuf[255];

    // debug
    if (flagE_R != WRITER && flagE_R != READER)
    {
        printf("ERROR: invalid flag!");
        return -1;
    }
    if (
        ((strcmp(MODEMDEVICE0, porta) != 0) &&
         (strcmp(MODEMDEVICE1, porta) != 0) &&
         (strcmp(MODEMDEVICE2, porta) != 0)))
    {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    fd = open(porta, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(porta);
        exit(-1);
	return -1;
    }

    set_save_port_settings(fd, &oldtio);

    printf("New termios structure set\n");

    if (flagE_R == WRITER)
    {
        send_set_message();
        (void)signal(SIGALRM, alarm_handler);

        char conf[255];
        while (STOP == FALSE && conta < 4)
        {
            if (flag)
            {
                alarm(3); // activa alarme de 3s
                flag = 0;
            }
            res = read(fd, conf, 1); /* returns after 5 chars have been input */
            //verify UA message
            ua_state_machine(conf);
            if (conta >= 4)
            {
                printf("ERROR: already resent message 3 times\n");
            }
        }

        printf("Read message confirmation: %2s:%d\n", conf, res);
	
        sleep(1);
    }
    else if (flagE_R == READER)
    {

        while (STOP == FALSE)
        {

            res = read(fd, recepBuf, 1); /* returns after 5 chars have been input */

            state = svision_state_machine(recepBuf[0], SET);

            if (state == STOP_S)
            {
                printf("Channel open: %2s:%d\n", recepBuf, res);
                STOP = TRUE;
            }
        }

        Send_UA_Message(fd);

        sleep(1);
    }

    printf("Succesfully opened");
    return fd;
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


int llwrite(int fd, char * buffer, int length){

	printf("Trying to write message\n");
 	unsigned char *buf = (unsigned char *)malloc((length + 6) * sizeof(unsigned char));
	int res = length + 6;
	
	char bcc2 = calculateBcc2(buffer, length);

    buf[0] = 0x7E;

	
    buf[1] = 0x03;

	if(lastS ==0){
    		buf[2] = 0x00;
		lastS=1; 
	}
	else{
		buf[2] = 0x40; 
		lastS=0;
	}

    buf[3] = buf[1] ^ buf[2];
	

     int current_index = 4;
  for (int i = 0; i < length; i++)
  {
    if (buffer[i] == FLAG)
    {
	buf = (unsigned char *)realloc(buf, ++res);

      buf[current_index] = ESC;
      printf("\nStuff %d: %x\n",current_index, buf[current_index]);
      buf[current_index+1] = 0x5e;
      printf("\nStuff %d: %x\n",(current_index+1), buf[current_index+1]);
      current_index += 2;
    }
    if (buffer[i] == ESC)
    {
	buf = (unsigned char *)realloc(buf, ++res);

      buf[current_index] = ESC;
      printf("\nStuff %d: %x\n",current_index, buf[current_index]);
      buf[current_index+1] = 0x5d;
      printf("\nStuff %d: %x\n",(current_index+1), buf[current_index+1]);
      current_index += 2;
    }
    else{
	buf[current_index] = buffer[i];
	current_index++;
    }
  }

	if(bcc2 == FLAG){

		buf = (unsigned char *)realloc(buf, ++res);
		buf[current_index] = ESC;
      		printf("\nStuff BCC %d: %x\n",current_index, buf[current_index]);
		buf[current_index + 1] = 0x5d;
      		printf("\nStuff BCC %d: %x\n",(current_index +1), buf[current_index+1]);
      		current_index += 2;
	}

	else if(bcc2 == ESC){

		buf = (unsigned char *)realloc(buf, ++res);
		buf[current_index] = ESC;
      		printf("\nStuff BCC %d: %x\n",current_index, buf[current_index]);
		buf[current_index + 1] = 0x5e;
      		printf("\nStuff BCC %d: %x\n",(current_index +1), buf[current_index+1]);
      		current_index += 2;
	}
	else{
		printf("\nDidnt Stuff BCC: %x\n",bcc2);
		buf[current_index] = bcc2;
		current_index++;
	}


	printf("Escrevendo a flag no index: %d", current_index);


	buf[current_index] = FLAG;

	for(int i =0; i < res; i++){
		printf("buf %d: %x\n",i, buf[i]);
	}
	
	printf("FD: %d\n", fd);
	write(fd, buf, res);
	printf("Message has been written\n");

	
	
        (void)signal(SIGALRM, write_alarm_handler);

	char conf;
	STOP=FALSE;
        while (STOP == FALSE && conta < 4)
        {
            if (flag)
            {
                alarm(3); // activa alarme de 3s
                flag = 0;
		write(fd,buf,res);
            }
            res = read(fd, &conf, 1); /* returns after 5 chars have been input */
            //verify UA message
	    if((conf == REJ1 && buf[2] == 0x40) || (conf == REJ0 && buf[2] == 0x00))
		alarm(0);
            rr_state_machine(conf,buf[2]);
            if (conta >= 4)
            {
                printf("ERROR: already resent message 3 times\n");
            }
        }

        printf("Read message confirmation: %c:%d\n", conf, res);

	


        sleep(1);
	return 0;

}

int llread(int fd, char * buffer){

	unsigned char buf;
	//unsigned char *buf =(unsigned char *)malloc(sizeof(unsigned char)); 
	int res=0;
	state=0;
	int rByte=-1;

	int count=0;
	printf("\nFD: %d\n", fd);

	while(state != STOP_S){
	   
		read(fd,&buf,1);
		//printf("Byte_received: %x\n", buf[0]);
        
		state = info_state_machine(buf, buffer, &res, &rByte);
		count++;

	}

	
	if(checkBcc2(buffer,res)){
		send_rr_message(rByte);	
		printf("Confirmation Succesful.\n");
	}
	else{
		send_rej_message(rByte);
		printf("Confirmation Failed.\n");
	}


	

	printf("\n\nMESSAGE SUCCESSFULLY READ\n\n");

	
    return 0;
}

int info_state_machine(char byte_received, char* buffer, int* res, int* rByte)
{

    unsigned char controlByte;
    if(byte_received != START || state == A_RCV){
	printf("%x\n",byte_received);}

   /* printf("FLAG char: %x\n", '~');
    printf("ESC char: %x\n", '}');
    printf("ESCESC char: %x\n", ']');*/

	
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



void send_rr_message(int r)
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



void send_rej_message(int r)
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


int svision_state_machine(char byte_received, char controlField)
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





void send_set_message()
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

void ua_state_machine(char conf[])
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

    if (state == STOP_S)
    { /* so we can printf... */
        printf("\nConfirmation message was obtained succesfully\n");
        STOP = TRUE;
        alarm(0);
    }
}



void rr_state_machine(char byte_received, char controlBuf)
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

    if (state == STOP_S)
    { /* so we can printf... */
        printf("\nRR message was obtained succesfully\n");
        STOP = TRUE;
        alarm(0);
    }
}


void Send_UA_Message(int fd)
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

int llclose(int fd){
    char conf[255];
    int res;
    //if sender:: send DISC, read DISC, SEND UA!
    if (writerVar == TRUE){
        send_disc_message();
        printf("DISC message sent!\n");

        ack_disc_message(conf);
        sleep(1);

        printf("DISC message received!\n");
        Send_UA_Message(fd);
        printf("UA message sent!\n");
    }
    else{

            //read disc
            ack_disc_message(conf);
            //send disc
      

            printf("DISC message received!");
            send_disc_message();
            printf("DISC message sent!");
            //read ua!
             res = read(fd, conf, 1);

            while (STOP == FALSE && conta < 4)
            {
                if (flag)
                {
                 alarm(3); // activa alarme de 3s
                 flag = 0;
                }
            res = read(fd, conf, 1);
            ua_state_machine(conf);
            if (conta >= 4)
             {
                 printf("ERROR: already resent message 3 times\n");
                 return -1;
                }   
            }

            printf("UA message received!");

         }
   

 if (close(fd)<0) 
        return -1;
    return 0;
}

void ack_disc_message(char conf[255]){
        (void)signal(SIGALRM, alarm_handler);

            while (STOP == FALSE && conta < 4)
            {
                if (flag)
                {
                 alarm(3); // activa alarme de 3s
                 flag = 0;
                }
                int res = read(fd, conf, 1);
                disc_state_machine(conf[0]);
            if (conta >= 4)
             {
                 printf("ERROR: already resent message 3 times\n");
                 return ;
                }   
            }

            printf("DISC message received \n");
}

void send_disc_message(void){

    char buf[255];
    buf[0] = 0x7E;

    buf[1] = 0x03;

    buf[2] = 0x0B;              

    buf[3] = buf[1] ^ buf[2];

    buf[4] = 0x7E;

    int res = write(fd, buf, 5);

}

int disc_state_machine(char byte_received)
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
      if (state == STOP_S)
    { /* so we can printf... */
        printf("\nConfirmation message was obtained succesfully\n");
        STOP = TRUE;
        alarm(0);
    }

    return state;
}
