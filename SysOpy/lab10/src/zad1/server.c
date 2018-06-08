
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/epoll.h>
#include <poll.h>
#include <pthread.h>


char** clients;
int* clientfd;
int numOfClients = 0;
int maxEvenets = 100;
int clusterSize = 20;


int initLocal(char* path)
{
    perror("local");

    int listenfd = 0;
    struct sockaddr_un serv_addr;

    listenfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, path);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);

    return listenfd;
}


int initNet(int port)
{
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 10);

    return listenfd;
}


struct soc
{
    int port;
    char* path;
};


void* monitor(void* arg) {

    struct soc soc = *(struct soc*)arg;

    struct epoll_event event;
    int poll = epoll_create1(0);

    perror("1");

    int netFd = initNet(soc.port);

    perror("2");

    event.data.fd = netFd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(poll, EPOLL_CTL_ADD, netFd, &event);

    perror("a");

    int localFd = initLocal(soc.path);
    event.data.fd = localFd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(poll, EPOLL_CTL_ADD, localFd, &event);

    struct epoll_event *events = calloc(maxEvenets, sizeof(struct epoll_event));

    char sendBuff[1025];
    char readBuff[128];

    memset(sendBuff, '0', sizeof(sendBuff));
    memset(readBuff, '0', sizeof(readBuff));


    while (1) {
        int n = epoll_wait(poll, events, maxEvenets, -1);
        for (int i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }

            if (events[i].data.fd == netFd || events[i].data.fd == localFd)
            {

                perror(NULL);
                int connfd = accept(events[i].data.fd, (struct sockaddr *) NULL, NULL);
                if (connfd == -1) continue;


                recv(connfd, readBuff, strlen(readBuff), 0);
                if (readBuff[0] == '1')
                {
                    char *read2 = calloc(1024, sizeof(char));
                    strcpy(read2, readBuff);

                    char *name = strtok(read2 + sizeof(char), "0");
                    printf("%s %d\n", name, numOfClients);
                    perror("if");
                    int flag = 1;
                    for (int i = 0; i < clusterSize; i++)
                    {
                        if (!strcmp(clients[i], name))
                        {
                            flag = 0;
                        }
                    }
                    if (flag == 1)
                    {
                        for (int i=0; i<clusterSize; i++)
                            if (clientfd[i] == -1)
                            {
                                clients[i] = name;
                                clientfd[i] = connfd;
                                break;
                            }
                        numOfClients++;
                        write(connfd, "1Y", 2);
                    }
                    else
                    {
                        write(connfd, "1N", 2);
                        close(connfd);
                    }

                    usleep(1000);
                }
            }
        }

    }
}


void* ping(void* a)
{
    while (1)
    {
        sleep(1);
        for (int i = 0; i < clusterSize; i++)
        {
            if (clientfd[i] != -1 && send(clientfd[i], "2", 2, MSG_NOSIGNAL) == -1)
            {
                printf("Node lost\n");
                clientfd[i] = -1;
                clients[i] = "";
            }
        }
    }
}



int main(int argc, char** argv)
{
    clients = calloc(clusterSize, sizeof(char*));
    clientfd = calloc(clusterSize, sizeof(int));
    for (int i=0; i<clusterSize; i++)
    {
        clients[i] = "";
        clientfd[i] = -1;
    }

    pthread_t watcher, pinger, sender;

    struct soc* s = calloc(1, sizeof(struct soc));
    s->path = argv[2];
    s->port = atoi(argv[1]);

    pthread_create(&watcher, NULL, monitor, s);
    pthread_create(&pinger, NULL, ping, NULL);

    while(1);

    return 0;
}