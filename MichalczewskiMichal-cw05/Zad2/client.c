#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// Max length of the msg
#define MAX_MSG 256

// Get line from input
char * getline(void);

int main(int argc, char* argv[])
{

    // Variables for string operations
	char *message = malloc(MAX_MSG * sizeof(char));
	char* line = malloc(2*MAX_MSG*sizeof(char));
	char* timbuf = malloc(32*sizeof(char));

	// Check if the number of arguments is OK
	if(argc != 2){
		printf("Wrong number of arguments\n");
		return 1;
	}

    // Name of our fifo, opening the fifo
	const char* fifoName = argv[1];
	int fd = open(fifoName, O_WRONLY);

    // Checking if we properly accessed the fifo
    if(fd < 0){
        printf("Some trouble opening the file\n");
        if( close(fd) < 0){
            printf("Some trouble closing the file\n");
        }
        return 1;
    }

	while(1){
		printf("Please input your messgage (Input q/Q to end the program):\n");
        // Get the mesage
		message = getline();

        // Get time stamp
		time_t rawtime;
	  	struct tm * timeinfo;
	  	time ( &rawtime );
	 	timeinfo = localtime ( &rawtime );

        // Format the time to our liking
	 	strftime(timbuf, sizeof(timbuf), "%H:%M", timeinfo);

        // Check if the user wants to quit the program
        if(strcmp(message, "q") == 0 || strcmp(message, "Q") == 0){
            // If yes, pass the information to server
            write(fd, message, strlen(message));
            break;
        } 

        // Format output string and write it to FIFO
		sprintf(line, "%d\t%s\t%s", getpid(), timbuf, message );
	    write(fd, line, strlen(line));

	}

    // After all has been done, close the file
    if( close(fd) < 0){
        printf("Some trouble closing the file\n");
        return 1;
    }

    return 0;
}

char * getline(void) {
    char * line = malloc(100*sizeof(char)), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);

        if(c == EOF)
            break;

        if(c == '\n')
            break;

        (*line++ = c);

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }
    }
    *line = '\0';
    return linep;
}