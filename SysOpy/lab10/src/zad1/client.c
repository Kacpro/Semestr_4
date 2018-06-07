#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/un.h>


void initNet(char* address, char* name)
{

    char* addr = strtok(address, ":");
    int port = atoi(strtok(NULL, "\0\n"));


    printf("%s %d\n", addr, port);

    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;


    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET, addr, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror(NULL);
        printf("\n Error : Connect Failed \n");
        return;
    }


    char* msg = calloc(100, sizeof(char));
    strcat(msg, "1");
    strcat(msg, name);
    write(sockfd, msg, strlen(msg));

    read(sockfd, recvBuff, sizeof(recvBuff));
    printf("%c\n", recvBuff[1]);
    if (recvBuff[0] == '1' && recvBuff[1] == 'N')
    {
        printf("Given name already exists\n");
        return;
    }

    while (1)
    {
        recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
        recvBuff[n] = 0;

    }
}


void initLocal(char* path, char* name)
{
    perror(NULL);
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_un serv_addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    strcpy(serv_addr.sun_path, path);
    serv_addr.sun_family = AF_UNIX;

    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    char* msg = calloc(100, sizeof(char));
    strcat(msg, "1");
    strcat(msg, name);
    write(sockfd, msg, strlen(msg));

    read(sockfd, recvBuff, sizeof(recvBuff));
    printf("%c\n", recvBuff[1]);

    if (recvBuff[0] == '1' && recvBuff[1] == 'N')
    {
        printf("Given name already exists\n");
        return;
    }

    while (1)
    {
        recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
        recvBuff[n] = 0;

    }
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

