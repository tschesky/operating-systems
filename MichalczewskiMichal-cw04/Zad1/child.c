#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

// Counter and variable to determine if we are finished
int counter = 0;
int finished = 0;

// Handler functions
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);

int main(int argc, char **argv) {

	// Signal actions
	struct sigaction *sigusr1_act = malloc(sizeof(struct sigaction));
	struct sigaction *sigusr2_act = malloc(sizeof(struct sigaction));

	// Set handlers for SIGUSR1 and SIGUSR2
	sigusr1_act->sa_handler = sigusr1_handler;
	sigusr2_act->sa_handler = sigusr2_handler;

	// Block all signals
	if(sigfillset(&(sigusr1_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}
	if(sigfillset(&(sigusr2_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}

	// Set actions
	if(sigaction(SIGUSR1, sigusr1_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}
	if(sigaction(SIGUSR2, sigusr2_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}

	// Unblock SIGUSR1 and SIGUSR2
	sigset_t mask;
	if(sigaddset(&mask, SIGUSR1) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigaddset(&mask, SIGUSR2) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}


	// Wait for the SIGUSR2 end-of-transmission signal
	while(!finished)
		pause();


	// Get our parent id
	pid_t ppid = getppid();

	// Send as much signals as we've received
	int i;
	for(i = 0; i < counter; i++)
		if(kill(ppid, SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}

	// Printf info
	printf("Child sent:\t %d SIGUSR1 signal to parent\n", i);
	printf("Child sent:\t 1 SIGUSR2 signal to parent\n\n");

	// Send the SIGUSR2, we're done
	if(kill(ppid, SIGUSR2) != 0){
		perror("in function kill()");
		exit(1);
	}

	// Free the structs
	free(sigusr1_act);
	free(sigusr2_act);
	return 0;
}

void sigusr1_handler(int sig){
	counter++;
}

void sigusr2_handler(int sig){
	finished = 1;
}
