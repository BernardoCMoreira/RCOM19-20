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
#include "linklayer.h"
#include "linklayer.c"

volatile int STOP = FALSE;

int flag = 1, conta = 1, fd;
int state = 0;
int lastS=0;
int writerVar=0, readerVar=0;

//alarmes

void alarm_handler() // atende alarme
{
    printf("alarme # %d\n", conta);
    flag = 1;
    conta++;
    send_set_message(fd);                 
}

void write_alarm_handler() // atende alarme
{
    printf("alarme # %d\n", conta);
    flag = 1;
    conta++;  
}

//auxiliar functions

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
                disc_state_machine(conf[0],state);
		
	        if (state == STOP_S)
	        { /* so we can printf... */
			printf("\nConfirmation message was obtained succesfully\n");
			STOP = TRUE;
			alarm(0);
		}
            if (conta >= 4)
             {
                 printf("ERROR: already resent message 3 times\n");
                 return ;
                }   
            }

            printf("DISC message received \n");
}






//LL functions

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
        send_set_message(fd);
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
            ua_state_machine(conf, state);
	    if (state == STOP_S)
	    { /* so we can printf... */
		printf("\nConfirmation message was obtained succesfully\n");
		STOP = TRUE;
		alarm(0);
	    }

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

            state = svision_state_machine(recepBuf[0], SET, state);

            if (state == STOP_S)
            {
                printf("Channel open: %2s:%d\n", recepBuf, res);
                STOP = TRUE;
            }
        }

        send_UA_Message(fd);

        sleep(1);
    }

    printf("Succesfully opened");
    return fd;
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
            rr_state_machine(conf,buf[2], state);
	    if (state == STOP_S)
	    { /* so we can printf... */
		printf("\nRR message was obtained succesfully\n");
		STOP = TRUE;
		alarm(0);
	    }
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
	int res=0;
	state=0;
	int rByte=-1;

	int count=0;
	printf("\nFD: %d\n", fd);

	while(state != STOP_S){
	   
		read(fd,&buf,1);
		state = info_state_machine(buf, buffer, &res, &rByte, state);
		count++;
	}

	if(checkBcc2(buffer,res)){
		send_rr_message(rByte, fd);	
		printf("Confirmation Succesful.\n");
	}
	else{
		send_rej_message(rByte, fd);
		printf("Confirmation Failed.\n");
	}

	printf("\n\nMESSAGE SUCCESSFULLY READ\n\n");

	
    return res;
}






int llclose(int fd){
    char conf[255];
    int res;
    //if sender:: send DISC, read DISC, SEND UA!
    if (writerVar == TRUE){
        send_disc_message(fd);
        printf("DISC message sent!\n");

        ack_disc_message(conf);
        sleep(1);

        printf("DISC message received!\n");
        send_UA_Message(fd);
        printf("UA message sent!\n");
    }
    else{

            //read disc
            ack_disc_message(conf);
            //send disc
      

            printf("DISC message received!");
            send_disc_message(fd);
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
            ua_state_machine(conf,state);
	    if (state == STOP_S)
	    { /* so we can printf... */
		printf("\nConfirmation message was obtained succesfully\n");
		STOP = TRUE;
		alarm(0);
	    }
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



