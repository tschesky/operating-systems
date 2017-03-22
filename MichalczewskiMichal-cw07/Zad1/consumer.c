#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define SHARED_MEMORY_MAX 64
#define MAX_PROCESSES  SHARED_MEMORY_MAX
#define RAND_RANGE 300
#define PRIME 30;
#define NOT_PRIME 40;

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/sem.h>

#define SHARED_KEY 1
#define SEMAPHORE_KEY 2
#define ACCESS_KEY 3

#define PRODUCER 69
#define CONSUMER 96

// Handler for stopping the process, quitting the program
void quitHandler();
void exitHandler();

// Producer/consumer
void consumer();
void producer();

// Check if prime number
int isPrime(int num);

// Shared memory pointer, global so we can access it from the at_exit function
int semPointer;
int sharedPointer;

void *sharedMemory;
int *sharedArray;
int *currentConsumer;
int *currentProducer;
int *consumerCounter;
int *producerCounter;

int semAlreadyExists = 0;
int whoAmI = 0;

// Structures for controlling the semaphores
struct sembuf arrayControlStart;
struct sembuf arrayControlStop;
struct sembuf arrayAccessIn;
struct sembuf arrayAccessOut;

union semun {
	int val;                /* wartość dla SETVAL */
	struct semid_ds *buf;   /* bufor dla IPC_STAT i IPC_SET */
	ushort *array;          /* tablica dla GETALL i SETALL */
	struct seminfo *__buf;  /* bufor dla IPC_INFO */
	void *__pad;
};

int main(int argc, char* argv[]){

	// Set the handler and atExit
	signal(SIGTSTP, quitHandler);
	atexit(exitHandler);
	
	// Check no. of arguments
	if(argc != 2){
		printf("Wrong number of arguments, you need to specify consumer/producer, using \"P/p\" or \"C/c\"!\n");
		return 1;
	}

	if(strlen(argv[1]) > 1){
		printf("The argument is invalid(too long!)\n");
		return 1;
	}

	// Shared memory 
	key_t shmKey= ftok(".", SHARED_KEY);
	sharedPointer = shmget(shmKey, SHARED_MEMORY_MAX + 4, IPC_CREAT | 0600);
	if(sharedPointer == -1){
		printf("Error in function shmget()!\n");
		exit(EXIT_FAILURE);
	}

	// Initialize the variables in shared memory segment
	sharedMemory = shmat(sharedPointer, NULL, 0);
	if(sharedMemory == (void*)-1){
		printf("Error in function shmat()!\n");
		exit(EXIT_FAILURE);
	}
	currentProducer = ((int *)sharedMemory);
	currentConsumer = ((int *)sharedMemory + 1);
	producerCounter = ((int *)sharedMemory + 2);
	consumerCounter = ((int *)sharedMemory + 3);
	sharedArray = ((int *)sharedMemory + 4);

	// Semaphores
	key_t semKey= ftok(".", SEMAPHORE_KEY);
	semPointer = semget(semKey, 2, IPC_CREAT | IPC_EXCL | 0600);

	// Check if the semaphore already exists
	if(semPointer == -1){
		if(errno == EEXIST){
			printf("The semaphore already exists, some other process has already started!\n");
			semAlreadyExists = 1;
		}else{
			printf("Error in function semget()!\n");
			exit(EXIT_FAILURE);
		}
	}

	// If the semget returned an error because of the semaphore already existing, we open we once again - this time without creating
	if(semAlreadyExists == 1){
		semPointer = semget(semKey, 1, 0);
		if(semPointer == -1){
			printf("Error in function semget()!\n");
			exit(EXIT_FAILURE);
		}
	}

	arrayControlStart.sem_num = 0;
	arrayControlStart.sem_op = -1;
	arrayControlStart.sem_flg = 0;

	arrayControlStop.sem_num = 0;
	arrayControlStop.sem_op = 1;
	arrayControlStop.sem_flg = 0;

	arrayAccessIn.sem_num = 1;
	arrayAccessIn.sem_op = -1;
	arrayAccessIn.sem_flg = 0;

	arrayAccessOut.sem_num = 1;
	arrayAccessOut.sem_op = 1;
	arrayAccessOut.sem_flg = 0;

	// Semaphore doesn't exist, it's the first instance of the program so we have to initialize the variables;
	if(semAlreadyExists == 0){

		// Set the max no of operations
		union semun tmp;
		tmp.val = MAX_PROCESSES;
		if(semctl(semPointer, 0, SETVAL, tmp) == -1){
			printf("Error in function semctl()!");
			exit(EXIT_FAILURE);
		}

		// Set the initial value of semaphore responsible for array access
		tmp.val = 1;
		if(semctl(semPointer, 1, SETVAL, tmp) == -1){
			printf("Error in function semctl()!");
			exit(EXIT_FAILURE);
		}
		// Initial number of tasks
		*currentConsumer = 0;
		*currentProducer = 0;
		*consumerCounter = 0;
		*producerCounter = 0;
	}

	// Execute for consumer
	if(strcmp(argv[1], "c") == 0 || strcmp(argv[1], "C") == 0){
		printf("Consumer!\n");
		whoAmI = CONSUMER;
		consumer();
	}

	// Execute from producer
	if(strcmp(argv[1], "p") == 0 || strcmp(argv[1], "P") == 0 ){
		printf("Producer!\n");
		whoAmI = PRODUCER;
		producer();
	}

	exit(EXIT_SUCCESS);
}

