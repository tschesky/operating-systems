#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG 256

int main(int argc, char* argv[])
{
	// Check if the number of arguments is OK
	if(argc != 2){
		printf("Wrong number of arguments\n");
		return 1;
	}

	const char* fifoName = argv[1];
	char* pid = calloc(16, sizeof(char));

	/* create the FIFO (named pipe) */
    if( mkfifo(fifoName, 0666) == -1){
    	printf("Some trouble making the FIFO!\n");
        return 1;
    }

    int fd;
    char *buf = calloc(MAX_MSG, sizeof(char));

    /* open, read, and display the message from the FIFO */
    fd = open(fifoName, O_RDONLY);

    if(fd < 0){
        printf("Some trouble opening the file\n");
        if( close(fd) < 0){
            printf("Some trouble closing the file\n");
        }
        return 1;
    }

    while(1){
	    read(fd, buf, MAX_MSG);

	    if(strcmp(buf, "q") == 0 || strcmp(buf, "Q") == 0) break;
	    printf("%s\n", buf);

	    memset(buf, 0, MAX_MSG);
	    memset(pid, 0, 16);
	}

    if( close(fd) < 0){
        printf("Some trouble closing the file\n");
        return 1;
    }

    /* remove the FIFO */
    unlink(fifoName);

    return 0;
}
