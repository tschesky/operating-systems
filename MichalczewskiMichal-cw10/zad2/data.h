#define MSSG_LNGTH 256
#define NICK_LNGTH 12
#define CLIENTS_MAX 15
#define LOCAL_TYPE 999
#define INTERNET_TYPE 1000


struct server {
    int sock;
    int type;
    struct sockaddr_in addr;
    struct sockaddr_un addrLocal;
};

struct connection {
    int sock;
    struct sockaddr_in addr;
    struct sockaddr_in local;
    char nick[NICK_LNGTH];
    int type;
};

struct msg {
    char nick[NICK_LNGTH];
    char buff[MSSG_LNGTH];
};