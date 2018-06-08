#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/un.h>


void receive(int fd, char* name)
{
    char recvBuff[1024];
    memset(recvBuff, '0',sizeof(recvBuff));

    char* msg = calloc(100, sizeof(char));
    strcat(msg, "1");
    strcat(msg, name);
    write(fd, msg, strlen(msg));

    read(fd, recvBuff, sizeof(recvBuff));
    printf("aaa%c\n", recvBuff[1]);

    if (recvBuff[0] == '1' && recvBuff[1] == 'N')
    {
        printf("Given name already exists\n");
        return;
    }

    while (1)
    {
        recv(fd, recvBuff, sizeof(recvBuff), 0);
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
    memset(&serv_addr, '0', sizeof(serv_addr));

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
    memset(&serv_addr, '0', sizeof(serv_addr));

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

