#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "data.h"


int socketFD;
int temp;
struct connection cntcn;
fd_set fds;
void sendServer(void *ptr) ;
void receiveFromServer(void *ptr);
char *nick;

int main(int argc, char **argv) {
    /*
     * Paremeters
     * 1-nick
     * 2- type of client (local = 0, internet = 1)
     * 3 - adres ip/path
     * 4 - port
     */

    if (argc != 4 && argc != 5) {
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    nick = malloc(sizeof(char)*NICK_LNGTH);
    memset(nick,'\0',sizeof(nick));
    
    strcpy(nick,argv[1]);

    if (atoi (argv[2]) == 1) {
        
        struct sockaddr_in stSockAddr;
        
        int res;
        
        socketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        cntcn.sock = socketFD;

        if (-1 == socketFD) {
            perror("cannot create socket");
            exit(EXIT_FAILURE);
        }

        memset(&stSockAddr, 0, sizeof(stSockAddr));

        stSockAddr.sin_family = AF_INET;

        stSockAddr.sin_port = htons((uint16_t )atoi(argv[4]));

        res = inet_pton(AF_INET, argv[3], &stSockAddr.sin_addr);

        if (0 > res) {
            perror("inet_pton");
            close(socketFD);
            exit(EXIT_FAILURE);
        }
        else if (0 == res) {
            perror("inet_pton");
            close(socketFD);
            exit(EXIT_FAILURE);
        }

        if (-1 == connect(socketFD, (struct sockaddr *) &stSockAddr, sizeof(stSockAddr))) {
            perror("connect failed");
            close(socketFD);
            exit(EXIT_FAILURE);
        }
    }

    if(atoi(argv[2])==0)
    {
        socketFD=socket(AF_UNIX,SOCK_STREAM,0);
        cntcn.sock=socketFD;
        struct sockaddr_un serv;
        memset(&serv,0,sizeof(serv));

        strcpy(serv.sun_path,argv[3]);
        serv.sun_family=AF_UNIX;
        int len = (int)strlen(serv.sun_path)+(int)sizeof(serv.sun_family);

        temp=connect(socketFD,(struct sockaddr *)&serv,len);
        if(temp==-1)
        {
            perror("connect():");
            exit(EXIT_FAILURE);
        }

    }


    pthread_t senderThread;
    pthread_t receiverThread;
    pthread_create(&senderThread,NULL,(void*)sendServer,(void*)(intptr_t)socketFD);
    pthread_create(&receiverThread,NULL,(void*)receiveFromServer,(void*)(intptr_t)socketFD);
    pthread_join(receiverThread,NULL);
    pthread_join(senderThread,NULL);

    (void) shutdown(socketFD, SHUT_RDWR);
    close(socketFD);
    printf("Client terminated!\n");
    return EXIT_SUCCESS;
}

void sendServer(void *ptr) {

    char buff2[MSSG_LNGTH]={0};
    memset(buff2,'\0',sizeof(buff2));
    char *nickNotification = "_R";
    int sock = (intptr_t)ptr;
    printf("Reading from user....\n");

    strcpy(buff2,nickNotification);
    strcpy(buff2+2,nick);

    write(sock, buff2,MSSG_LNGTH);

    while (fgets(buff2, sizeof(buff2), stdin)!=NULL) {
        int rec = (int)write(sock, buff2, MSSG_LNGTH);
        if (rec == -1) {
            printf("Error while sending to server\n");
        }
    }

    printf("Sender exited \n");
    pthread_exit((void *) EXIT_SUCCESS);
}

void receiveFromServer(void *ptr) {

    char buff[MSSG_LNGTH] ={0};
    int sock = (intptr_t)ptr;
    FD_ZERO(&fds);
    FD_SET(sock,&fds);
    while (1) {

        select(sock+1, &fds, NULL, NULL, NULL);
        if(FD_ISSET(sock,&fds))
        {
            int num = (int) read(sock, buff, MSSG_LNGTH);
            if (num > 0) {

                printf("%s\n", buff);
            }
        }
    }


}
