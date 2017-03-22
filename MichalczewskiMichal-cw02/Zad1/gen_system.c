#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// File generator using system calls
int main(int argc, char* argv[]){

	// Initialize the seed
   	srand(time(NULL)); 

   	// Check if the number of arguments is OK. We need: filename.txt, no. of record, lenght of one record
	if(argc != 4){
		printf("Wrong number of arguments\n");
		return 1;
	}

	// Reading the arguments
	const char* fileName = argv[1];
	const int noOfLines = atoi(argv[2]);
	const int arraySize = atoi(argv[3]);

	//printf("Lenght of a generated array: %u\n", arraySize);

	// Opening the file
	int fileDesc = open(fileName, O_CREAT | O_RDWR | O_TRUNC);
	
	// Handling errors with opening the file
	if(fileDesc < 0){
		printf("Some trouble opening the file\n");
		if( close(fileDesc) < 0){
			printf("Some trouble closing the file");
		}
		return 1;
	}

	// Buffer for generating strings
	char* buff = (char*)calloc(arraySize, sizeof(char));

	// Loop generating a string, writing it to file as many times as neccessary
	for(int i = 0; i < noOfLines; i++){
		for(int j = 0; j < arraySize; j++){
			buff[j] = (char)((rand() % 25) + 65);		
		}
		// Writing line to the file
		if( write(fileDesc, buff, arraySize) != arraySize){
			printf("Some trouble writing to the file\n");
			// If sth went wrong I close the file
			if( close(fileDesc) < 0){
				printf("Some trouble closing the file");
			}
			return 1;
		}
		// Writing the newline character
    	if( write(fileDesc, "\n", 1) != 1){
			printf("Some trouble writing to the file\n");
			// If sth went wrong I close the file
			if( close(fileDesc) < 0){
				printf("Some trouble closing the file");
			}
			return 1;
		}
	}

	// Closing the file after all has been done
	if( close(fileDesc) < 0){
		printf("Some trouble closing the file");
	}

	// Freeing the buffer space
	free(buff);

	return 0;
}
