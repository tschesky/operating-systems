#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include "data.h"
#include <errno.h>


#define CLIENTS_MAX 100
#define INACTIVITY_MAX 9

fd_set fds;

struct clientProfile clients[CLIENTS_MAX];

int connected = 0;

int socketInternet;

int socketUnixLocal;

struct sockaddr_un selfUnixLocal;



int checkIfRegistered(char *nick);

int checkIfActive(char *nick);

int getNumber(char *nick);



void deactivateUsers();

void sendToAll(struct msg msg1);

void setLastActivityTime(int index);

void internetHandler(struct msg msg1, struct sockaddr_in);

void unixHandler(struct msg msg1);



void intHandler(int signo) {
    
    printf("Exiting\n");
    
    close(socketInternet);
    
    close(socketUnixLocal);
    
    exit(EXIT_SUCCESS);
}



void atexit_function(void) {
    
    intHandler(1);
    
    exit(EXIT_SUCCESS);
}



int main(int argc, char *argv[]) {
    
    atexit(atexit_function);

    if (signal(SIGINT, intHandler) == SIG_ERR) {
        perror("signal():");
        return 1;
    }


    for (int i = 0; i < CLIENTS_MAX; i++) {
        memset(&clients[i], '0', sizeof(clients[i]));
        clients[i].lastActivity = -1;
    }


    char buf[MSSG_LNGTH];
    
    struct sockaddr_in selfInternet;
    
    struct sockaddr_in tmp;

    struct timeval time_restriction;
    
    time_restriction.tv_sec = 10;
    
    time_restriction.tv_usec = 0;
    
    int len = sizeof(struct sockaddr_in);
    
    
    if (argc < 3) {
        printf("Bad arguments\n");
        return 1;
    }


    //**************************** Local Internet Socket *********************************************

    int port = atoi(argv[1]);


    if ((socketInternet = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        return 1;
    }

    
    memset((char *) &selfInternet, 0, sizeof(struct sockaddr_in));
    
    selfInternet.sin_family = AF_INET;
    
    selfInternet.sin_port = htons(port);
    
    selfInternet.sin_addr.s_addr = htonl(INADDR_ANY);
    
    
    
    if (bind(socketInternet, (struct sockaddr *) &selfInternet, sizeof(selfInternet)) == -1) {
        perror("bind()");
        return 1;
    }
    
    

    //************************************* Local Unix Socket *******************************************************
    
    if ((socketUnixLocal = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("socket():");
        return 1;
    }
    
    
    selfUnixLocal.sun_family = AF_UNIX;
    
    strcpy(selfUnixLocal.sun_path, argv[2]);
    
    
    if ( bind(socketUnixLocal, (struct sockaddr *) &selfUnixLocal, sizeof(struct sockaddr_un)) < 0) {
        perror("bind():");
        return 1;
    }
    

    //*********************************** File descriptors set *****************************************
    
    FD_ZERO(&fds);
    
    FD_SET(socketInternet, &fds);
    
    FD_SET(socketUnixLocal, &fds);
    
    int max = socketInternet > socketUnixLocal ? socketInternet + 1 : socketUnixLocal + 1;

    struct msg msg1;

    memset(msg1.buf, '\0', sizeof(msg1.buf));

    memset(msg1.nick, '\0', sizeof(msg1.nick));



    //************************************* Main loop ************************************************
    
    while (1) {
    
        FD_SET(socketInternet, &fds);
    
        FD_SET(socketUnixLocal, &fds);
    
        select(max, &fds, NULL, NULL, &time_restriction);
    
    
        if (FD_ISSET(socketInternet, &fds)) {
    
            int n_int = (int)recvfrom(socketInternet, (void *) &msg1, sizeof(struct msg), MSG_DONTWAIT,
                                 (struct sockaddr *) &tmp, &len);
    
            if (n_int < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom():");
                exit(EXIT_FAILURE);
            
                
            } else if (n_int > 0)
                internetHandler(msg1,tmp);
        }

        if (FD_ISSET(socketUnixLocal, &fds)) {
            
            int n_local = (int) recvfrom(socketUnixLocal, (void *) &msg1, sizeof(struct msg), MSG_DONTWAIT, NULL, 0);
            
            if (n_local < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("recvfrom():");
                exit(EXIT_FAILURE);
         
            } else if (n_local > 0) 
                unixHandler(msg1);
        }
        
        FD_ZERO(&fds);
        deactivateUsers();
        time_restriction.tv_sec = 10;
        time_restriction.tv_usec = 0;
    
        
    }
}



void unixHandler(struct msg msg1) {
  
    if (checkIfRegistered(msg1.nick) == 0) {
        
        printf("CLient %s not registered\n", msg1.nick);
        
        if (strncmp(msg1.buf, "_R", 2) != 0) {
            return;
        }
        
        printf("Registering user!\n");

        strcpy(clients[connected].nick, msg1.nick);
        clients[connected].active = 1;
        clients[connected].sock = msg1.sock;
        clients[connected].type = UNIX;
        setLastActivityTime(connected);
        connected++;
        
    } else {
        
        int index = getNumber(msg1.nick);
        
        if (checkIfActive(msg1.nick)) {
           
            if (strncmp(msg1.buf, "_R", 2) == 0) {
                
                clients[index].active = 1;
                
                setLastActivityTime(index);
                
                printf("Client %s -> has got 30s more\n", msg1.nick);
                
            } else {
        
            printf("%s\n", msg1.buf);
            
            setLastActivityTime(index);
            
            sendToAll(msg1);
            }
                
                
        } else {
            
            printf("Not active!\n");

            if (strncmp(msg1.buf, "_R", 2) == 0) {

                clients[index].active = 1;

                setLastActivityTime(index);

                printf("Client %s -> has got 30s more\n", msg1.nick);
            }
        }
    }
}



void internetHandler(struct msg msg1, struct sockaddr_in tmp) {
    
    if (checkIfRegistered(msg1.nick) == 0) {
    
        printf("User not registered!\n");
    
        if (strncmp(msg1.buf, "_R", 2) != 0) {
            return;
        }
        printf("Registering user!\n");

        strcpy(clients[connected].nick, msg1.nick);
        clients[connected].active = 1;
        clients[connected].inter = tmp;
        clients[connected].type = INTERNET;
        setLastActivityTime(connected);
        connected++;
    
        
    } else {
        
        int index = getNumber(msg1.nick);
        
        if (checkIfActive(msg1.nick)) {
            
            if (strncmp(msg1.buf, "_R", 2) == 0) {
        
                clients[index].active = 1;
        
                setLastActivityTime(index);
        
                printf("Client %s -> has got 30s more\n", msg1.nick);
        
            } else {printf("%s\n", msg1.buf);
        
                setLastActivityTime(index);
        
                sendToAll(msg1);
            }
        
            
        } else {
        
            if (strncmp(msg1.buf, "_R", 2) == 0) {
        
                clients[index].active = 1;
        
                setLastActivityTime(index);
        
                printf("Client %s -> has got 30s more\n", msg1.nick);
            }
        }
    }
}



int checkIfRegistered(char *nick) {
    
    int i;
    for (i = 0; i < connected; i++) {
    
        if (strncmp(clients[i].nick, nick, strlen(nick)) == 0) {
            return 1;
        }
   
    }
    return 0;
}



int checkIfActive(char *nick) {
   
    int i;
    for (i = 0; i < connected; i++) {
    
        if (strcmp(clients[i].nick, nick) == 0) {
    
            if (clients[i].active == 1) {
                return 1;
            }
    
        }
    
    }
    return 0;
}



int getNumber(char *nick) {
    
    int i;
    for (i = 0; i < connected; i++) {
        
        if (strcmp(clients[i].nick, nick) == 0) {
            return i;
        }
    
    }
    return -1;
}



void sendToAll(struct msg msg1) {
    printf("Sending...\n");
    
    int tmp;
    int i;
    for (i = 0; i < connected; i++) {
        
        if (clients[i].active == 0) {
            continue;
        }
        
        if (clients[i].type == INTERNET) {
        
            tmp = (int) sendto(socketInternet, (void *) &msg1, sizeof(struct msg), 0,
                              (struct sockaddr *) &clients[i].inter, sizeof(struct sockaddr_in));
        
            if (tmp < 0) {
                perror("sendto():");
                exit(EXIT_FAILURE);
            }
            
        } else {

            struct sockaddr_un temp;
            
            temp.sun_family = AF_UNIX;
            
            strcpy(temp.sun_path, clients[i].nick);
            
            tmp = (int) sendto(clients[i].sock, (void *) &msg1, sizeof(struct msg), 0, (struct sockaddr *) &temp,
                              sizeof(struct sockaddr_un));
            
            if (tmp < 0) {
                perror("sendto():");
            }
        }
    }
}



void setLastActivityTime(int index) {
   
    struct timeval tv;
   
    if (gettimeofday(&tv, NULL) == -1) {
        perror("gettimeofday():");
        return;
    }
    
    clients[index].lastActivity = tv.tv_sec;
}



void deactivateUsers() {
    
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) == -1) {
        perror("gettimeofday():");
        return;
    }
    
    printf("checking if everybody active....\n");
    
    long currentSec = tv.tv_sec;
    
    int i;
    for (i = 0; i < connected; i++) {
    
        if ((currentSec - clients[i].lastActivity > INACTIVITY_MAX) && (clients[i].active == 1)) {
            clients[i].active = 0;
            printf("CLient %s deactivated\n", clients[i].nick);
        }
    
    }
    printf("                                    .... OK\n");
}