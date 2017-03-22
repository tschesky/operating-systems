#define _XOPEN_SOURCE
#define _BSD_SOURCE
#define MAX_MESSAGE_LENGTH 256
#define MAX_CLIENT_NAME 32
#define MAX_CLIENTS 32


#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>

// Struct for keeping list of clients
struct client{
	mqd_t queue;
	mqd_t queueOut;
	pid_t pid;
	char name[MAX_CLIENT_NAME];
	int active;
};

// Adding Client, Removing Client and resending the message to all connected clients
int addClient(char* msg, int pid);
void removeClient(pid_t pid);
void sendOver(char* msg, struct client* client);

// Handler for stopping the server, quitting the program
void quitHandler();
void exitHandler();
void notificationHandler();
void notificationHandler2();

// Clear inactive clients
void defragmentClients();

// List of currently connected cliets and  the current number of them
struct client clientList[MAX_CLIENTS];
int noClients = 0;
mqd_t queue;
struct sigevent notify;
struct sigevent notify2;

int main(int argc, char* argv[]){

	// Check no. of arguments
	if(argc != 1){
		printf("Wrong number of arguments, server takes no parameters!\n");
		return 1;
	}

	struct mq_attr attr;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20;
	attr.mq_flags = O_NONBLOCK;

	queue = mq_open("/serverQ", O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &attr);
	if(queue < 0){
		printf("Error while creating server queue in function mq_open()!\n");
		return 1;
	}
	
	notify.sigev_notify = SIGEV_SIGNAL;
	notify.sigev_signo = SIGRTMAX;

	notify2.sigev_notify = SIGEV_SIGNAL;
	notify2.sigev_signo = (SIGRTMAX-1);

	if(signal(SIGRTMAX, notificationHandler) == SIG_ERR){
		printf("Error in signal()!\n");
		return 1;
	}

	if(mq_notify(queue, &notify) == -1){
		printf("Error in mq_notify(), main()!\n");
		return 1;
	}

	// Set the handler and atExit
	signal(SIGTSTP, quitHandler);
	atexit(exitHandler);

	printf("-----------Server has been created-----------\n");

	while(1) pause();

	return 0;
}

void notificationHandler(){
		char* message = (char*)calloc(MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, sizeof(char));
		
		unsigned int client_pid;
		if(mq_receive(queue, message, MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, &client_pid) < 0){
			if(errno != EAGAIN){ 
				printf("Error in msgrcv() from server queue!\n");
			}
		}else{
			if( addClient(message, client_pid) != 0) printf("Error adding the client!\n");	
		}

		if(mq_notify(queue, &notify) == -1){
			printf("Error in mq_notify()!: %d\n", errno);
		}
}

void notificationHandler2(){
	char* message = (char*)calloc(MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, sizeof(char));
	unsigned int client_pid;

	for(int i = 0; i < noClients; i++){
			if( mq_receive(clientList[i].queue, message, MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, &client_pid) < 0){
				if(errno != EAGAIN){ 
					printf("Error in mq_receive from client queue: %d\n", errno);
				}
			}else{

				if( strcmp(message, "exit") == 0 ){
					clientList[i].active = 0;
					printf(">>\t%s has disconeccted!\n", clientList[i].name);
					defragmentClients();
				} else {
					sendOver(message, &clientList[i]);
				}
			}
			fflush(stdout);

			if(mq_notify(clientList[i].queue, NULL) == -1){
				printf("Error in mq_notify(), main()!\n");
			}
			if(mq_notify(clientList[i].queue, &notify2) == -1){
				printf("Error in mq_notify(), main()!\n");
			}
		}

	
}

// Adding newly created client
int addClient(char msg[], int pid){

	printf("Pid: %d\n", pid);
	printf("Msh: %s\n", msg);

	clientList[noClients].pid = (pid_t)pid;
	strcpy(clientList[noClients].name, msg);
	clientList[noClients].active = 1;

	char* buff = (char*)calloc(MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, sizeof(char));
	sprintf(buff, "/%s%d", msg, pid);
	printf("Name: %s\n", buff);
	clientList[noClients].queue = mq_open(buff, O_RDONLY);
	if(clientList[noClients].queue < 0){
		printf("Error while creating client input queue in function mq_open()!: %d\n", errno);
		return 1;
	}

	sprintf(buff, "/%s%dOut", msg, pid);

	clientList[noClients].queueOut = mq_open(buff, O_WRONLY);
	if(clientList[noClients].queueOut < 0){
		printf("Error while creating client output queue in function mq_open()!: %d\n", errno);
		return 1;
	}


	if(signal(SIGRTMAX-1, notificationHandler2) == SIG_ERR){
		printf("Error in signal()!\n");
		return 1;
	}

	if(mq_notify(clientList[noClients].queue, &notify2) == -1){
		printf("Error in mq_notify(), in AddClient()!\n");
		return 1;
	}

	noClients++;

	if(kill((pid_t)pid, SIGUSR1) == -1){
		printf("Error in kill()!\n");
	}

	printf("%s has connected!\n", msg);
	fflush(stdout);	
	return 0;
}

void sendOver(char*msg, struct client* client){
	
	char* output = (char*)calloc(MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, sizeof(char));
	char timeBuff[20];
	time_t tmpTime = time(NULL);
	struct tm* timeinfo = localtime(&tmpTime);
	strftime(timeBuff, sizeof(timeBuff), "%R", timeinfo);
	sprintf(output, "%s[%s] wrote: %s" , client->name, timeBuff, msg);

	printf(">>\t%s\n", output);

	// Sending the message
	for(int i = 0; i < noClients; i++){
		if(clientList[i].pid != client->pid){
			if( mq_send(clientList[i].queueOut, output, MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20, (unsigned int)getpid() ) < 0){
				printf("Error in mq_send()!\n");
			}

			if(mq_notify(clientList[i].queue, NULL) == -1){
				printf("Error in mq_notify(), main()!\n");
			}
			if(mq_notify(clientList[i].queue, &notify2) == -1){
				printf("Error in mq_notify(), main()!\n");
			}
		}
	}
}

void quitHandler(){
	exit(EXIT_SUCCESS);
}

void exitHandler(){
	printf("Stopping the server, closing all connected clients!\n");
	if( mq_unlink("/server_queue") < 0){
		printf("Error in mq_unlink() while exitting progam!\n");
	}

	for(int i = 0; i < noClients; i++){
		kill(clientList[i].pid, SIGUSR2);
	}
}

void defragmentClients(){
	for(int i = 0; i < noClients; i++){
		if(clientList[i].active == 0){
			if(i == noClients-1) noClients --;
			else if(i == noClients-2){
				clientList[i] = clientList[noClients-1];
				noClients--;
			}
			else{
				for(int j = i; j < noClients-1; j++){
					clientList[i] = clientList[i+1];
				}
				noClients--;
			}
		}
	}
}


