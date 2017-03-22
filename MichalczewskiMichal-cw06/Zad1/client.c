#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define MAX_MESSAGE_LENGTH 256
#define MAX_CLIENT_NAME 32

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

// Message struct, client sends his pind, desired msg and the time it has been sent
struct msg{
	int type;
	char text[MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20];
};

// Reading user input
int readUserInput(struct msg* buf);

// Reading signal from server
void connectSignal();
void closeSignal();
void exitHandler();

// Are we connected?
int connected = 0;
pid_t pid;

int sender(int myQueue);
int receiver(int myQueue);

int main(int argc, char* argv[]){

	struct msg buffer;
	char * name = (char*)malloc(sizeof(char));
	
	// Check no. of arguments
	if(argc != 2){
		printf("Wrong number of arguments, you need to specify client's name!\n");
		return 1;
	}
	if(strlen(argv[1]) > 32 ){
		printf("Sorry, client's name can't be longer than 32 characters!\n");
	}

	// Register handler function - server validated the connection with SIGUSR1
	signal(SIGUSR1, connectSignal);
	signal(SIGUSR2, closeSignal);

	atexit(exitHandler);

	// Generate the server key, later to use it for msg processing
	key_t key = ftok(".", 1);
	key_t myKey = ftok(".", getpid());
	key_t myKeyIn = ftok(".", getpid() + 1000);

	// Quit if error occured
	if(key == -1){
		printf("Error in ftok() function, exiting the program!\n");
		return 1;
	}

	// Obtain the queue, create it if non-existent
	int queue = msgget(key, IPC_CREAT | 0600);
	int myQueue = msgget(myKey, IPC_CREAT | 0600);
	int myQueueIn = msgget(myKeyIn, IPC_CREAT | 0600);

	// Check for errors
	if(queue < 0){
		printf("Error while creating the queue!\n");
		return 1;
	}

	printf(">> \tConnecting to the server....\n");

	// Send the name to connect to server
	buffer.type = getpid();
	strcpy(name, argv[1]);
	strcpy(buffer.text, argv[1]);

	// Sending the message
	if( msgsnd(queue, &buffer, sizeof(buffer), 0) < 0){
			printf("Error in msgsnd()!\n");
	}

	// Waiting for validation
	while(connected == 0) pause();

	printf(">> \tSuccesfully connected! Type your messages below. To quit type 'exit'.\n");

	pid = fork();

	if(pid > 0){
		sender(myQueue);
	} else if( pid == 0){
		receiver(myQueueIn);
	} else {
		printf("Some rror while forking, sorr!\n");
		return 1;
	}
	
	return 0;
}

int readUserInput(struct msg* buf){

	char* tmp = (char*)malloc(MAX_MESSAGE_LENGTH);
	int c, i = 0;


	while(1){
		c = getchar();

		if(c == '\n'){
			tmp[i] = 0;
			break;
		}

		tmp[i] = c;
		if(i == MAX_MESSAGE_LENGTH -1){
			return 1;
		}
		i++;
	}

	strcpy(buf->text, tmp);
	buf->type = getpid();

	return 0;

}

void connectSignal(){
	connected = 1;
}

int sender(int myQueue){

	// Buffer for reading and sending msgs, variable for clearing the unwanted input
	struct msg buffer;
	int buffClear;

	// Main loop
	while(1){
		printf(">> \t");
		if( readUserInput(&buffer) == 1){
			printf("The message is too long and can't be processed!\n");
			while( (buffClear = getchar()) != '\n' && buffClear != EOF);
			continue;
		}
			
		if( msgsnd(myQueue, &buffer, sizeof(buffer), 0) < 0){
			printf("Error in msgsnd()!\n");
		}		

		if(strcmp(buffer.text, "exit") == 0) exit(EXIT_SUCCESS);	

		// printf("%d\t%s\t%s\n", buffer.pid, buffer.text, ctime(&(buffer.timeStamp)));
	}

}

int receiver(int myQueue){

	struct msg buffer;

	while(1){
		if( msgrcv(myQueue, &buffer, sizeof(buffer), 0, IPC_NOWAIT ) < 0){
			if(errno != EAGAIN && errno != ENOMSG){ 
				printf("Error in msgrcv()! from client queue in client's receiver process!\n");
			}
		}else{
			
			printf("%s\n",buffer.text);
			printf(">>\t");
			fflush(stdout);
		}
	}

}

void closeSignal(){
	exit(EXIT_SUCCESS);
}

void exitHandler(){
	printf("Killing the child process and terminating client!\n");
	kill(pid, SIGTERM);
}