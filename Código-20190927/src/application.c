#include "application.h"

volatile int STOP = FALSE;

int flag = 1, conta = 1, fd;
int state = 0;

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

int llwrite(int fd, char *buffer, int length)
{

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
        if (buffer[i] == FLAG || buffer[i] == ESC)
        {
            buf = (unsigned char *)realloc(buf, ++res);

            buf[current_index] = ESC;
            buf[current_index + 1] = buffer[i] ^ ESC_OR;
            current_index += 2;
        }
        else
        {
            buf[current_index] = buffer[i];
            current_index++;
        }
    }

    if (bcc2 = FLAG || bcc2 == ESC)
    {

        buf = (unsigned char *)realloc(buf, ++res);
        buf[current_index] = ESC;
        buf[current_index + 1] = bcc2 ^ ESC_OR;
        current_index += 2;
    }

    buf[current_index] = FLAG;
}
