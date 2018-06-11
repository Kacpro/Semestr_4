#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/un.h>


int calculate(char* e)
{
    int arg1 = atoi(strtok(e, " "));
    char* op = strtok(NULL, " ");
    int arg2 = atoi(strtok(NULL, " \n"));

    switch(op[0])
    {
        case '+': return arg1 + arg2;
        case '-': return arg1 - arg2;
        case '*': return arg1 * arg2;
        case '/': return arg1 / arg2;
        default: return -1;
    }
}


void receive(int fd, char* name)
{
    char recvBuff[1024];

    char* msg = calloc(128, sizeof(char));
    strcat(msg, "1");
    strcat(msg, name);
    write(fd, msg, strlen(msg));
    free(msg);

    read(fd, recvBuff, sizeof(recvBuff));

    if (recvBuff[0] == '1' && (recvBuff[1] == 'N' || recvBuff[1] == 'Y'))
    {
        if (recvBuff[1] == 'N')
        {
            printf("Given name already exists in cluster\n");
            return;
        }
        else
        {
            printf("Connected to cluster\n");
        }
    }
    else
    {
        printf("Unknown message\n");
        return;
    }

    while (1)
    {
        recv(fd, recvBuff, sizeof(recvBuff), 0);
        if (recvBuff[0] == '3')
        {
            char* result = calloc(10, sizeof(char));
            int p = (int)recvBuff[1];
            printf("Calc request: %s", recvBuff + 2 * sizeof(char));
            sprintf(result, "%d", calculate(recvBuff + 2 * sizeof(char)));

            char* msg = calloc(128, sizeof(char));
            char pos[] = {(char)p, '\0'};
            strcpy(msg, "3");
            strcat(msg, pos);
            strcat(msg, result);

            write(fd, msg, 128);
            free(msg);
        }

        usleep(1000);
    }
}


void initNet(char* address, char* name)
{
    char* addr = strtok(address, ":");
    int port = atoi(strtok(NULL, "\0\n"));
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, addr, &serv_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    receive(sockfd, name);
}


void initLocal(char* path, char* name)
{
    int sockfd = 0;
    struct sockaddr_un serv_addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    strcpy(serv_addr.sun_path, path);
    serv_addr.sun_family = AF_UNIX;

    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    receive(sockfd, name);
}

int main(int argc, char** argv)
{
    if (argc != 4) return -1;
    char* name = argv[1];
    int mode = atoi(argv[2]);
    char* address = argv[3];

    if (mode == 0) initNet(address, name);
    if (mode == 1) initLocal(address, name);
    return 0;
}

