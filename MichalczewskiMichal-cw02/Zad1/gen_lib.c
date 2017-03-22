#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// File generator using library functions
int main(int argc, char* argv[]){

	// Initialize the seed
   	srand(time(NULL)); 

   	// Check if the no. of arguments is OK - we need: filename.txt, no. of record, no. of characters per record
	if(argc != 4){
		printf("Wrong number of arguments\n");
		return 1;
	}

	// Reading the paremeters
	const char* fileName = argv[1];
	const int noOfLines = atoi(argv[2]);
	const int arraySize = atoi(argv[3]);

	//printf("Lenght of a generated array: %u\n", arraySize);

	// Opening the file
	FILE* fHandle = fopen(fileName, "w");
	
	// Handling problems with opening
	if(fHandle == NULL){
		printf("Some trouble opening the file\n");
		if( fclose(fHandle) != 0){
			printf("Some trouble closing the file");
		}
		return 1;
	}

	// Buffer used to generate random strings
	char* buff = (char*)calloc(arraySize, sizeof(char));



	// Loop generating a string, writing it to file as many times as neccessary
	for(int i = 0; i < noOfLines; i++){
		for(int j = 0; j < arraySize; j++){
			buff[j] = (char)((rand() % 25) + 65);		
		}
		// Writing line to the file
		if( fwrite(buff, sizeof(char), arraySize, fHandle) != (unsigned int)arraySize){
			printf("Some trouble writing to the file\n");
			// If sth went wrong I close the file
			if( fclose(fHandle) != 0){
				printf("Some trouble closing the file");
			}
			return 1;
		}
		// Writing the newline character
    	if( fwrite("\n", sizeof(char), 1, fHandle) != 1){
			printf("Some trouble writing to the file\n");
			// If sth went wrong I close the file
			if( fclose(fHandle) != 0){
				printf("Some trouble closing the file");
			}
			return 1;
		}
	}

	// Closing the file after all has been done
	if( fclose(fHandle) != 0){
		printf("Some trouble closing the file");
	}

	// Freeing the buffer space
	free(buff);

	return 0;
}