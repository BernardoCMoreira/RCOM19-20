#include "link.h"

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

    for (int i = 1; i < length; i++)
    {
        bcc2 ^= buffer[i];
    }

    return bcc2;
}