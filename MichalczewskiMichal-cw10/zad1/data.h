#define MSSG_LNGTH 256
#define NICK_LNGTH 12
#define INTERNET 666
#define UNIX 667
#include <sys/un.h>

struct msg {
    char buf[MSSG_LNGTH];
    char nick[NICK_LNGTH];
    int sock;
};

struct clientProfile {
    struct sockaddr_in inter;
    char nick [NICK_LNGTH];
    long lastActivity;
    int active;
    int type;
    int sock;
};