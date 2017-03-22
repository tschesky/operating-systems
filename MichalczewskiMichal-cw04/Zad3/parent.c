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

	// Set desired actions, this time we send them as real-time signals - we do it by using the following macro
	// Thanks to that one that more signal of given type can be queued
	if(sigaction(SIGRTMIN+SIGUSR1, sigusr1_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}
	if(sigaction(SIGRTMIN+SIGUSR2, sigusr2_act, NULL) != 0){
		perror("in function sigaction()");
		exit(1);
	}

	// Read no. of iterations
	int loop_length = atoi(argv[1]);

	// Block signals no. 1 and 2
	sigset_t mask;
	if(sigaddset(&mask, SIGRTMIN+SIGUSR1) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigaddset(&mask, SIGRTMIN+SIGUSR2) != 0){
		perror("in function sigaddset()");
		exit(1);
	}
	if(sigprocmask(SIG_BLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	// Fork new proccess and exec child program
	pid_t pid = fork();
	if(pid == 0){
		execl("./child", "child", NULL);
	}

	// Unblock our signals
	if(sigprocmask(SIG_UNBLOCK, &mask, NULL) != 0){
		perror("in function sigprocmask()");
		exit(1);
	}

	printf("Parent sent:\t%d SIGUSR1 signals to child\n", loop_length);
	
	// Send loop_lenght singals to child
	int i;
	for(i = 0; i < loop_length; i++){
		if(kill(pid, SIGRTMIN+SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}
	}

	// Send end of transmission signal
	if(kill(pid, SIGRTMIN+SIGUSR2) != 0){
		perror("in function kill()");
		exit(1);
	}

	printf("Parent sent:\t 1 SIGUSR2 signal to child\n\n");

	// Wait for info from child process
	while(!finished) pause();

	printf("\n%10d signals SIGUSR1 were sent to child\n%10d signals SIGUSR1 were received by parent\n", loop_length, counter);
	
	if(loop_length == counter) printf("\nEverything went fine.\n\n");
	else printf("%10d signals were lost.\n\n", loop_length - counter);

	// Free structs
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

