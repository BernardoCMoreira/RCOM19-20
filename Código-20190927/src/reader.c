/*Non-Canonical Input Processing*/

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
#include "application.c"

int readerVAR = TRUE;

int main(int argc, char** argv)
{

	llopen(argv[1],READER);
    return 0;
}

