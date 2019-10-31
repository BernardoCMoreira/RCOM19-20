/*Non-Canonical Input Processing*/
#include "writer.h"

int alarmCount = 0;
int flag = 0;
int expectedControlByte = 0;

struct termios oldtio, newtio;


void alarmHandler()
{
  printf("Alarm#%d\n", alarmCount + 1);
  flag = 1;
  alarmCount++;
}

int main(int argc, char **argv)
{
  int fd;
  off_t sizeFile; 
  off_t index = 0;
  int res = 0;

  if ((argc < 3) ||
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


  (void)signal(SIGALRM, alarmHandler);
  
  int sizeOfFileName = strlen(argv[2]);
  unsigned char *fileName = (unsigned char *)malloc(sizeOfFileName);
  

  FILE *f;
  struct stat metadata;
  unsigned char *mensagem;
  fileName = argv[2];

  if ((f = fopen((char *)fileName, "rb")) == NULL)
  {
    perror("error opening file!");
    exit(-1);
  }
  stat((char *)fileName, &metadata);
  (sizeFile) = metadata.st_size;

  mensagem = (unsigned char *)malloc(sizeFile);

  fread(mensagem, sizeof(unsigned char), sizeFile, f);

  printf("Establishing connection...\n");

  llopen(fd);

  printf("Connection established\n");

  printf("Writing start packet...\n");

  unsigned char *start = getControlPacket(C_START, sizeFile, fileName, sizeOfFileName, &res);

  llwrite(fd, start, res);

  printf("Start packet has been written\n");


  int sizePacket = INITIAL_PACKET_SIZE;

  printf("Writing data packets...\n");


  while (sizePacket == INITIAL_PACKET_SIZE && index < sizeFile)
  {
    
    unsigned char *packet = getPacket(mensagem, &index, &sizePacket, sizeFile);
    
    int headerSize = sizePacket;
    unsigned char *mensagemHeader = addHeader(packet, sizeFile, &headerSize);
    
    if (!llwrite(fd, mensagemHeader, headerSize))
    {
      printf("NUM_ALARM reached\n");
      return -1;
    }
  }

  printf("Data packets have been written\n");

  printf("Writing end message...\n");

  unsigned char *end = getControlPacket(C_END, sizeFile, fileName, sizeOfFileName, &res);
  llwrite(fd, end, res);

  printf("End message has been written\n");

  printf("Attempting to close...\n");

  llclose(fd);

  printf("Successfully closed\n");

  sleep(1);

  close(fd);
  return 0;
}


int llopen(int fd)
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

  newtio.c_cc[VTIME] = 1; /* inter-unsigned character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 unsigned chars received */

  /*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a

  leitura do(s) pr�ximo(s) caracter(es)

  */

  tcflush(fd, TCIOFLUSH);


  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");


  unsigned char c;
  do
  {
    write_ctrl_frame(fd, SET);
    alarm(TIMEOUT);
    flag = 0;
    int state = 0;

    while (!stop && !flag)
    {
      read(fd, &c, 1);
      ua_state_machine(&c, &state);
    }
  } while (flag && alarmCount < NUM_ALARM);
  if (flag && alarmCount == 3)
  {
    return 0;
  }
  else
  {
    flag = 0;
    alarmCount = 0;
    return 1;
  }
}

int llwrite(int fd, unsigned char *buffer, int size)
{
  unsigned char bcc2;
  unsigned char *bcc2Stuffed = (unsigned char *)malloc(sizeof(unsigned char));
  unsigned char *buf = (unsigned char *)malloc((size + 6) * sizeof(unsigned char));
  int bufSize = size + 6;
  int bcc2Size = 1;
  bcc2 = calculateBcc2(buffer, size);

  if (bcc2 == FLAG_RCV)
  {
    bcc2Stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char *));
    bcc2Stuffed[0] = ESC;
    bcc2Stuffed[1] = 0x5e;
    bcc2Size++;
  }
  else
  {
    if (bcc2 == ESC)
    {
      bcc2Stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char *));
      bcc2Stuffed[0] = ESC;
      bcc2Stuffed[1] = 0x5d;
      bcc2Size++;
    }
  }

  int lastRr = 0;

  buf[0] = FLAG_RCV;
  buf[1] = A_RCV;
  if (expectedControlByte == 0)
  {
    buf[2] = C10;
  }
  else
  {
    buf[2] = C11;
  }
  buf[3] = (buf[1] ^ buf[2]);

  int i = 0;
  int j = 4;
  for (; i < size; i++)
  {
    if (buffer[i] == FLAG_RCV)
    {
      buf = (unsigned char *)realloc(buf, ++bufSize);
      buf[j] = ESC;
      buf[j + 1] = 0x5e;
      j = j + 2;
    }
    else
    {
      if (buffer[i] == ESC)
      {
        buf = (unsigned char *)realloc(buf, ++bufSize);
        buf[j] = ESC;
        buf[j + 1] = 0x5d;
        j = j + 2;
      }
      else
      {
        buf[j] = buffer[i];
        j++;
      }
    }
  }

  if (bcc2Size == 1)
    buf[j] = bcc2;
  else
  {
    buf = (unsigned char *)realloc(buf, ++bufSize);
    buf[j] = bcc2Stuffed[0];
    buf[j + 1] = bcc2Stuffed[1];
    j++;
  }
  buf[j + 1] = FLAG_RCV;

  do
  {

    write(fd, buf, bufSize);

    flag = 0;
    alarm(TIMEOUT);
    unsigned char C = read_ctrl_frame(fd);
    if ((C == RR1 && expectedControlByte == 0) || (C == RR0 && expectedControlByte == 1))
    {
      lastRr = 0;
      alarmCount = 0;
      expectedControlByte ^= 1;
      alarm(0);
    }
    else
    {
      if (C == REJ1 || C == REJ0)
      {
        lastRr = 1;
        alarm(0);
      }
    }
  } while ((flag && alarmCount < NUM_ALARM) || lastRr);
  if (alarmCount >= NUM_ALARM)
    return 0;
  else
    return 1;
}



void llclose(int fd)
{
  write_ctrl_frame(fd, DISC);
  unsigned char controlByte;
  
  controlByte = read_ctrl_frame(fd);

  while (controlByte != DISC)
  {
    controlByte = read_ctrl_frame(fd);
  }
  
  write_ctrl_frame(fd, UA);
  

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }
}

