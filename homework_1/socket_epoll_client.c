#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#define PORT        4000
#define MAX_EVENTS  10

const char connect_msg[] = {"Connect to server.\n"};


struct client_bound
{
    int sock_fd;
    int epoll_fd;
    int nfds;
    int nfd;
    struct epoll_event ev, all_events[MAX_EVENTS];
    struct sockaddr_in server_info;
    socklen_t addrlen;
    bool run;
};


struct calc_bound
{
    char _header;
    float A;
    float B;
    char operator;
};


int init_socket(struct client_bound *s, uint32_t ip_address, int port)
{
    s->sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    
    if(s->sock_fd == -1)
    {
        printf("Fail to create a socket.\n");
        return -1;
    }

    s->addrlen = sizeof(s->server_info);

    s->server_info.sin_family = AF_INET;
    s->server_info.sin_addr.s_addr = ip_address;
    s->server_info.sin_port = htons(port);

    s->run = true;
    return 0;
}


int connect_socket(struct client_bound *s)
{
    int err = connect(s->sock_fd, (struct sockaddr *)&s->server_info, s->addrlen);
    if(err == -1)
    {
        perror("connect");
        return -1;
    }

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

    s->ev.events = EPOLLIN; // | EPOLLET;
    s->ev.data.fd = 0;
    if(epoll_ctl(s->epoll_fd, EPOLL_CTL_ADD, 0, &s->ev) == -1)
    {
        perror("epoll_ctl: stdin");
        return -1;
    }

    return 0;
}


int wait_epoll(struct client_bound *s, int max_events)
{
    s->nfds = epoll_wait(s->epoll_fd, s->all_events, max_events, 10);
    if(s->nfds == -1)
    {
        perror("epoll_wait");
        return -1;
    }

    return 0;
}


int recv_data(struct client_bound *s, char *input_buffer, uint32_t buffer_size)
{
    *input_buffer = 0;
    if(s->all_events[s->nfd].events & EPOLLIN)
    {
        int count = recv(s->all_events[s->nfd].data.fd, input_buffer, buffer_size-1, MSG_DONTWAIT);
        if(count == -1)
        {
            if(errno != EAGAIN)
            {
                perror("recv");
                close(s->all_events[s->nfd].data.fd);
                epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, s->all_events[s->nfd].data.fd, &s->ev);
                return -1;
            }
        }
        if(count == 0)
        {
            close(s->all_events[s->nfd].data.fd);
            epoll_ctl(s->epoll_fd, EPOLL_CTL_DEL, s->all_events[s->nfd].data.fd, &s->ev);
            s->run = false;
            return 0;
        }
        input_buffer[count] = 0;
        printf("%s", input_buffer);
        fflush(stdout);
    }

    return 0;
}


int calc(char *input_buffer, struct client_bound *client)
{
    static struct calc_bound data = {0};
    static int status = 0;

    switch(status)
    {
    case 0:
        printf("init ...\n");
        data.A = 0;
        data.B = 0;
        data.operator = 0;
        printf("Input operand A: ");
        status = 1;
        break;
    case 1:
        sscanf(input_buffer, "%f", &data.A);
        printf("Input operator : ");
        status = 3;
        break;
    case 2:
        sscanf(input_buffer, "%f", &data.B);
        printf("Check data:\n");
        printf("A = %f\n", data.A);
        printf("B = %f\n", data.B);
        printf("O = %c\n", data.operator);
        printf("Press ENTER to continue.");
        status = 4;
        break;
    case 3:
        sscanf(input_buffer, "%c", &data.operator);
        printf("Input operand B: ");
        status = 2;
        break;
    case 4:
        printf("Send data to server ...\n");
        send(client->sock_fd, &data, sizeof(struct calc_bound), MSG_DONTWAIT);
        status = 0;
        break;
    default:
        status = 0;
        break;
    }
 
    fflush(stdout);
    return 0;
}


int socket_client(int ip, int port, int max_events)
{
    char input_buffer[256];
    char send_buffer[256];
    struct client_bound *client;

    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);

    client = (struct client_bound *)calloc(1, sizeof(struct client_bound));
    
    assert(init_socket(client, ip, port) == 0);
    assert(connect_socket(client) == 0);
    while(client->run)
    {
        assert(wait_epoll(client, max_events) == 0);
        for(client->nfd = 0; client->nfd < client->nfds; client->nfd++)
        {
            if(client->all_events[client->nfd].data.fd == client->sock_fd)
            {
                assert(recv_data(client, input_buffer, sizeof(input_buffer)) == 0);
                input_buffer[0] = 0;
                assert(calc(input_buffer, client) == 0);
            }
            else if(client->all_events[client->nfd].data.fd == 0)
            {
                fgets(input_buffer, sizeof(input_buffer), stdin);
                assert(calc(input_buffer, client) == 0);
                // while((fgets(send_buffer, sizeof(send_buffer), stdin)) != NULL)
                //    send(client->sock_fd, send_buffer, strlen(send_buffer), MSG_DONTWAIT);
            }
        }
        // sleep(0);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: client IP\n");
        printf("Example: client 127.0.0.1\n");
    }
    else
        socket_client(inet_addr(argv[1]), PORT, MAX_EVENTS);

    return 0;
}

