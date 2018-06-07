
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/epoll.h>


char** clients;
int numOfClients = 0;


void initLocal(char* path)
{
    perror(NULL);

    int listenfd = 0;
    int connfd;
    struct sockaddr_un serv_addr;
    time_t ticks;

    char sendBuff[1025];
    char readBuff[128];

    listenfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    memset(readBuff, '0', sizeof(readBuff));

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, path);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);

    while(1)
    {
        perror(NULL);
        connfd = accept(listenfd, (struct sockaddr*)NULL, SOCK_NONBLOCK);

        if (connfd != -1) recv(connfd, readBuff, strlen(readBuff), MSG_DONTWAIT);
        if (readBuff[0] == '1')
        {
            char* read2 = calloc(1024, sizeof(char));
            strcpy(read2, readBuff);

            char* name = strtok(read2 + sizeof(char), "0");
            printf("%s %d\n", name, numOfClients);
            perror("if");
            int flag = 1;
            for (int i=0; i<numOfClients; i++)
            {
                printf("%s\n", clients[i]);
                if (!strcmp(clients[i], name))
                {
                    flag = 0;
                }
            }
            if (flag == 1)
            {
                clients[numOfClients] = name;
                numOfClients++;
                write(connfd, "1Y", 2);
            }
            else
            {
                write(connfd, "1N", 2);
            }

        }
        readBuff[0] = 'A';
        perror("write");


        if (connfd != -1) close(connfd);
        sleep(1);
    }
}



void initNet(int port)
{
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    char readBuff[128];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    memset(readBuff, '0', sizeof(readBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);


    while(1)
    {
        perror(NULL);
        int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        recv(connfd, readBuff, strlen(readBuff), MSG_DONTWAIT);
        if (readBuff[0] == '1')
        {
            char* read2 = calloc(1024, sizeof(char));
            strcpy(read2, readBuff);

            char* name = strtok(read2 + sizeof(char), "0");
            printf("%s %d\n", name, numOfClients);
            perror("if");
            int flag = 1;
            for (int i=0; i<numOfClients; i++)
            {
                printf("%s\n", clients[i]);
                if (!strcmp(clients[i], name))
                {
                    flag = 0;
                }
            }
            if (flag == 1)
            {
                clients[numOfClients] = name;
                numOfClients++;
                write(connfd, "1Y", 2);
            }
            else
            {
                write(connfd, "1N", 2);
            }

        }
        readBuff[0] = 'A';

        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuff, strlen(sendBuff));

        close(connfd);
        sleep(1);
    }
}


void monitor()
{
    struct epoll_event event;
    int poll = epoll_create1(0);

//    event.data.fd =
}



int main(int argc, char** argv)
{
    clients = calloc(20, sizeof(char*));
//    initNet(atoi(argv[1]));

    initLocal(argv[1]);

    return 0;
}