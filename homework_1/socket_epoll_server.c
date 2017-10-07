#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#define IP_ADDRESS  INADDR_ANY
#define PORT        4000
#define BACKLOG     5
#define MAX_EVENTS  10
#define MAX_CLIENTS 500

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


struct calc_bound
{
    char _header;
    float A;
    float B;
    char operator;
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
                perror("recv");
                close(s->all_events[s->nfd].data.fd);
                // epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, s->all_events[s->nfd].data.fd, &s->ev);
                return -1;
            }
            return 0;
        }
        if(count == 0)
        {
            close(s->all_events[s->nfd].data.fd);
            // epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, s->all_events[s->nfd].data.fd, &s->ev);
            return 0;
        }
        input_buffer[count] = 0;

        struct calc_bound *ABC;
        if(input_buffer[0] == 0x00)
            ABC = (struct calc_bound *)input_buffer;
        else
            return 0;
        
        int forClientSockfd = s->all_events[s->nfd].data.fd;
        float operand_A = ABC->A;
        float operand_B = ABC->B;
        char operator = ABC->operator;
        float ans = 0;
        char output_buffer[256] = {0};

        bool have_ans = true;
        switch(operator)
        {
        case '+':
            ans = operand_A + operand_B;
            break;
        case '-':
            ans = operand_A - operand_B;
            break;
        case '*':
            ans = operand_A * operand_B;
            break;
        case '/':
            if(operand_B!=0)
                ans = operand_A / operand_B;
            else
            {
                printf("Divied by 0.\n");
                send(forClientSockfd, "Divied by 0.\n", 13, 0);
                have_ans = false;
            }
            break;
        default:
            printf("Does not support.\n");
            send(forClientSockfd, "Does not support.\n", 18, 0);
            have_ans = false;
        }

        if(have_ans)
        {
            snprintf(output_buffer, sizeof(output_buffer), "%.2lf %c %.2lf = %.2lf\n", operand_A, operator, operand_B, ans);
            printf("%s", output_buffer);
            send(forClientSockfd, output_buffer, strlen(output_buffer), 0);
        }

        // printf("%s", input_buffer);
        // fflush(stdout);
    }
    // send(all_events[n].data.fd, "ABC GOGO", strlen("ABC GOGO"), MSG_DONTWAIT);
    return 0;
}


int socket_epoll(int ip, int port, int backlog, int max_events)
{
    char input_buffer[256];
    struct server_bound *server;

    server = (struct server_bound *)calloc(1, sizeof(struct server_bound));

    assert(init_socket(server, ip, port) == 0);
    assert(listen_socket(server, backlog) == 0);
    while(true)
    {
        assert(wait_epoll(server, max_events) == 0);
        for(server->nfd = 0; server->nfd < server->nfds; server->nfd++)
        {
            if(server->all_events[server->nfd].data.fd == server->sock_fd)
                assert(accept_client(server) == 0);
            else
                assert(provide_server(server, input_buffer, sizeof(input_buffer)) == 0);
        }
    }
}

int main(int argc, char *argv[])
{
    socket_epoll(INADDR_ANY, PORT, BACKLOG, MAX_EVENTS);

    return 0;
}

