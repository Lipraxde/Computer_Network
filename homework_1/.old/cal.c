
#define BUFFER_SIZE 256

struct calculator
{
    float operand_A;
    float operand_B;
    char operator;
    float ans;
    int clientfd;
    char recv_buffer[BUFFER_SIZE];
};
