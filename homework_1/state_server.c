#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "status.h"

const char connect_msg[] = {"COnnect to server.\n"};

struct calculator
{
    float operand_A;
    float operand_B;
    char operator;
    float ans;
};

struct server_bound
{
    int sockfd;
    int clientfd;
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    socklen_t addrlen;
};

void init_socket(status_t *);
void listen_socket(status_t *);
void accept_client(status_t *);
void provide_server(status_t *);

void init_socket(status_t *s)
{
#define server ((struct server_bound *)(*s).p)
#define sock_fd server->sockfd

    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(sock_fd==-1)
    {
        printf("Fail to create a socket.\n");
        s->return_val = -1;
        s->run = false;
        return;
    }

    server->addrlen = sizeof(server->client_info);

    server->server_info.sin_family = PF_INET;
    server->server_info.sin_addr.s_addr = INADDR_ANY;
    server->server_info.sin_port = htons(4000);

    (*s).now_status = listen_socket;
}

void listen_socket(status_t *s)
{
#define server ((struct server_bound *)(*s).p)
#define sock_fd server->sockfd
    
    if(bind(sock_fd, (struct sockaddr *)&server->server_info, server->addrlen)<0)
    {
        perror("Bind failed.\n");
        s->return_val = -1;
        s->run = false;
    }

    listen(sock_fd, 5);
    system("ip address");

    (*s).now_status = accept_client;
}

void accept_client(status_t *s)
{
#define server ((struct server_bound *)(*s).p)
#define sock_fd server->sockfd
#define client_fd server->clientfd

    printf("Wait client connect.\n");
    client_fd = accept(sock_fd, (struct sockaddr *)&server->client_info, &(server->addrlen));

    if(client_fd < 0)
    {
        perror("Accept failed");
        s->return_val = -1;
        s->run = false;
        return;
    }

    printf("Connection accepted.\n");
    (*s).now_status = provide_server;
}

void provide_server(status_t *s)
{

    
    (*s).now_status = accept_client;
}

int main(int argc, char *argv[])
{
    status_t sock_mach = {NULL, NULL, true, 0};

    {
        struct server_bound server1 = {0};
        sock_mach.p = &server1;
        STATE_MACHINE(sock_mach, init_socket);
    }

    return 0;
}

