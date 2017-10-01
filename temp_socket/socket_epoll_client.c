#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PORT        4000
#define BACKLOG     5
#define MAX_EVENTS  10

const char connect_msg[] = {"Connect to server.\n"};


struct client_bound
{
    int sock_fd;
    struct sockaddr_in server_info;
    socklen_t addrlen;
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

    return 0;
}


int socket_client(int ip, int port)
{
    struct client_bound *client;

    client = (struct client_bound *)calloc(1, sizeof(struct client_bound));

    init_socket(client, ip, port);
    connect_socket(client);
    while(true)
    {
        char input_buffer[256];
        int count = recv(client->sock_fd, input_buffer, sizeof(input_buffer), MSG_DONTWAIT);
        if(count == -1)
        {
            if(errno != EAGAIN)
            {
                perror("recv");
                close(client->sock_fd);
                return -1;
            }
        }
        if(count == 0)
        {
            close(client->sock_fd);
            break;
        }
        printf("%s", input_buffer);
        fflush(stdout);



        char send_buffer[256];
        
        while((fgets(send_buffer, sizeof(send_buffer), stdin)) != NULL)
            send(client->sock_fd, send_buffer, strlen(send_buffer), 0);

        sleep(10);
    }

    return 0;
}

int main(int argc, char *argv[])
{

    socket_client(inet_addr(argv[1]), PORT);

    return 0;
}

