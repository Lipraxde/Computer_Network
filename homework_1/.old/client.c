#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(int argc , char *argv[])
{

    //socket的建立
    int sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_in info = {0};
    info.sin_family = PF_INET;
    
    //localhost test
    inet_pton(AF_INET, argv[1], &info.sin_addr.s_addr);
    info.sin_port = htons(4000);

    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1)
        printf("Connection error");

    //Send a message to server
    char receiveMessage[256] = {0};
    char send_msg[256] = {0};
    
    for(int i=0;i<3;i++)
    {
        recv(sockfd, receiveMessage, sizeof(receiveMessage), 0);
        printf("%s", receiveMessage);
        scanf("%255s", send_msg);
        send(sockfd, send_msg, strlen(send_msg), 0);
    }
    recv(sockfd, receiveMessage, sizeof(receiveMessage), 0);
    printf("%s", receiveMessage);

    printf("close Socket\n");
    close(sockfd);
    return 0;
}

