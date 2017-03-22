#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define SHARED_MEMORY_MAX 64
#define MAX_PROCESSES  SHARED_MEMORY_MAX
#define RAND_RANGE 300
#define PRIME 30;
#define NOT_PRIME 40;

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

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

// Functions for creating/opening shared variables, depending on wheter they already have been created
void openStuff();
void createStuff();
void mapStuff();

// Check if prime number
int isPrime(int num);

// Shared memory pointer, global so we can access it from the at_exit function
int producerCounterTmp;
int consumerCounterTmp;
int currentProducerTmp;
int currentConsumerTmp;
int sharedPointer;

void *sharedMemory;
int *sharedArray;
int *currentConsumer;
int *currentProducer;
int *consumerCounter;
int *producerCounter;

sem_t *arrayAccess;
sem_t *populationControl;

int semAlreadyExists = 0;
int whoAmI = 0;


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

	// Try to create shared memory segment, if it already exists just open it with rd/wrt permissions
	semAlreadyExists = 0;
	sharedPointer = shm_open("/sharedArray", O_RDWR | O_CREAT | O_EXCL, 0600);
	if(sharedPointer == -1){
		if(errno == EEXIST){
			semAlreadyExists = 1;
			printf("Shared memory already exists, we dont have to create it.\n");
		}
		else{
			printf("Problem in function shm_open()!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Already exists, we just open all variables - make a function() for that!
	if(semAlreadyExists == 1){
		openStuff();
	} else {
	// The segment doesn't exist, so we need to create all the variables - make a function() for that!
		createStuff();
	}
	// Map the descriptors to pointers
	mapStuff();
	// If it's the first instance of program make the values 0
	if(semAlreadyExists == 0){
		(*producerCounter) = 0;
		(*consumerCounter) = 0;
		(*currentProducer) = 0;
		(*currentConsumer) = 0;
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
		sem_wait(arrayAccess);
		// Write random crap into array
		sharedArray[*currentProducer] = (int)rand() % RAND_RANGE;

		tmpTime = time(NULL);
		timeinfo = localtime(&tmpTime);
		strftime(timeBuff, sizeof(timeBuff), "%R", timeinfo);

		// Output info
		sprintf(output, "(%d, %s) wrote number:\t%d to array. Waiting tasks:\t%d\n" , pid, timeBuff, sharedArray[*currentProducer], (*producerCounter + *consumerCounter));
		printf("%s", output);
		(*currentProducer)++;
		sem_post(arrayAccess);
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
	sem_wait(populationControl);

	// Main program loop
	while(1){
		sem_wait(arrayAccess);

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
		sem_post(arrayAccess);

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

	sem_post(populationControl);

	if(whoAmI == CONSUMER) (*producerCounter)--;
	else if(whoAmI == PRODUCER) (*consumerCounter)--;

	sem_unlink("/arrayAccess");
	sem_unlink("/populationControl");

	if(semAlreadyExists == 1){
		shm_unlink("/sharedArray");
		shm_unlink("/currentConsumer");
		shm_unlink("/currentProducer");
		shm_unlink("/producerCount");
		shm_unlink("/consumerCount");
	}




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

void createStuff(){
		arrayAccess = sem_open("/arrayControl", O_CREAT, 0600, 1);
		if(arrayAccess == SEM_FAILED){
			printf("Error in sem_open!\n");
			exit(EXIT_FAILURE);
		}

		populationControl = sem_open("/populationControl", O_CREAT, 0600, SHARED_MEMORY_MAX);
		if(populationControl == SEM_FAILED){
			printf("Error in sem_open!\n");
			exit(EXIT_FAILURE);
		}

		producerCounterTmp = shm_open("/producerCount",O_RDWR | O_CREAT, 0600);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		consumerCounterTmp = shm_open("/consumerCount", O_RDWR | O_CREAT, 0600);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		currentProducerTmp = shm_open("/currentProducer", O_RDWR | O_CREAT, 0600);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		currentConsumerTmp = shm_open("/currentConsumert", O_RDWR | O_CREAT, 0600);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		if(ftruncate(sharedPointer, sizeof(int) * SHARED_MEMORY_MAX) == -1){
			printf("Error in ftruncate()!\n");
			exit(EXIT_FAILURE);
		}

		if(ftruncate(consumerCounterTmp, sizeof(int)) == -1){
			printf("Error in ftruncate()!\n");
			exit(EXIT_FAILURE);
		}

		if(ftruncate(producerCounterTmp, sizeof(int)) == -1){
			printf("Error in ftruncate()!\n");
			exit(EXIT_FAILURE);
		}

		if(ftruncate(currentConsumerTmp, sizeof(int)) == -1){
			printf("Error in ftruncate()!\n");
			exit(EXIT_FAILURE);
		}

		if(ftruncate(currentProducerTmp, sizeof(int)) == -1){
			printf("Error in ftruncate()!\n");
			exit(EXIT_FAILURE);
		}
}

void openStuff(){
		arrayAccess = sem_open("/arrayControl", 0);
		if(arrayAccess == SEM_FAILED){
			printf("Error in sem_open!\n");
			exit(EXIT_FAILURE);
		}

		populationControl = sem_open("/populationControl", 0);
		if(populationControl == SEM_FAILED){
			printf("Error in sem_open!\n");
			exit(EXIT_FAILURE);
		}

		sharedPointer = shm_open("/sharedArray",O_RDWR | O_CREAT, 0600);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);

		}

		producerCounterTmp = shm_open("/producerCount", O_RDWR, 0);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		consumerCounterTmp = shm_open("/consumerCount", O_RDWR, 0);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		currentProducerTmp = shm_open("/currentProducer", O_RDWR, 0);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}

		currentConsumerTmp = shm_open("/currentConsumert", O_RDWR, 0);
		if(sharedPointer == -1){
			printf("Error in shm_open!\n");
			exit(EXIT_FAILURE);
		}
}

void mapStuff(){

	sharedArray = (int *) mmap(NULL, SHARED_MEMORY_MAX, PROT_READ | PROT_WRITE, MAP_SHARED, sharedPointer, 0);
	if(sharedArray == MAP_FAILED){
		printf("Error in mmap()!\n");
		exit(EXIT_FAILURE);
	}

	producerCounter = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, producerCounterTmp, 0);
	if(producerCounter == MAP_FAILED){
		printf("Error in shm_open!\n");
		exit(EXIT_FAILURE);
	}

	consumerCounter = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, consumerCounterTmp, 0);
	if(consumerCounter == MAP_FAILED){
		printf("Error in mmap()!\n");
		exit(EXIT_FAILURE);
	}

	currentProducer = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, currentProducerTmp, 0);
	if(currentProducer == MAP_FAILED){
		printf("Error in mmap()!\n");
		exit(EXIT_FAILURE);
	}

	currentConsumer = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, currentConsumerTmp, 0);
	if(currentConsumer == MAP_FAILED){
		printf("Error in mmap()!\n");
		exit(EXIT_FAILURE);
	}
}