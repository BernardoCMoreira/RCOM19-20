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

int packetNumber = 0;
int stop = 0;
extern int flag ;

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

unsigned char *addHeader(unsigned char *buffer, off_t fileSize, int *packetSize)
{
  unsigned char *finalPacket = (unsigned char *)malloc(fileSize + 4);
  finalPacket[0] = DATA_PACKET;
  finalPacket[1] = packetNumber % 255;
  finalPacket[2] = (int)fileSize / 256;
  finalPacket[3] = (int)fileSize % 256;
  
  memcpy(finalPacket + 4, buffer, *packetSize);
  
  *packetSize += 4;
  packetNumber++;

  return finalPacket;
}

unsigned char *getPacket(unsigned char *buffer, off_t *index, int *packetSize, off_t fileSize)
{
 
  off_t tmpIndex = *index;
  if (*index + *packetSize > fileSize)
  {
    *packetSize = fileSize - *index;
  }
   unsigned char *packet = (unsigned char *)malloc(*packetSize);

  for (int i=0 ; i < *packetSize; i++, tmpIndex++)
  {
    packet[i] = buffer[tmpIndex];
  }

  *index = tmpIndex;
  return packet;
}

void ua_state_machine(unsigned char *conf, int *state)
{

  switch (*state)
  {
  case 0:
    if (*conf == FLAG_RCV)
      *state = 1;
    break;

  case 1:
    if (*conf == A_RCV)
      *state = 2;
    else
    {
      if (*conf == FLAG_RCV)
        *state = 1;
      else
        *state = 0;
    }
    break;

  case 2:
    if (*conf == UA)
      *state = 3;
    else
    {
      if (*conf == FLAG_RCV)
        *state = 1;
      else
        *state = 0;
    }
    break;

  case 3:
    if (*conf == (A_RCV ^ UA))
      *state = 4;
    else
      *state = 0;
    break;

  case 4:
    if (*conf == FLAG_RCV)
    {
      stop = 1;
      alarm(0);
    }
    else
      *state = 0;
    break;
  }
}

unsigned char read_ctrl_frame(int fd)
{
  int state = 0;
  unsigned char c;
  unsigned char C;

  while (!flag && state != 5)
  {
    read(fd, &c, 1);
    switch (state)
    {
    case 0:
      if (c == FLAG_RCV)
        state = 1;
      break;
      
    case 1:
      if (c == A_RCV)
        state = 2;
      else
      {
        if (c == FLAG_RCV)
          state = 1;
        else
          state = 0;
      }
      break;
      
    case 2:
      if (c == RR0 || c == RR1 || c == REJ0 || c == REJ1 || c == DISC)
      {
        C = c;
        state = 3;
      }
      else
      {
        if (c == FLAG_RCV)
          state = 1;
        else
          state = 0;
      }
      break;

    case 3:
      if (c == (A_RCV ^ C))
        state = 4;
      else
        state = 0;
      break;

    case 4:
      if (c == FLAG_RCV)
      {
        alarm(0);
        state = 5;
        return C;
      }
      else
        state = 0;
      break;
    }
  }
  return 0xFF;
}

unsigned char calculateBcc2(unsigned char *mensagem, int size)
{
  unsigned char BCC2 = mensagem[0];
  int i;
  for (i = 1; i < size; i++)
  {
    BCC2 ^= mensagem[i];
  }
  return BCC2;
}


unsigned char *getControlPacket(unsigned char controlField, off_t fileSize, unsigned char *fileName, int fileNameLength, int *res)
{
  *res = 9 * sizeof(unsigned char) + fileNameLength;
  unsigned char *package = (unsigned char *)malloc(*res);

  if (controlField == C_START)
    package[0] = C_START;
  else
    package[0] = C_END;

  package[1] = T1;
  package[2] = L1;
  package[3] = (fileSize >> 24) & 0xFF;
  package[4] = (fileSize >> 16) & 0xFF;
  package[5] = (fileSize >> 8) & 0xFF;
  package[6] = fileSize & 0xFF;
  package[7] = T2;
  package[8] = fileNameLength;
  
  for (int k = 0; k < fileNameLength; k++)
  {
    package[9 + k] = fileName[k];
  }

  return package;
}