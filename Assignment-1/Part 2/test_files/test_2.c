#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

int main()
{
    int fd = open("/proc/partb_1_20CS30051_20CS30061", O_RDWR);
    int n;
    printf("Enter the size of deque: ");
    scanf("%d", &n);
    char k = (char)n;
    int ret_val = write(fd, &k, 1);
    
    if(ret_val < 0)
    {
        printf("Error : Could not initialise\n");
        return 0;
    }

    for (int i = 0; i < n; i++) {
    	int x;
    	printf("Enter a number to be written to the deque: ");
    	scanf("%d", &x);
        int ret = write(fd, &x, sizeof(int));
        printf("Process[%d] Write: %d, Return: %d\n", getpid(), x, ret);
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
