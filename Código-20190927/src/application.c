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

void alarm_handler() // atende alarme
{
    printf("alarme # %d\n", conta);
    flag = 1;
    conta++;
    send_set_message();
}

int llopen(char *porta, int flagE_R)
{

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
        sleep(1);
    }
    else if (flagE_R == READER)
    {

        while (STOP == FALSE)
        {

            res = read(fd, recepBuf, 1); /* returns after 5 chars have been input */

            state = set_state_machine(recepBuf[0]);

            if (state == STOP_S)
            {
                printf("%2s:%d\n", recepBuf, res);
                STOP = TRUE;
            }
        }

        Send_UA_Message(fd);

        sleep(1);
    }
    return 0;
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

    newtio.c_cc[VTIME] = 0.1; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 0;

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    return 0;
}


char calculateBcc2(char *buffer, int length)
{
  char bcc2 = buffer[0];
  
  for (int i = 1; i < size; i++)
  {
    bcc2 ^= buffer[i];
  }

  return bcc2;
}


int llwrite(int fd, char * buffer, int length){

	
 	unsigned char *buf = (unsigned char *)malloc((length + 6) * sizeof(unsigned char));
	int res = length + 6;
	
	char bcc2 = calculateBcc2(buffer, length);

    buf[0] = 0x7E;
	
    buf[1] = 0x03;

    buf[2] = 0x03;

    buf[3] = buf[1] ^ buf[2];

     int current_index = 4;
  for (int i = 0; i < length; i++)
  {
    if (buffer[i] == FLAG)
    {
	buf = (unsigned char *)realloc(buf, ++res);

      buf[current_index] = ESC;
      buf[current_index+1] = buffer[i] ^ ESC_OR;
      current_index += 2;
    }
    else
    {
      if (buffer[i] == ESC)
      {
	buf = (unsigned char *)realloc(buf, ++res);
        buf[current_index] = ESC;
        buf[current_index + 1] = buffer[i] ^ ESC_OR;
        current_index+= 2;
      }
      else
      {
        buf[current_index] = buffer[i];
        current_index++;
      }
    }
  }







}


int set_state_machine(char byte_received)
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
        if (byte_received == 0x03)
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

    char buf[255];
    buf[0] = 0x7E;

    buf[1] = 0x03;

    buf[2] = 0x03;

    buf[3] = buf[1] ^ buf[2];

    buf[4] = 0x7E;

    int res = write(fd, buf, 5);

    printf("%d bytes written\n", res);
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

void Send_UA_Message(int fd)
{

    printf("Trying to send message confirmation.\n");

    char buf[255];
    buf[0] = 0x7E;
    buf[1] = 0x03;
    buf[2] = 0x07;
    buf[3] = buf[1] ^ buf[2];
    buf[4] = 0x7E;

    write(fd, buf, 5);

    printf("Message confirmation sent.\n");
}
