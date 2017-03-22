#define _POSIX_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

// Variables for counting signals
int counter = 0;
int finished = 0;
int respond = 0;
pid_t pid;

// Variables for counting signals
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);

int main(int argc, char **argv) {

	// Variables for counting signals
	if(argc != 2){
		printf("Wrong no. of arguments!\n");
		return 1;
	}

	// Sigaction structs for handiling signals
	struct sigaction *sigusr1_act = malloc(sizeof(struct sigaction));
	struct sigaction *sigusr2_act = malloc(sizeof(struct sigaction));

	// Set funs to handle signals
	sigusr1_act->sa_handler = sigusr1_handler;
	sigusr2_act->sa_handler = sigusr2_handler;

	// Ingore all other signals
	if(sigfillset(&(sigusr1_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}
	if(sigfillset(&(sigusr2_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}

	// Set desired actions
	if(sigaction(SIGUSR1, sigusr1_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}
	if(sigaction(SIGUSR2, sigusr2_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}

	// Read number of iterations
	int loop_length = atoi(argv[1]);

	sigset_t mask;

	// Add user defined signals 1 and 2 to mask
	if(sigaddset(&mask, SIGUSR1) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigaddset(&mask, SIGUSR2) != 0){
		perror("in function sigaddset()");
		exit(1);
	}

	// Block the signals
	if(sigprocmask(SIG_BLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	// Fork a new process and execute the child program
	pid = fork();
	if(pid == 0){
		execl("./child", "child", NULL);
	}

	// Unblock signals 1 and 2
	if(sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	// Sent the signals desired number of times
	int i;
	for(i = 0; i < loop_length; i++){
		// Send signal
		if(kill(pid, SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}

		// Wait till we receive a confirmation signal
		while(counter == 0) pause();
		// Reset the counter
		counter = 0;
	}

	printf("Parent sent:\t%d SIGUSR1 signals to child\n", i);
	printf("Parent sent:\t 1 SIGUSR2 signal to child\n\n");

	// Send end of transmission signal
	kill(pid, SIGUSR2);

	// Wait for end of transmission signal
	respond = 1;
	while(!finished) pause();

	// We dont need to respond to anything now
	respond = 0;

	printf("\n%10d signals SIGUSR1 were sent to child\n%10d signals SIGUSR1 were received by parent\n", i, counter);

	// Check if we intercepted all the signals
	if(loop_length == counter) printf("\nEverything went fine.\n\n");
	else printf("\%d signals were lost.\n\n", loop_length - counter);

	// Free the structures
	free(sigusr1_act);
	free(sigusr2_act);

	return 0;
}

// Handler functions
void sigusr1_handler(int sig){
	counter++;
	if(respond)
		if(kill(pid, SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}
}

// Simply wait for sigusr2 and set a variable
void sigusr2_handler(int sig){
	finished = 1;
}

