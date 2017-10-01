#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "status.h"

#define IP_ADDRESS  INADDR_ANY
#define PORT        4000
#define BACKLOG     5
#define MAX_EVENTS  10
#define PACK_BEGIN  "_BEGIN"
#define PACK_END    "_END"

const char connect_msg[] = {"Connect to server.\n"};


struct server_bound
{
    int sock_fd;
    int client_fd;
    int epoll_fd;
    int nfds;
    int nfd;
    struct epoll_event ev, all_events[MAX_EVENTS];
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    socklen_t addrlen;
};


int init_socket(struct server_bound *s, uint32_t ip_address, int port)
{
    s->sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(s->sock_fd == -1)
    {
        printf("Fail to create a socket.\n");
        return -1;
    }

    s->addrlen = sizeof(s->client_info);

    s->server_info.sin_family = AF_INET;
    s->server_info.sin_addr.s_addr = ip_address;
    s->server_info.sin_port = htons(port);

    return 0;
}


int listen_socket(struct server_bound *s, int backlog)
{
    if(bind(s->sock_fd, (struct sockaddr *)&s->server_info, s->addrlen) < 0)
    {
        perror("bind");
        return -1;
    }

    listen(s->sock_fd, backlog);

    s->epoll_fd = epoll_create1(0);
    if(s->epoll_fd == -1)
    {
        perror("epoll_create1");
        return -1;
    }

    s->ev.events = EPOLLIN;
    s->ev.data.fd = s->sock_fd;
    if(epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, s->sock_fd, &s->ev) == -1)
    {
        perror("epoll_ctl: sock_fd");
        return -1;
    }

    return 0;
}


int wait_epoll(struct server_bound *s, int max_events)
{
    s->nfds = epoll_wait(s->epoll_fd, s->all_events, max_events, 10);
    if(s->nfds == -1)
    {
        perror("epoll_wait");
        return -1;
    }

    return 0;
}


int accept_client(struct server_bound *s)
{
    s->client_fd = accept(s->sock_fd, (struct sockaddr *)&s->client_info, &(s->addrlen));
    if(s->client_fd == -1)
    {
        perror("accept");
        return -1;
    }
    
    printf("Connection accepted.\n");
    send(s->client_fd, connect_msg, strlen(connect_msg), MSG_DONTWAIT);
    
    s->ev.events = EPOLLIN; // | EPOLLET;
    s->ev.data.fd = s->client_fd;
    if(epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, s->client_fd, &s->ev) == -1)
    {
        perror("epoll_ctl: client_fd");
        return -1;
    }

    return 0;
}


int provide_server(struct server_bound *s, char *input_buffer, uint32_t buffer_size)
{

    if(s->all_events[s->nfd].events & EPOLLIN)
    {
        int count = recv(s->all_events[s->nfd].data.fd, input_buffer, buffer_size-1, MSG_DONTWAIT);
        if(count == -1)
        {
            if(errno != EAGAIN)
            {
                perror ("read");
                close(s->all_events[s->nfd].data.fd);
                epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, s->all_events[s->nfd].data.fd, &s->ev);
                return -1;
            }
        }
        if(count == 0)
        {
            close(s->all_events[s->nfd].data.fd);
            epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, s->all_events[s->nfd].data.fd, &s->ev);
        }
        input_buffer[count] = '\0';
        printf("%s", input_buffer);
        fflush(stdout);
    }
    // send(all_events[n].data.fd, "ABC GOGO", strlen("ABC GOGO"), MSG_DONTWAIT);
    return 0;
}


int socket_epoll(int ip, int port, int backlog, int max_events)
{
    char input_buffer[256];
    struct server_bound *server;

    server = (struct server_bound *)calloc(1, sizeof(struct server_bound));

    init_socket(server, ip, port);
    listen_socket(server, backlog);
    while(true)
    {
        wait_epoll(server, max_events);
        for(server->nfd = 0; server->nfd < server->nfds; server->nfd++)
        {
            if(server->all_events[server->nfd].data.fd == server->sock_fd)
                accept_client(server);
            else
                provide_server(server, input_buffer, sizeof(input_buffer));
        }
    }



}

int main(int argc, char *argv[])
{
    socket_epoll(INADDR_ANY, PORT, BACKLOG, MAX_EVENTS);

    return 0;
}

