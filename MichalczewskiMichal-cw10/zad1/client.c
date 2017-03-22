#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include "data.h"


int socketInternet;

int mySocket;

struct sockaddr_in server;

struct sockaddr_un serverLocal;

fd_set fds;

int type;

char *nick;



void receiver(void *ptr);

void writer(void *ptr);



void intHandler(int signo) {
    
    printf("Bye!");
    
    close(mySocket);
    
    exit(EXIT_SUCCESS);
}


void atexit_function(void) {
    
    intHandler(1);
}



int main(int argc, char *argv[]) {

    signal(SIGINT,intHandler);

    atexit(atexit_function);

//    int len = sizeof(struct sockaddr_in);

    char buf[MSSG_LNGTH];

    struct hostent *host;

    int port;

    /*
     * 1-type (local = 0, internet = 1)
     * 2-nick
     * 3-host/path
     * 4-port
     */

    type = atoi(argv[1]);
   
    nick = argv[2];

    if (type == 1) {
   
        host = gethostbyname(argv[3]);
   
        if (host == NULL) {
            perror("gethostbyname");
            return 1;
        }

        port = atoi(argv[4]);


        if ((socketInternet = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {

            perror("socket");
            return 1;
        }


        memset((char *) &server, 0, sizeof(struct sockaddr_in));

        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        server.sin_addr = *((struct in_addr *) host->h_addr_list[0]);
    
        
    } else {
        
        socketInternet = socket(AF_UNIX, SOCK_DGRAM, 0);
        
        if (socketInternet < 0) {
            perror("socket():");
            exit(EXIT_FAILURE);
        }
        
        strcpy(serverLocal.sun_path, argv[3]);
        
        serverLocal.sun_family = AF_UNIX;

        mySocket = socket(AF_UNIX, SOCK_DGRAM, 0);
        
        if (mySocket < 0) {
            perror("socket():");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_un selfServer;

        strcpy(selfServer.sun_path, nick);

        selfServer.sun_family = AF_UNIX;

        if (bind(mySocket, (struct sockaddr *) &selfServer, sizeof(struct sockaddr_un)) < 0) {
            perror("bind():");
            exit(EXIT_FAILURE);
        }

    }

    pthread_t receiverThread;
    
    pthread_t writerThread;

    pthread_create(&receiverThread, NULL, (void *) receiver, NULL);
    pthread_create(&writerThread, NULL, (void *) writer, NULL);
    pthread_join(receiverThread, NULL);
    pthread_join(writerThread, NULL);


    close(socketInternet);
    return 0;
}


void writer(void *ptr) {

    struct msg msg1;

    memset(msg1.buf, '\0', sizeof(msg1.buf));

    memset(msg1.nick, '\0', sizeof(msg1.nick));

    strcpy(msg1.nick, nick);

    msg1.sock = mySocket;


    printf("Your nick:%s\n", msg1.nick);

    while (1) {

        fgets(msg1.buf, 256, stdin);

        if (type == 1) {

            if (sendto(socketInternet, (void *) &msg1, sizeof(struct msg), 0, (struct sockaddr *) &server,
                       sizeof(struct sockaddr_in)) == -1) {
                perror("sendto()");
                pthread_exit((void *) EXIT_FAILURE);
            }
        
        
        } else {
        
            if (sendto(socketInternet, (void *) &msg1, sizeof(struct msg), 0, (struct sockaddr *) &serverLocal,
                       sizeof(struct sockaddr_un)) == -1) {
                perror("sendto()");
                pthread_exit((void *) EXIT_FAILURE);
            }
        }
    }

    pthread_exit((void *) EXIT_SUCCESS);
}


void receiver(void *ptr) {
    
    struct msg msg1;
    
    memset(msg1.buf, '\0', sizeof(msg1.buf));
    memset(msg1.nick, '\0', sizeof(msg1.nick));
    
    int size = sizeof(struct sockaddr_in);

    FD_ZERO(&fds);
    
    if (type == 1) {
    
        FD_SET(socketInternet, &fds);
    
        while (1) {
    
            select(socketInternet + 1, &fds, NULL, NULL, NULL);
            recvfrom(socketInternet, (void *) &msg1, sizeof(struct msg), 0, NULL, &size);
            
            if(strcmp(msg1.nick,nick)==0) {
                continue;
            }
            
            printf("%s:\t%s\n", msg1.nick, msg1.buf);
        }
    
    } else {
    
        FD_SET(mySocket, &fds);
    
        while (1) {
    
            select(mySocket + 1, &fds, NULL, NULL, NULL);
    
            recvfrom(mySocket, (void *) &msg1, sizeof(struct msg), 0, NULL, 0);


            if(strcmp(msg1.nick,nick)==0) {
                continue;
            }
        
            printf("Nick:%s\tMessage:%s\n", msg1.nick, msg1.buf);
        }
    }

    pthread_exit((void *) EXIT_SUCCESS);
}