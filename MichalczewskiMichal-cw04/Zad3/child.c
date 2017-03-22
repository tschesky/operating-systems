#define _POSIX_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

// Variables for counting signals
int counter = 0;
int finished = 0;

// Variables for counting signals
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);

int main(int argc, char **argv) {

	// Sigaction structs for handiling signals
	struct sigaction *sigusr1_act = malloc(sizeof(struct sigaction));
	struct sigaction *sigusr2_act = malloc(sizeof(struct sigaction));

	// Set funs to handle signals
	sigusr1_act->sa_handler = sigusr1_handler;
	sigusr2_act->sa_handler = sigusr2_handler;

	// Set actions in structs
	if(sigfillset(&(sigusr1_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}
	if(sigfillset(&(sigusr2_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}

	// Set signals - this time as real-time signals
	if(sigaction(SIGRTMIN+SIGUSR1, sigusr1_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}
	if(sigaction(SIGRTMIN+SIGUSR2, sigusr2_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}

	// Unblock signals 1 and 2
	sigset_t mask;
	if(sigaddset(&mask, SIGRTMIN+SIGUSR1) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigaddset(&mask, SIGRTMIN+SIGUSR2) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	// Wait for end of transmission from parent process
	while(!finished) pause();

	// Get parent id
	pid_t ppid = getppid();

	// Send back as many signals as we received
	int i;
	for(i = 0; i < counter; i++)
		if(kill(ppid, SIGRTMIN+SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}
	
	printf("Child sent:\t%d SIGUSR1 signals to parent\n", i);
	printf("Child sent:\t 1 SIGUSR2 signal to parent\n\n");

	// Send back end of transmission to parent process
	if(kill(ppid, SIGRTMIN+SIGUSR2) != 0){
		perror("in function kill()");
		exit(1);
	}

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
