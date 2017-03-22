#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

// Counting signals
int counter = 0;
int finished = 0;
int respond = 0;
pid_t ppid;

// Handerl functins
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);

int main(int argc, char **argv) {

	// Stucts for setting sig actions
	struct sigaction *sigusr1_act = malloc(sizeof(struct sigaction));
	struct sigaction *sigusr2_act = malloc(sizeof(struct sigaction));

	// Setting handlers
	sigusr1_act->sa_handler = sigusr1_handler;
	sigusr2_act->sa_handler = sigusr2_handler;

	// Ingore all oher signals
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

	// Get parent id
	ppid = getppid();

	// Enable responding
	respond = 1;

	// Set usr defined signals as ones we pass through now
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

	// Wait for end of transmission
	while(!finished) pause();

	// No need to respond now
	respond = 0;

	// We send back as many signals as we received
	int i, loop_len = counter;
	for(i = 0; i < loop_len; i++){

		counter = 0;
		if(kill(ppid, SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}

		while(counter == 0) pause();
	}

	// Print output
	printf("Child sent:\t%d SIGUSR1 signals to parent\n", i);
	printf("Child sent:\t 1 SIGUSR2 signal to parent\n\n");

	// Send end of transmission signal
	if(kill(ppid, SIGUSR2) != 0){
		perror("in function kill()");
		exit(1);
	}

	// Free buffers
	free(sigusr1_act);
	free(sigusr2_act);
	return 0;
}

// FUnctions same as in parent
void sigusr1_handler(int sig){
	counter++;
	if(respond)
		if(kill(ppid, SIGUSR1) != 0){
			perror("in function kill()");
			exit(1);
		}
}

void sigusr2_handler(int sig){
	finished = 1;
}
