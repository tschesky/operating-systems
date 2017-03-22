#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>
#include <sys/times.h>

// Program sorting the specified file, using bubble sort and system calls
// Program parameters are: filename.txt, length of one record
int main(int argc, char* argv[]){
	
	// Check if the number of arguments is OK
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return 1;
	}

	// Read the arguments
	const char* fileName = argv[1];
	const int lineLenght = atoi(argv[2]);

	// Buffers used to sort our file. Only two records are kept in the memory at any given time
	char* buff = (char*)calloc(lineLenght, sizeof(char));
	char* buff2 = (char*)calloc(lineLenght, sizeof(char));

	// Opening the file
	int fileDesc = open(fileName, O_RDWR);
	
	// Handling any problems that may arise
	if(fileDesc < 0){
		printf("Some trouble opening the file\n");
		if( close(fileDesc) < 0){
			printf("Some trouble closing the file");
		}
		return 1;
	}
	
	// Variable use to determine if the file is already sorted
	struct tms start_time;
	struct tms end_time;

	if ( times(&start_time) < 0) printf("Problem with times function!\n");

	int swapped = 1;
	int proper;
	int proper2;
	while(swapped){
		swapped = 0;
		// Go to the beggining of the file
		lseek(fileDesc, 0, SEEK_SET);
		// Read a line, store the result in an integer
		proper = read(fileDesc, buff, lineLenght);
		// Omit the newline character
		lseek(fileDesc, 1, SEEK_CUR);
		// Read next line
		proper2 = read(fileDesc, buff2, lineLenght);
		// Omit the newline character
		lseek(fileDesc, 1, SEEK_CUR);

		// Swapping elements till the end of the file
		while( (proper != 0) && (proper2 != 0) ){
			// Checking if the lines were read properly
			if( proper < 0 && proper2 < 0){
				printf("Some trouble reading from the file, improper line lenght at line: ");
				// If not, close the file
				if( close(fileDesc) < 0){
					printf("Some trouble closing the file");
				}
				return 1;	
			}
			
			// Compare two strings, if they are in a wrong order swap them
			if( strcmp(buff, buff2) > 0 ){
				// Swapped, so we need to set the variable
				swapped = 1;
				// Go back two lines
				lseek(fileDesc, -((2*lineLenght)+2), SEEK_CUR);
				// Write buff2
				write(fileDesc, buff2, lineLenght);
				write(fileDesc, "\n", 1);
				// Write buff
				write(fileDesc, buff, lineLenght);
				write(fileDesc, "\n", 1);
			}
			// Go back one line
			lseek(fileDesc, -(lineLenght+1), SEEK_CUR);

			//Load lines into the buffers again
			proper = read(fileDesc, buff, lineLenght);
			lseek(fileDesc, 1, SEEK_CUR);
			proper2 = read(fileDesc, buff2, lineLenght);
			lseek(fileDesc, 1, SEEK_CUR);
		}
	}

	if ( times(&end_time) < 0) printf("Problem with times function!\n");
	else{
		printf("\tSorting wiht system calls:\n");
		printf("\tUser time = %fms\n", (((double)(end_time.tms_utime - start_time.tms_utime)) / sysconf(_SC_CLK_TCK)) * 1000);
        printf("\tSystem time = %fms\n\n",(((double)(end_time.tms_stime - start_time.tms_stime)) / sysconf(_SC_CLK_TCK)) * 1000);
	}

	if( close(fileDesc) < 0){
			printf("Some trouble closing the file");
		}

	free(buff);
	free(buff2);
	return 0;
}
