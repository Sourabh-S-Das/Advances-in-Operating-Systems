#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main()
{
    int fd = open("/proc/partb_1_20CS30051_20CS30061", O_RDWR);
    char k = (char)7;
    printf("%d\n", k);
    int ret_val = write(fd, &k, 1);
    if(ret_val < 0)
    {
        printf("Error : Could not initialise\n");
        return 0;
    }

    int fd1 = open("/proc/partb_1", O_RDWR);
    if(fd1 < 0)
    {
        printf("Error: The proc filer could not be initialised\n");
    }

    for (int i = 0; i < 7; i++) {
    	int x;
    	scanf("%d", &x);
        int ret = write(fd, &x, sizeof(int));
        printf("Process[%d] Write: %d, Return: %d\n", getpid(), x, ret);
        usleep(100);
    }
    for (int i = 0; i < 7; i++) {
        int out;
        int ret = read(fd, &out, sizeof(int));
        printf("Process[%d] Read: %d, Return: %d\n", getpid(), out, ret);
        usleep(100);
    }
    close(fd);

    return 0;
}
