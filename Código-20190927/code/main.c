#include <stdio.h>
#include "link.c"

int main(int argc, char *argv[])
{
    int fd;
    if ((argc < 3) || ((strcmp("read", argv[1]) != 0) && (strcmp("write", argv[1]))))
    {
        printf("Wrong number of arguments");
        return -1;
    }

    //Hard coded. testing
    fd = llopen(argv[2], RECETOR);

    // llcolse(fd, RECETOR);

    return 0;
}