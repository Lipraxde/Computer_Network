#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "status.h"

#define MAX_EVENTS  10
#define IP_ADDRESS  INADDR_ANY
#define PORT        4000
#define PACK_BEGIN  "_BEGIN"
#define PACK_END    "_END"

const char connect_msg[] = {"Connect to server.\n"};

struct server_bound
{
    int _sockfd;
    int _clientfd;
    int _epollfd;
    int _nfds;
    int _nfd;
    struct epoll_event _ev, _all_events[MAX_EVENTS];
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    socklen_t addrlen;
};

void init_socket(status_t *);
void listen_socket(status_t *);
void accept_client(status_t *);
void wait_epoll(status_t *);
void provide_server(status_t *);

#define server      ((struct server_bound *)(*s).p)
#define sock_fd     server->_sockfd
#define client_fd   server->_clientfd
#define epoll_fd    server->_epollfd
#define n_fds       server->_nfds
#define n           server->_nfd
#define ev          server->_ev
#define all_events  server->_all_events

void init_socket(status_t *s)
{
    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(sock_fd == -1)
    {
        printf("Fail to create a socket.\n");
        s->return_val = -1;
        s->run = false;
        return;
    }

    server->addrlen = sizeof(server->client_info);

    server->server_info.sin_family = PF_INET;
    server->server_info.sin_addr.s_addr = IP_ADDRESS;
    server->server_info.sin_port = htons(PORT);

    (*s).now_status = listen_socket;
}

void listen_socket(status_t *s)
{
    if(bind(sock_fd, (struct sockaddr *)&server->server_info, server->addrlen) < 0)
    {
        perror("bind");
        s->return_val = -1;
        s->run = false;
    }

    listen(sock_fd, 5);

    epoll_fd = epoll_create1(0);
    if(epoll_fd == -1)
    {
        perror("epoll_create1");
        s->return_val = -1;
        s->run = false;
        return;
    }

    ev.events = EPOLLIN;
    ev.data.fd = sock_fd;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev) == -1)
    {
        perror("epoll_ctl: sock_fd");
        s->return_val = -1;
        s->run = false;
        return;
    }

    (*s).now_status = wait_epoll;
}

void wait_epoll(status_t *s)
{
    n_fds = epoll_wait(epoll_fd, all_events, MAX_EVENTS, -1);
    if(n_fds == -1)
    {
        perror("epoll_wait");
        s->return_val = -1;
        s->run = false;
        return;
    }

    n = 0;
    (*s).now_status = accept_client;
}

void accept_client(status_t *s)
{
    if(n == n_fds)
    {
        (*s).now_status = wait_epoll;
        return;
    }

    if(all_events[n].data.fd == sock_fd)
    {
        client_fd = accept(sock_fd, (struct sockaddr *)&server->client_info, &(server->addrlen));
        if(client_fd == -1)
        {
            perror("accept");
            s->return_val = -1;
            s->run = false;
            return;
        }
        
        printf("Connection accepted.\n");
        send(client_fd, connect_msg, strlen(connect_msg), MSG_DONTWAIT);
        
        ev.events = EPOLLIN; // | EPOLLET;
        ev.data.fd = client_fd;
        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
        {
            perror("epoll_ctl: client_fd");
            s->return_val = -1;
            s->run = false;
            return;
        }

        n++;
        return;
    }

    (*s).now_status = provide_server;
}

void provide_server(status_t *s)
{
    char input_buffer[256];
    int lfd = all_events[n].data.fd;
    uint32_t levent = all_events[n].events;

    // Study ev.events
    if(lfd != sock_fd)
    {
        printf("fd.events: %d\n", levent);
        printf("fd: %d\n", lfd);        
        if(levent & EPOLLIN)
        {
            int count = recv(lfd, input_buffer, sizeof(input_buffer)-1, MSG_DONTWAIT);
            if(count == -1)
            {
                if(errno != )
                n++
                (*s).now_status = accept_client;
            }
            if(count == 0)
            {
                close(lfd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, lfd, &ev);
            }
            input_buffer[count] = '\0';
            printf("%s", input_buffer);
            fflush(stdout);
        }
            // send(all_events[n].data.fd, "ABC GOGO", strlen("ABC GOGO"), MSG_DONTWAIT);
    }

    n++;
    (*s).now_status = accept_client;
}

#undef server
#undef sock_fd
#undef client_fd
#undef epoll_fd
#undef n_fds
#undef n
#undef ev
#undef all_events

int main(int argc, char *argv[])
{
    status_t sock_mach = {NULL, NULL, true, 0};

    {
        struct server_bound server1 = {0};
        sock_mach.p = &server1;
        STATE_MACHINE(sock_mach, init_socket);
        close(server1._sockfd);
    }

    return 0;
}