// Producer function
void producer(){

	// Variables for printing output
	pid_t pid = getpid();
	char timeBuff[20];
	time_t tmpTime;
	struct tm* timeinfo;
	char output[100];
	srand(time(NULL));
	(*producerCounter)++;

	// Main program loop
	while(1){
		//	Descrease the semaphore resposible for no. of allowed processses, lock the array to perform operations
		if(semop(semPointer, &arrayControlStart, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
		}
		if(semop(semPointer, &arrayAccessIn, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
		}
		// Write random crap into array
		sharedArray[*currentProducer] = (int)rand() % RAND_RANGE;

		tmpTime = time(NULL);
		timeinfo = localtime(&tmpTime);
		strftime(timeBuff, sizeof(timeBuff), "%R", timeinfo);

		// Output info
		sprintf(output, "(%d, %s) wrote number:\t%d to array. Waiting tasks:\t%d\n" , pid, timeBuff, sharedArray[*currentProducer], (*producerCounter + *consumerCounter));
		printf("%s", output);
		(*currentProducer)++;

		// Release lock for array
		if(semop(semPointer, &arrayAccessOut, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
		}
		// Wait for some time
		usleep(5000000);
	}
}

// Consumer function
void consumer(){

	// Output info
	pid_t pid = getpid();
	char timeBuff[20];
	time_t tmpTime;
	struct tm* timeinfo;
	char output[100];
	(*consumerCounter)++;

	// Main program loop
	while(1){
		// Lock semaphores
		if(semop(semPointer, &arrayControlStart, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
		}
		if(semop(semPointer, &arrayAccessIn, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
		}
		
		tmpTime = time(NULL);
		timeinfo = localtime(&tmpTime);
		strftime(timeBuff, sizeof(timeBuff), "%R", timeinfo);

		if( isPrime(sharedArray[*currentConsumer]) == 0 ){
			sprintf(output, "(%d, %s) Checked number:\t%d and it is not prime.\n" , pid, timeBuff, sharedArray[*currentConsumer]);
			printf("%s", output);
		}
		else if( isPrime(sharedArray[*currentConsumer]) == 1 ){
			sprintf(output, "(%d, %s) Checked number:\t%d and it is prime.\n" , pid, timeBuff, sharedArray[*currentConsumer]);
			printf("%s", output);
		}
		(*currentConsumer)++;

		// Release lock for array
		if(semop(semPointer, &arrayAccessOut, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
		}

		usleep(5000000);

	}
	

}

// Handler for SIGSTP singal
void quitHandler(){
	exit(EXIT_SUCCESS);
}

// AtExit function
void exitHandler(){
	printf("Stopping the process, detaching the shared memory segment!\n");
	shmdt(sharedArray);
	if(semop(semPointer, &arrayControlStop, 1) == -1){
			printf("Error in function semop()!\n");
			exit(EXIT_FAILURE);
	}

	if(whoAmI == CONSUMER) (*producerCounter)--;
	else if(whoAmI == PRODUCER) (*consumerCounter)--;

	if(semAlreadyExists == 0){
		semctl(semPointer, 0, IPC_RMID);
		shmctl(sharedPointer, IPC_RMID, NULL);
	}
	shmdt(sharedArray);
}

// Check if prime, return 0 if it's not, return 1 if it is
int isPrime(int number){

	if (number <= 1) return 0; // zero and one are not prime
    unsigned int i;
    for (i=2; i*i<=number; i++) {
        if (number % i == 0) return 0;
    }
    return 1;
}
