#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main()
{
    int val[] = {4, 5, 9, 3, 2, 1, 254561};
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
        printf("Sahi Chal rha hai boss\n");
    }

    for (int i = 0; i < 7; i++) {
        int ret = write(fd, &val[i], sizeof(int));
        printf("Process[%d] Write: %d, Return: %d\n", getpid(), val[i], ret);
        usleep(100);
    }
    for (int i = 0; i < n; i++) {
        int out;
        int ret = read(fd, &out, sizeof(int));
        printf("Process[%d] Read: %d, Return: %d\n", getpid(), out, ret);
        usleep(100);
    }
    close(fd);

    return 0;
}