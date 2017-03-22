#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#define TAB_SIZE 256
#define NO_READERS 10
#define RAND_RANGE 256

#define READER 1
#define WRITER 2

int tab[TAB_SIZE];

// Handler for stopping the process, quitting the program
void quitHandler();
void exitHandler();

void clearArray();

// Execute functions
void* reader(void* arg);
void* writer(void* arg);

// Semaphores
int activeReaders = 0;
int activeWriters = 0;
int workingReaders = 0;
int workingWriters = 0;

sem_t readerSem;
sem_t writerSem;

pthread_mutex_t variableProtection;
pthread_mutex_t writerExclude;

// Arrays of threads
pthread_t *readerTab;
pthread_t *writerTab;


int main(int argc, char* argv[]){

	// Set the handler and atExit
	signal(SIGTSTP, quitHandler);
	atexit(exitHandler);

	// Check arguments
	if(argc != 2){
		printf("Wrong number of arguments, you need to specify the number of readers!\n");
		return 1;
	}

	int no_writers = atoi(argv[1]);

	// Init tabs of threads
	if( (readerTab = malloc(sizeof(pthread_t) * NO_READERS)) == NULL){
		printf("Problem with malloc() while creating readers!\n");
		exit(EXIT_FAILURE);
	}
	if( (writerTab = malloc(sizeof(pthread_t) * no_writers)) == NULL){
		printf("Problem with malloc() while creating wirters!\n");
		exit(EXIT_FAILURE);
	}

	// Clear the tab of integers
	clearArray();


	// Initialize the semaphores
	if(sem_init(&readerSem, 0, 0) == - 1){
		printf("Problem with sem_init() while creating readers!\n");
		exit(EXIT_FAILURE);
	}
	// Initialize the semaphores
	if(sem_init(&readerSem, 0, 0) == - 1){
		printf("Problem with sem_init() while creating writers!\n");
		exit(EXIT_FAILURE);
	}

	// Initialize and set mutex attributes to a standard mutex, it's simple but prone to errors (No error detection and undefined behaviour for many cases)
	pthread_mutexattr_t attr;
	if(pthread_mutexattr_init(&attr) != 0 ){
		printf("Error in function pthread_mutexattr_init()!\n");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) != 0){
		printf("Error in function pthread_mutexattr_settype()!\n");
		exit(EXIT_FAILURE);
	}
	// Initialize the mutex 1
	if(pthread_mutex_init(&variableProtection, &attr) != 0){
		printf("Error in function pthread_mutex_init()!\n");
		exit(EXIT_FAILURE);
	}
	// Initialize the mutex 2
	if(pthread_mutex_init(&writerExclude, &attr) != 0){
		printf("Error in function pthread_mutex_init()!\n");
		exit(EXIT_FAILURE);
	}

	// Create writers
	for(int i = 0; i < no_writers; i++){
		if(pthread_create(writerTab + i, NULL, writer, NULL) != 0){
			printf("Error in function pthread_create()!\n");
			exit(EXIT_FAILURE);
		}
	}

	srand(time(NULL));
	int randomInteger[NO_READERS];

	// Create readers
	for(int i = 0; i < NO_READERS; i++){
		randomInteger[i] = (int)rand() % RAND_RANGE;
		if(pthread_create(readerTab + i, NULL, reader, randomInteger + i) != 0){
			printf("Error in function pthread_create()!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Wait for writers to finish eating
	for(int i = 0; i < NO_READERS; i++){
		if(pthread_join(readerTab[i], NULL) != 0){
			printf("Error in function pthread_join()!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Wait for readers to finish eating
	for(int i = 0; i < no_writers; i++){
		if(pthread_join(writerTab[i], NULL) != 0){
			printf("Error in function pthread_join()!\n");
			exit(EXIT_FAILURE);
		}
	}

	


	exit(EXIT_SUCCESS);
}

// Reader function
void* reader(void* arg){

	usleep(500000);
	int number = *(int*)arg;
	int index = 0;
	int found = 0;

	srand(time(NULL));
	
	while(1){

		// Ask to modify variables
		if(pthread_mutex_lock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}
			activeReaders++;
			if(activeWriters == 0){
				while(workingReaders < activeReaders){
					workingReaders++;
					sem_post(&readerSem);
				}
			}

		// Stop modyfing variables
		if(pthread_mutex_unlock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		// Read from tab
		sem_wait(&readerSem);
		if(tab[index] == number){
			found++;
		}
		index++;
		index = index % TAB_SIZE;

		// Ask to modify variables
		if(pthread_mutex_lock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}
			
			workingReaders--;
			activeReaders--;
			if(workingReaders == 0){
				while(workingWriters < activeWriters){
					workingWriters++;
					sem_post(&writerSem);
				}
			}

		// Stop modyfing variables
		if(pthread_mutex_unlock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		printf("Czytelnik szukajacy liczby:\t%d, obecnie w:\t%d elemencie, odnaleziono:\t%d\n", number, index, found);
		usleep(1000000 * ((int)rand() % 5));

	}
}

// Writer function
void* writer(void* arg){

	usleep(500000);


	srand(time(NULL));
	int index;
	int number;

	while(1){

		// Ask to modify variables
		if(pthread_mutex_lock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}
			activeWriters++;
			if(activeReaders == 0){
				while(workingWriters < activeWriters){
					workingWriters++;
					sem_post(&writerSem);
				}
			}

		// Stop modyfing variables
		if(pthread_mutex_unlock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		sem_wait(&writerSem);
		if(pthread_mutex_lock(&writerExclude) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		index = (int)rand() % TAB_SIZE;
		number = (int)rand() % RAND_RANGE;
		tab[index] = number;

		if(pthread_mutex_unlock(&writerExclude) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		// Ask to modify variables
		if(pthread_mutex_lock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}
			workingWriters--;
			activeWriters--;

			if(workingWriters == 0){

				while(workingReaders < activeReaders){
					workingReaders++;
					sem_post(&readerSem);
				}

			}

		// Stop modyfing variables
		if(pthread_mutex_unlock(&variableProtection) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		printf("Pisarz wpisal liczbe:\t%d, pod index:\t%d\n", number, index);
		usleep(1000000 * ((int)rand() % 5));


	}

}

// Fill array wit 0's
void clearArray(){

	for (int i = 0; i < TAB_SIZE; i++)
	{
		tab[i] = 0;
	}

}

// Handler function for exiting at SIGSTP
void quitHandler(){

}

// At exit function
void exitHandler(){

}