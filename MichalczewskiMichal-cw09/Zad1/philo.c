#define _GNU_SOURCE

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
#include <pthread.h>

// Size of the problem
#define NO_PHILOSOPHERS 5

// Handler for stopping the process, quitting the program
void quitHandler();
void exitHandler();

// Monitor - waiter variables
pthread_t philosophers[NO_PHILOSOPHERS];
pthread_mutex_t waiter;

int id[NO_PHILOSOPHERS];			// Id's of philosophers
int whoIsEating[NO_PHILOSOPHERS];	// when tab[i] = 0, the i'th philosopher isn't eating
int whoHasFork[NO_PHILOSOPHERS];	// when tab[i] = -1, no one is currently in possesion of this fork

// Function to create threads from
void* threadFunction(void* arg);

// Functions to operate on forks
void pickForkUp(int philosopherID, int forkID);
void putForkDown(int philosopherID, int forkID);


int main(int argc, char* argv[]){

	// Set the handler and atExit
	signal(SIGTSTP, quitHandler);
	atexit(exitHandler);

	// Mutex attribute object
	pthread_mutexattr_t attr;

	// Initialize and set mutex attributes to a standard mutex, it's simple but prone to errors (No error detection and undefined behaviour for many cases)
	if(pthread_mutexattr_init(&attr) != 0 ){
		printf("Error in function pthread_mutexattr_init()!\n");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL) != 0){
		printf("Error in function pthread_mutexattr_settype()!\n");
		exit(EXIT_FAILURE);
	}
	// Initialize the mutex used by philosophers - there is only one, arbitrary mutex
	if(pthread_mutex_init(&waiter, &attr) != 0){
		printf("Error in function pthread_mutex_init()!\n");
		exit(EXIT_FAILURE);
	}

	// Init the variables
	for(int i = 0; i < NO_PHILOSOPHERS; i++){
		id[i] = i;
		whoIsEating[i] = 0;
		whoHasFork[i] = -1;
	}

	srand((int)time(NULL));

	// Create the philosophers
	for(int i = 0; i < NO_PHILOSOPHERS; i++){
		if(pthread_create(philosophers + i, NULL, threadFunction, id + i) != 0){
			printf("Error in function pthread_create()!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Wait for philosophers to finish eating
	for(int i = 0; i < NO_PHILOSOPHERS; i++){
		if(pthread_join(philosophers[i], NULL) != 0){
			printf("Error in function pthread_join()!\n");
			exit(EXIT_FAILURE);
		}
	}

	exit(EXIT_SUCCESS);
}

void* threadFunction(void* arg){

	int myID = *(int*)arg;
	int myLeftFork = myID;
	int myRightFork = (myID + 1) % 5;

	while(1){
		// Think for some time
		printf("Philosopher:\t%d is currently thinking about the meaninglessness of human life.\n", myID);
		sleep( (double)(rand() % 30 ) / 10 );

		// Ask the permission of waiter
		if(pthread_mutex_lock(&waiter) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		// Wait for my forks to become available
		while(whoHasFork[myLeftFork] != -1 && whoHasFork[myRightFork]) sleep(0.1);

		// Pick forks up and eat for some time
		pickForkUp(myID, myLeftFork);
		pickForkUp(myID, myRightFork);

		// Tell the waiter I'm done picking up my forks
		if(pthread_mutex_unlock(&waiter) != 0){
			printf("Error in function pthread_mutex_lock()!\n");
			exit(EXIT_FAILURE);
		}

		// Eat for some time
		printf("Philosopher:\t%d is currently eating.\n", myID);
		sleep( (double)(rand() % 30 ) / 10 );

		// Put down forks
		putForkDown(myID, myLeftFork);
		putForkDown(myID, myRightFork);

	}

	return NULL;
}

void pickForkUp(int philosopherID, int forkID){
	whoHasFork[forkID] = philosopherID;
}
void putForkDown(int philosopherID, int forkID){
	whoHasFork[forkID] = -1;
}


// Handler function for exiting at SIGSTP
void quitHandler(){

}

// At exit function
void exitHandler(){

}