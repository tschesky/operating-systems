#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include "data.h"
#include <sys/select.h>
#include <unistd.h>
#include <bits/signum.h>
#include <signal.h>


int rc;

int sock;

int mySocket;

int connected = 0;

int removed = 0;

struct server *servers;

fd_set fds[CLIENTS_MAX];

struct connection con[CLIENTS_MAX];


void removeClient(struct connection *c) {
    
    int i;
    for (i = 0; i < connected; i++) {
        if (strcmp (c->nick, con[i].nick) == 0){
            
            for (i; i < connected; i++){
                if (i < connected - 1){
                con[i] = con[i+1];
                } else {
                   memset(&con[i],0,sizeof(struct connection));
                   
                }
            }
        }
    }
    
}


void obtainFd(struct connection * arrConn, int nConn, struct server *arrServ, int nServ, int *fd_max){
    
    int m = -1;
    int i;
    for (i = 0; i < nServ; i++) {
        
        struct server *s = &arrServ[i];
        if (s->sock >= 0) {
            
            if (m < s->sock) {
                m = s->sock;
            }
            FD_SET (s->sock, fds);
        }
    }

    for(i = 0; i < nConn; i++) {
        struct connection *c = &arrConn[i];
        
        if (c->sock >= 0) {
            
            if(m < c->sock) {
                m = c->sock;
            }
            FD_SET (c->sock,fds);
        }
    }
    
    *fd_max = m +1;
}

void newClientHandler(struct server *pServer) {
    
    int sockTemp;
    printf("New  client obtaining connection\n");

    struct connection tmpConnection;
    
    int lngth = sizeof(tmpConnection.addr);
    
    sockTemp = accept(pServer->sock,(struct sockaddr *)&tmpConnection.addr,&lngth);
    
    if (sockTemp == -1) {
        perror("accept():");
        exit(EXIT_FAILURE);
    }


    lngth = sizeof(tmpConnection.local);

    if (getsockname (sockTemp, (struct sockaddr *) &tmpConnection.local, &lngth) == -1) {
        perror("getsockename():");
        exit(EXIT_FAILURE);
    }

    tmpConnection.sock=sockTemp;


    if(connected < CLIENTS_MAX) {

        int i;
        for(i = 0; i < CLIENTS_MAX; i++) {
            
            if (con[i].sock < 0) {
                
                con[i] = tmpConnection;
                printf("Client connected\n");
                break;
            }
        }
        connected++;
    }
}

void connectionHandler(struct connection *c) {

    char buf[MSSG_LNGTH];
    
    int tmp;
    
    if ((tmp = read(c->sock,buf,MSSG_LNGTH)) == -1) {
        perror("read():");
        exit(EXIT_FAILURE);
    
        
    } else if (tmp == 0) {
        removeClient(c);
        removed = 1;
    }

    if(!removed){

        if(strncmp(buf,"_R",2) == 0) { 
            
            strncpy(c->nick, buf + 2,NICK_LNGTH);
            int len = (int)strlen(c->nick);
            printf("%s registered\n",c->nick);
            return;
        }
        
        buf[strlen(buf)-1]='\0';
        
        printf("\n%s >> %s\n",c->nick,buf);
    
    
        int i;
        for (i = 0; i < CLIENTS_MAX; i++){
            
            if (con[i].sock == -1) {
                continue;
            }
            
            if (con[i].sock == c->sock) {
                continue;
            }
            
            char buff2[MSSG_LNGTH];
            
            memset(buff2,'\0',sizeof(buff2));
    
            strncpy(buff2,"\n",1);
            strncpy(buff2+1,c->nick,strlen(c->nick));
            strncpy(buff2+strlen(c->nick)+1," >> ",strlen(buf));
            strncpy(buff2+strlen(c->nick)+5,buf,strlen(buf));
    
            if((int)write(con[i].sock,buff2,MSSG_LNGTH) < 0) {
                printf("Error while sending message!\n");
                perror("write():");
                exit(EXIT_FAILURE);
            }
        }
    } else {
        removed = 0;
    }
}



void atexitFun(void) {
    system("rm -r socket1");
}

int main(int argc, char **argv) {

    servers=malloc(sizeof(struct server)*2);
    atexit(atexitFun);

    for(int i = 0;i<CLIENTS_MAX;i++)
    {
        memset(&con[i],0,sizeof(struct connection));
        con[i].sock=-1;
    }


    sock= -1;
    int t = 1;
    int fd_max = -1;
    struct sockaddr_in socketInternetAddr;
    struct sockaddr_un socketUnixAddr;


    sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sock < 0) {
        perror("socket error:");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&t,sizeof(t)) < 0) {
        perror("setsockopt():");
        exit(EXIT_FAILURE);
    }

    socketInternetAddr.sin_family=AF_INET;
    socketInternetAddr.sin_port=htons(atoi(argv[1]));
    socketInternetAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servers[0].addr=socketInternetAddr;
    servers[0].sock=sock;
    servers[0].type=INTERNET_TYPE;


    if (bind(sock,(struct sockaddr *)&socketInternetAddr,sizeof(socketInternetAddr)) < 0) {
        perror("bind():");
        exit(EXIT_FAILURE);
    }

    if (listen(sock,1) < 0) {
        perror("listen():");
        exit(EXIT_FAILURE);
    }


    mySocket = socket(AF_UNIX,SOCK_STREAM,0);
    
    if (mySocket < 0) {
        perror("socket error:");
        exit(EXIT_FAILURE);
    }
    
    socketUnixAddr.sun_family=AF_UNIX;
    
    strcpy(socketUnixAddr.sun_path,argv[2]);
    
    servers[1].type=LOCAL_TYPE;
    
    servers[1].addrLocal=socketUnixAddr;
    
    servers[1].sock=mySocket;


    if(bind(mySocket,(struct sockaddr *)&socketUnixAddr, sizeof(socketUnixAddr)) < 0) {
        perror("bind():");
        exit(EXIT_FAILURE);
    }

    if (listen(mySocket,10) < 0) {
        perror("listen():");
        exit(EXIT_FAILURE);
    }


    while(1) {
        obtainFd(con,CLIENTS_MAX,servers,2,&fd_max);


        if (select(fd_max,fds,NULL,NULL,NULL) < 0) {
            perror("select():");
            exit(EXIT_FAILURE);
        }

        int i;
        for (i = 0; i < 2; i++) {
            if ((servers[i].sock >= 0) && FD_ISSET(servers[i].sock, fds))
                newClientHandler(&servers[i]);
        }
 
        for(i =0; i < CLIENTS_MAX; ++i) {
         
            struct connection *c = &con[i];
         
            if( c->sock >= 0 && FD_ISSET (c->sock, fds))
                connectionHandler(c);

        }
    }
    return EXIT_SUCCESS;
}

