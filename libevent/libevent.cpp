#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <event.h>

#define PORT 25341
#define BACKLOG 5
#define MEM_SIZE 1024

struct event_base* base;
struct sock_ev
{
    struct event* read_ev;
    struct event* write_ev;
    char* buff;
};

void onAccept(int sock, short event, void* arg);
void onRead(int sock, short event, void* arg);
void onWrite(int sock, short event, void* arg);
void release_sock_event(struct sock_ev* ev);

int main()
{
    struct sockaddr_in my_addr;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) );
    memset(&my_addr, 0, sizeof(my_addr));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
    listen(sock, BACKLOG);

    struct event listen_ev;
    base = event_base_new();
    event_set(&listen_ev, sock, EV_READ | EV_PERSIST, onAccept, NULL);
    event_base_set(base, &listen_ev);
    event_add(&listen_ev, NULL);
    event_base_dispatch(base);


    return 0;
}

void onAccept(int sock, short event, void* arg)
{
    struct sockaddr_in cli_addr;
    int newfd, sin_size;
    struct sock_ev* ev = (struct sock_ev*) malloc(sizeof(struct sock_ev));
    ev->read_ev = (struct event*) malloc(sizeof(struct event));
    ev->write_ev = (struct event*) malloc(sizeof(struct event));
    sin_size = sizeof(struct sockaddr_in);

    newfd = accept(sock, (struct sockaddr*)&cli_addr, (socklen_t*)&sin_size);

    event_set(ev->read_ev, newfd, EV_READ | EV_PERSIST, onRead, ev);
    event_base_set(base, ev->read_ev);
    event_add(ev->read_ev, NULL);
    printf("a new connect build\n");
}

void onRead(int sock, short event, void* arg)
{
    struct event* write_ev;
    int size;
    struct sock_ev* ev = (struct sock_ev*) arg;
    ev->buff = (char*) malloc(MEM_SIZE);
    bzero(ev->buff, MEM_SIZE);
    
    size = recv(sock, ev->buff, MEM_SIZE, 0);
    printf("receive data:%s, size%d\n", ev->buff, size);
    if(0 == size)
    {
        release_sock_event(ev);
        close(sock);
        return;
    }

    event_set(ev->write_ev, sock, EV_WRITE, onWrite, ev->buff);
    event_base_set(base, ev->write_ev);
    event_add(ev->write_ev, NULL);
}

void onWrite(int sock, short event, void* arg)
{
    char* buff = (char*) arg;
    send(sock, buff, strlen(buff), 0);

    free(buff);
}

void release_sock_event(struct sock_ev* ev)
{
    if(NULL != ev)
    {
        event_del(ev->read_ev);
        free(ev->read_ev);
        free(ev->write_ev);
        free(ev->buff);
        free(ev);
    }
}
