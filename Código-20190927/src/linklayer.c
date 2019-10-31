#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "linklayer.h"



int control_state_machine(int fd, unsigned char controlByte)
{
  int state = 0;
  unsigned char byte_received;

  while (state != 5)
  {
    read(fd, &byte_received, 1);

    switch (state)
    {
    case 0:
      if (byte_received == FLAG_RCV)
        state = 1;
      break;

    case 1:
      if (byte_received == A_RCV)
        state = 2;
      else
      {
        if (byte_received == FLAG_RCV)
          state = 1;
        else
          state = 0;
      }
      break;

    case 2:
      if (byte_received == controlByte)
        state = 3;
      else
      {
        if (byte_received == FLAG_RCV)
          state = 1;
        else
          state = 0;
      }
      break;

    case 3:
      if (byte_received == (A_RCV ^ controlByte))
        state = 4;
      else
        state = 0;
      break;
      
    case 4:
      if (byte_received == FLAG_RCV)
      {
        state = 5;
      }
      else
        state = 0;
      break;
    }
  }
  return 1;
}




void write_ctrl_frame(int fd, unsigned char controlByte){

  unsigned char frame[5];
  frame[0] = FLAG_RCV;
  frame[1] = A_RCV;
  frame[2] = controlByte;
  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG_RCV;

  write(fd, frame, 5);
}


int checkBcc2(unsigned char *buffer, int res){

  unsigned char bcc2 = buffer[0];

  for (int i = 1; i < res - 1; i++)
  {
    bcc2 ^= buffer[i];
  }

  if (bcc2 == buffer[res - 1])
  {
    return 1;
  }
  else
    return 2;

}



unsigned char *removeHeader(unsigned char *buffer, int bufferLength, int *dataLength)
{
  
  unsigned char *noHeader = (unsigned char *)malloc(bufferLength - 4);

  for (int i = 0; i < bufferLength; i++)
  {
    noHeader[i] = buffer[i+4];
  }

  *dataLength = bufferLength - 4;

  return noHeader;
}



int reachedEnd(unsigned char *end)
{
    if (end[0] == C_END)
      return 1;
    else
      return 0;
    
    
}
