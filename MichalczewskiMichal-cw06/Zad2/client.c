#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define MAX_MESSAGE_LENGTH 256
#define MAX_CLIENT_NAME 32
#define MAX_CLIENTS 14

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
#include <mqueue.h>

// Reading user input
int readUserInput(char* text);

// Reading signal from server
void connectSignal();
void closeSignal();
void exitHandler();

// Are we connected?
int connected = 0;
pid_t pid;

mqd_t queue;
mqd_t myQueue;
mqd_t myQueueOut;
struct sigevent notify2;
char * name;

int sender();
void receiver();

int main(int argc, char* argv[]){

	name = (char*)malloc(sizeof(char));
	
	// Check no. of arguments
	if(argc != 2){
		printf("Wrong number of arguments, you need to specify client's name!\n");
		return 1;
	}
	if(strlen(argv[1]) > 32 ){
		printf("Sorry, client's name can't be longer than 32 characters!\n");
	}

	// Send the name to connect to server
	strcpy(name, argv[1]);

	// Register handler function - server validated the connection with SIGUSR1
	signal(SIGUSR1, connectSignal);
	signal(SIGUSR2, closeSignal);

	atexit(exitHandler);

	struct mq_attr attr;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20;
	attr.mq_flags = O_NONBLOCK;

	queue = mq_open("/serverQ", O_WRONLY);

	char buff[MAX_MESSAGE_LENGTH + 15];
	sprintf(buff, "/%s%d", name, getpid());
	printf("%s\n", buff);
	myQueue = mq_open(buff, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &attr);

	sprintf(buff, "/%s%dOut", name, getpid());
	myQueueOut = mq_open(buff, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &attr);

	if(queue < 0){
		printf("Error while opening server queue in function mq_open()!: %d\n", errno);
		return 1;
	}

	if(myQueue < 0){
		printf("Error while opening client output queue in function mq_open()!: %d\n", errno);
		return 1;
	}

	if(myQueueOut < 0){
		printf("Error while opening client input queue in function mq_open()!: %d\n", errno);
		return 1;
	}

	printf("%d", getpid());
	// Sending the message
	if( mq_send(queue, name, strlen(name), (unsigned int)getpid() ) < 0){
			printf("Error in msgsnd()!\n");
	}

	printf(">> \tConnecting to the server....\n");

	// Waiting for validation
	while(connected == 0) pause();

	printf(">> \tSuccesfully connected! Type your messages below. To quit type 'exit'.\n");

	pid = fork();

	if(pid > 0){
		sender();
	} else if( pid == 0){

		notify2.sigev_notify = SIGEV_SIGNAL;
		notify2.sigev_signo = SIGRTMAX;
		if(signal(SIGRTMAX, receiver) == SIG_ERR){
			printf("Error in signal()!\n");
			return 1;
		}

		if(mq_notify(myQueueOut, &notify2) == -1){
			printf("Error in mq_notify()!: %d\n", errno);
			return 1;
		}

		while(1) pause();

	} else {
		printf("Some error while forking, sorr!\n");
		return 1;
	}
	
	return 0;
}

int readUserInput(char* text){

	int c, i = 0;
	while(1){
		c = getchar();

		if(c == '\n'){
			text[i] = 0;
			break;
		}

		text[i] = c;
		if(i == MAX_MESSAGE_LENGTH -1){
			return 1;
		}
		i++;
	}
	return 0;

}

void connectSignal(){
	connected = 1;
}

int sender(){

	// Buffer for reading and sending msgs, variable for clearing the unwanted input
	int buffClear;
	char *tmp = (char*)calloc(MAX_MESSAGE_LENGTH, sizeof(char));
	// Main loop
	while(1){
		printf(">> \t");
		if( readUserInput(tmp) == 1){
			printf("The message is too long and can't be processed!\n");
			while( (buffClear = getchar()) != '\n' && buffClear != EOF);
			continue;
		}
			
		printf("Tmp: %s\n", tmp);
		if( mq_send(myQueue, tmp, MAX_MESSAGE_LENGTH, (unsigned int)getpid() ) < 0){
			printf("Error in mq_send()!: %d\n", errno);
		}		

		if(strcmp(tmp, "exit") == 0) exit(EXIT_SUCCESS);	

		// printf("%d\t%s\t%s\n", buffer.pid, buffer.text, ctime(&(buffer.timeStamp)));
	}

}

void receiver(){

		char* tmp = (char*)calloc(MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, sizeof(char));
		unsigned int client_pid;

		if(mq_receive(myQueueOut, tmp, MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, &client_pid) < 0){
			if(errno != EAGAIN && errno != ENOMSG){ 
				printf("Error in mq_receive()! from client queue in client's receiver process!: %d\n", errno);
			}
		}else{
			
			printf("%s\n",tmp);
			printf(">>\t");
			fflush(stdout);
		}

		if(mq_notify(myQueueOut, &notify2) == -1){
			printf("Error in mq_notify()!: %d\n", errno);
		}
}

void closeSignal(){
	exit(EXIT_SUCCESS);
}

void exitHandler(){
	printf("Killing the child process and terminating client!\n");
	
	char buff[MAX_MESSAGE_LENGTH + 15];

	sprintf(buff, "/%s%d", name, getpid());
	if( mq_unlink(buff) < 0){
		printf("Error in mq_unlink() while exitting progam!\n");
	}

	sprintf(buff, "/%s%dOut", name, getpid());
	if( mq_unlink(buff) < 0){
		printf("Error in mq_unlink() while exitting progam!\n");
	}

	kill(pid, SIGTERM);
}