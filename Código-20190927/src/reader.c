/*Non-Canonical Input Processing*/
#include "reader.h"

int expectedControlByte = 0;
struct termios oldtio, newtio;

int main(int argc, char **argv)
{
  int fd;
  int res = 0;
  int startPacketSize = 0;
  off_t blobSize = 0;
  off_t index = 0;
  unsigned char *buffer;
  unsigned char *startPacket;
  unsigned char *blob;

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0) &&
       (strcmp("/dev/ttyS2", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }
  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  printf("Establishing connection...\n");
  llopen(fd);
  printf("Connection established\n");

  //reading and parsing start packet
  

  printf("Reading start packet...\n");
  startPacket = llread(fd, &startPacketSize);
 
  int L2Byte = (int)startPacket[8];
  unsigned char *fileName = (unsigned char *)malloc(L2Byte + 1);

  
  for (int i = 0; i < L2Byte; i++)
  {
    fileName[i] = startPacket[9 + i];
  }

  fileName[L2Byte] = '\0';


  printf("Start packet obtained\n");
  
  //reading data packets

  printf("Reading data packets...\n");

  blobSize = (startPacket[3] << 24) | (startPacket[4] << 16) | (startPacket[5] << 8) | (startPacket[6]);

  blob = (unsigned char *)malloc(blobSize);

  while (TRUE)
  {
    buffer = llread(fd, &res);
    if (res == 0)
      continue;
    if (reachedEnd(buffer))
    {
      printf("End message has been reached\n");
      break;
    }

    int sizeWithoutHeader = 0;

    buffer = removeHeader(buffer, res, &sizeWithoutHeader);

    memcpy(blob + index, buffer, sizeWithoutHeader);
    index += sizeWithoutHeader;
  }

  
  printf("Data packets have been read succesfully\n");

  //saving to file

  printf("Creating file...\n");

  FILE *file = fopen((char *)fileName, "wb+");
  fwrite((void *)blob, 1, blobSize, file);
  fclose(file);


  printf("File done\n");




  llclose(fd);


  sleep(1);
 
  close(fd);
  return 0;
}

void llclose(int fd)
{

  printf("Trying to close...\n");
  control_state_machine(fd, DISC);
  write_ctrl_frame(fd, DISC);
  control_state_machine(fd, UA);
  printf("Succesfully closed\n");

  tcsetattr(fd, TCSANOW, &oldtio);
}

void llopen(int fd)
{

if (tcgetattr(fd, &oldtio) == -1)
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

  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  printf("New termios structure set\n");

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("Reading SET message...\n");
  if (control_state_machine(fd, SET))
  {

    printf("SET message succesfully written\n");
    write_ctrl_frame(fd, UA);
  }
  printf("Sent UA message comfirmation\n");
}

unsigned char *llread(int fd, int *bufferSize)
{
  unsigned char *buffer = (unsigned char *)malloc(0);
  *bufferSize = 0;
  unsigned char c_read;
  int frame = 0;
  int sendStatus = FALSE;
  unsigned char c;
  int state = 0;

  while (state != 6)
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
      if (c == C10)
      {
        frame = 0;
        c_read = c;
        state = 3;
      }
      else if (c == C11)
      {
        frame = 1;
        c_read = c;
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
      if (c == (A_RCV ^ c_read))
        state = 4;
      else
        state = 0;
      break;
    case 4:
      if (c == FLAG_RCV)
      {
        if (checkBcc2(buffer, *bufferSize))
        {
          if (frame == 0)
            write_ctrl_frame(fd, RR1);
          else
            write_ctrl_frame(fd, RR0);

          state = 6;
          sendStatus = TRUE;
        }
        else
        {
          if (frame == 0)
            write_ctrl_frame(fd, REJ1);
          else
            write_ctrl_frame(fd, REJ0);
          state = 6;
          sendStatus = FALSE;
        }
      }
      else if (c == ESC)
      {
        state = 5;
      }
      else
      {
        buffer = (unsigned char *)realloc(buffer, ++(*bufferSize));
        buffer[*bufferSize - 1] = c;
      }
      break;
    case 5:
      if (c == 0x5E)
      {
        buffer = (unsigned char *)realloc(buffer, ++(*bufferSize));
        buffer[*bufferSize - 1] = FLAG_RCV;
      }
      else
      {
        if (c == 0x5D)
        {
          buffer = (unsigned char *)realloc(buffer, ++(*bufferSize));
          buffer[*bufferSize - 1] = ESC;
        }
        else
        {
          perror("Character after ESC not recognized");
          exit(-1);
        }
      }
      state = 4;
      break;
    }
  }
  buffer = (unsigned char *)realloc(buffer, *bufferSize - 1);

  *bufferSize = *bufferSize - 1;
  if (sendStatus)
  {
    if (frame == expectedControlByte)
    {
      expectedControlByte ^= 1;
    }
    else
      *bufferSize = 0;
  }
  else
    *bufferSize = 0;
  return buffer;
}
