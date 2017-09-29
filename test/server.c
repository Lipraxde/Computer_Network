#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    // socket的建立
    char message[] = {"Connect to server.\n"};
    char *msg_arg[] = {"Input operand A: ", 
                       "Input operand B: ", 
                       "Input operator : "};
    char input_buffer[256] = {0};
    char output_buffer[256] = {0};
    double operand_A = 0;
    double operand_B = 0;
    double ans = 0;
    char operator = '\0';
    int sockfd = 0,forClientSockfd = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in serverInfo,clientInfo = {0};
    socklen_t addrlen = sizeof(clientInfo);
    int server_addrlen = sizeof(serverInfo);
    // bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(4000);
    bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
    listen(sockfd,5);

    system("ip address");
    while(1) {
        forClientSockfd = accept(sockfd, (struct sockaddr*)&clientInfo, &addrlen);
        
        for(int i=0; i<3; i++)
        {
            send(forClientSockfd, msg_arg[i], strlen(msg_arg[i]), 0);

            int set_strend = recv(forClientSockfd, input_buffer, sizeof(input_buffer)-1, 0);
            input_buffer[set_strend] = '\0';

            switch(i)
            {
            case 0:
                sscanf(input_buffer, "%lf", &operand_A);
                printf("A = %lf\n", operand_A);
                break;
            case 1:
                sscanf(input_buffer, "%lf", &operand_B);
                printf("B = %lf\n", operand_B);
                break;
            case 2:
                operator = input_buffer[0];
                printf("C = %c\n", operator);
                break;
            default:
                break;
            }
        }

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
    }

    return 0;
}

