/*Non-Canonical Input Processing*/
#include "reader.h"

int esperado = 0;
struct termios oldtio, newtio;

int main(int argc, char **argv)
{
  int fd;
  int sizeMessage = 0;
  unsigned char *mensagemPronta;
  int sizeOfStart = 0;
  unsigned char *start;
  off_t sizeOfGiant = 0;
  unsigned char *giant;
  off_t index = 0;

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0)))
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


  llopen(fd);
  start = llread(fd, &sizeOfStart);

  
  int L2 = (int)start[8];
  unsigned char *nameOfFile = (unsigned char *)malloc(L2 + 1);

  
  for (int i = 0; i < L2; i++)
  {
    nameOfFile[i] = start[9 + i];
  }

  nameOfFile[L2] = '\0';

  
  sizeOfGiant = (start[3] << 24) | (start[4] << 16) | (start[5] << 8) | (start[6]);

  giant = (unsigned char *)malloc(sizeOfGiant);

  while (TRUE)
  {
    mensagemPronta = llread(fd, &sizeMessage);
    if (sizeMessage == 0)
      continue;
    if (reachedEnd(mensagemPronta))
    {
      printf("End message received\n");
      break;
    }

    int sizeWithoutHeader = 0;

    mensagemPronta = removeHeader(mensagemPronta, sizeMessage, &sizeWithoutHeader);

    memcpy(giant + index, mensagemPronta, sizeWithoutHeader);
    index += sizeWithoutHeader;
  }

  printf("Mensagem: \n");
  int i = 0;
  for (; i < sizeOfGiant; i++)
  {
    printf("%x", giant[i]);
  }


  FILE *file = fopen((char *)nameOfFile, "wb+");
  fwrite((void *)giant, 1, sizeOfGiant, file);
  fclose(file);

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

  if (control_state_machine(fd, SET))
  {
    printf("Recebeu SET\n");
    write_ctrl_frame(fd, UA);
    printf("Mandou UA\n");
  }
}

unsigned char *llread(int fd, int *sizeMessage)
{
  unsigned char *message = (unsigned char *)malloc(0);
  *sizeMessage = 0;
  unsigned char c_read;
  int trama = 0;
  int mandarDados = FALSE;
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
        trama = 0;
        c_read = c;
        state = 3;
      }
      else if (c == C11)
      {
        trama = 1;
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
        if (checkBcc2(message, *sizeMessage))
        {
          if (trama == 0)
            write_ctrl_frame(fd, RR1);
          else
            write_ctrl_frame(fd, RR0);

          state = 6;
          mandarDados = TRUE;
          printf("Enviou RR, T: %d\n", trama);
        }
        else
        {
          if (trama == 0)
            write_ctrl_frame(fd, REJ1);
          else
            write_ctrl_frame(fd, REJ0);
          state = 6;
          mandarDados = FALSE;
          printf("Enviou REJ, T: %d\n", trama);
        }
      }
      else if (c == ESC)
      {
        state = 5;
      }
      else
      {
        message = (unsigned char *)realloc(message, ++(*sizeMessage));
        message[*sizeMessage - 1] = c;
      }
      break;
    case 5:
      if (c == 0x5E)
      {
        message = (unsigned char *)realloc(message, ++(*sizeMessage));
        message[*sizeMessage - 1] = FLAG_RCV;
      }
      else
      {
        if (c == 0x5D)
        {
          message = (unsigned char *)realloc(message, ++(*sizeMessage));
          message[*sizeMessage - 1] = ESC;
        }
        else
        {
          perror("Non valid character after escape character");
          exit(-1);
        }
      }
      state = 4;
      break;
    }
  }
  printf("Message size: %d\n", *sizeMessage);
  message = (unsigned char *)realloc(message, *sizeMessage - 1);

  *sizeMessage = *sizeMessage - 1;
  if (mandarDados)
  {
    if (trama == esperado)
    {
      esperado ^= 1;
    }
    else
      *sizeMessage = 0;
  }
  else
    *sizeMessage = 0;
  return message;
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

void write_ctrl_frame(int fd, unsigned char controlByte){

  unsigned char frame[5];
  frame[0] = FLAG_RCV;
  frame[1] = A_RCV;
  frame[2] = controlByte;
  frame[3] = frame[1] ^ frame[2];
  frame[4] = FLAG_RCV;

  write(fd, frame, 5);
}

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
