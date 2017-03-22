#define _POSIX_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

// Counter of sigs
int counter = 0;
int finished = 0;

// Handlers for SIGUSR1 and SIGUSR2
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);

int main(int argc, char **argv) {

	// Check no of argumets
	if(argc != 2){
		printf("Wrong no of arguments!\n");
		return 1;
	}

	// Structs for setting sigactions
	struct sigaction *sigusr1_act = malloc(sizeof(struct sigaction));
	struct sigaction *sigusr2_act = malloc(sizeof(struct sigaction));

	// Set the handlers for both signals
	sigusr1_act->sa_handler = sigusr1_handler;
	sigusr2_act->sa_handler = sigusr2_handler;

	// Ignore all other signals
	if(sigfillset(&(sigusr1_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}
	if(sigfillset(&(sigusr2_act->sa_mask)) != 0){
		perror("in function sigfillset()");
		exit(1);
	}

	// Set actions to signals
	if(sigaction(SIGUSR1, sigusr1_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}
	if(sigaction(SIGUSR2, sigusr2_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}

	// No. of iterations 
	int loop_length = atoi(argv[1]);
	sigset_t mask;

	// Add SIGUSR1 and SIGUSER2 to mask
	if(sigaddset(&mask, SIGUSR1) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigaddset(&mask, SIGUSR2) != 0){
		perror("in function sigaddset()");
		exit(1);
	}

	// Proccess the mask, blocking SIGUSR1 and SIGUSR2
	if(sigprocmask(SIG_BLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	// Forking and starting the child proccess
	pid_t pid = fork();
	if(pid == 0){
		execl("./child", "child", NULL);
	}

	// Unblock SIGUSR1 and SIGUSR2
	if(sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	printf("Parent sent:\t%d SIGUSR1 signals to child\n", loop_length);
	
	// Send n SIGUSR1 to child
	for(int i = 0; i < loop_length; i++){
		if(kill(pid, SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}
	}
	// Send SIGUSR2 as end of transmission
	if(kill(pid, SIGUSR2) != 0){
		perror("in function kill()");
		exit(1);
	}
	printf("Parent sent:\t 1 SIGUSR2 signal to child\n\n");

	// Handler for SIGUSR2 is set so we wait till child proccess finishes and sends us the isgnal
	while(!finished) pause();

	printf("\n%10d signals SIGUSR1 were sent to child\n%10d signals SIGUSR1 were received by parent\n", loop_length, counter);
	
	if(loop_length == counter) printf("\nEverything went fine.\n\n");
	else printf("%10d signals were lost.\n\n", loop_length - counter);

	// Free the structs
	free(sigusr1_act);
	free(sigusr2_act);

	return 0;
}

// Handlers, when we catch SIGUSER1 we increment
void sigusr1_handler(int sig){
	counter++;
}

// When SIGUSR2 is caught, we finish
void sigusr2_handler(int sig){
	finished = 1;
}

