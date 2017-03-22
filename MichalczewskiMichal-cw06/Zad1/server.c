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

// Message struct, client sends his pind, desired msg and the time it has been sent
struct msg{
	int type;
	char text[MAX_MESSAGE_LENGTH + MAX_CLIENT_NAME + 20];
};

// Struct for keeping list of clients
struct client{
	int queue;
	int queueOut;
	pid_t pid;
	char name[MAX_CLIENT_NAME];
	int active;
};

// Adding Client, Removing Client and resending the message to all connected clients
void addClient(struct msg* data);
void removeClient(pid_t pid);
void sendOver(struct msg* data, struct client* client);

// Handler for stopping the server, quitting the program
void quitHandler();
void exitHandler();

// Clear inactive clients
void defragmentClients();

// List of currently connected cliets and  the current number of them
struct client clientList[MAX_CLIENTS];
int noClients = 0;
int queue;

int main(int argc, char* argv[]){

	// Buffer for reading from two kinds of queues - the one made by server and the one made by clients
	struct msg buffer;
	struct msg buffer2;

	// Check no. of arguments
	if(argc != 1){
		printf("Wrong number of arguments, server takes no parameters!\n");
		return 1;
	}

	// Generate the server key, later to use it for msg processing
	key_t key = ftok(".", 1);

	// Quit if error occured
	if(key == -1){
		printf("Error in ftok() function, exiting the program!\n");
		return 1;
	}

	// Obtain the queue, create it if non-existent, allow child processes to acces it
	queue = msgget(key, IPC_CREAT | 0600);

	// Check for errors
	if(queue < 0){
		printf("Error while creating the queue!\n");
		return 1;
	}

	// Set the handler and atExit
	signal(SIGTSTP, quitHandler);
	atexit(exitHandler);

	printf("-----------Server has been created-----------\n");

	// MAIN LOOP
	while(1){
		if( msgrcv(queue, &buffer, sizeof(buffer), 0, IPC_NOWAIT ) < 0){
			if(errno != EAGAIN && errno != ENOMSG){ 
				printf("Error in msgrcv() from server queue!\n");
			}
		}else{

			addClient(&buffer);	
		}

		for(int i = 0; i < noClients; i++){

			if( msgrcv(clientList[i].queue, &buffer2, sizeof(buffer), 0, IPC_NOWAIT ) < 0){
				if(errno != EAGAIN && errno != ENOMSG){ 
					printf("Error in msgrcv()! from client queue\n");
				}
			}else{

				if( strcmp(buffer2.text, "exit") == 0 ){
					clientList[i].active = 0;
					printf(">>\t%s has disconeccted!\n", clientList[i].name);
					defragmentClients();
				} else {
					sendOver(&buffer2, &clientList[i]);
				}
			}
		}

	}	

	return 0;
}

// Adding newly created client
void addClient(struct msg* data){

	clientList[noClients].pid = (pid_t)data->type;
	strcpy(clientList[noClients].name, data->text);
	clientList[noClients].active = 1;

	key_t tmp = ftok(".", data->type);
	clientList[noClients].queue = msgget(tmp, IPC_CREAT);

	key_t tmp2 = ftok(".", (data->type + 1000));
	clientList[noClients].queueOut = msgget(tmp2, IPC_CREAT);

	noClients++;

	if(kill((pid_t)data->type, SIGUSR1) == -1){
		printf("Error in kill()!\n");
	}

	printf("%s has connected!\n", data->text);
	fflush(stdout);	

}

void sendOver(struct msg* data, struct client* client){
	
	char output[MAX_MESSAGE_LENGTH + MAX_MESSAGE_LENGTH + 20];
	char timeBuff[20];
	time_t tmpTime = time(NULL);
	struct tm* timeinfo = localtime(&tmpTime);
	strftime(timeBuff, sizeof(timeBuff), "%R", timeinfo);
	sprintf(output, "%s[%s] wrote: %s" , client->name, timeBuff, data->text);

	struct msg tmpStruct;
	tmpStruct.type = 1;
	strcpy(tmpStruct.text, output);

	printf(">>\t%s\n", output);

	// Sending the message
	for(int i = 0; i < noClients; i++){
		if(clientList[i].pid != client->pid){
			if( msgsnd(clientList[i].queueOut, &tmpStruct, sizeof(tmpStruct), 0) < 0){
				printf("Error in msgsnd()!\n");
			}
		}
	}
}

void quitHandler(){
	exit(EXIT_SUCCESS);
}

void exitHandler(){
	printf("Stopping the server, closing all connected clients!\n");
	if( msgctl(queue, IPC_RMID, (struct msqid_ds *)NULL) < 0){
		printf("Error in msgctl() while exitting progam!\n");
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


