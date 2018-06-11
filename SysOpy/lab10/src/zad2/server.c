#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <signal.h>


struct node
{
    char* name;
    int fd;
    int num;
};


int numOfClients = 0;
int maxEvenets = 100;
int clusterSize = 20;
int socPoll;


struct node* clients;


int initLocal(char* path)
{
    int listenfd = 0;
    struct sockaddr_un serv_addr;

    listenfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, path);

    unlink(path);
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
    socPoll = epoll_create1(0);

    struct epoll_event event;

    int netFd = initNet(soc.port);
    event.data.fd = netFd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(socPoll, EPOLL_CTL_ADD, netFd, &event);

    int localFd = initLocal(soc.path);
    event.data.fd = localFd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(socPoll, EPOLL_CTL_ADD, localFd, &event);

    struct epoll_event *events = calloc(maxEvenets, sizeof(struct epoll_event));

    char readBuff[128];

    while (1)
    {
        int n = epoll_wait(socPoll, events, maxEvenets, -1);
        for (int i = 0; i < n; i++)
        {
            for (int j=0; j<128; j++) readBuff[j] = '\0';

            if (events[i].data.fd == netFd || events[i].data.fd == localFd)
            {
                int clientFD = accept(events[i].data.fd, (struct sockaddr *) NULL, 0);
                if (clientFD == -1) continue;

                recv(clientFD, readBuff, 128, 0);
                if (readBuff[0] == '1')
                {
                    char *read2 = calloc(128, sizeof(char));
                    strcpy(read2, readBuff);

                    char *name = strtok(read2 + sizeof(char), "\0");

                    int flag = 1;
                    for (int j = 0; j < clusterSize; j++)
                    {
                        if (!strcmp(clients[j].name, name))
                        {
                            flag = 0;
                        }
                    }

                    if (flag == 1)
                    {
                        for (int j=0; j<clusterSize; j++)
                        {
                            if (clients[j].fd == -1)
                            {
                                clients[j].name = name;
                                clients[j].fd = clientFD;
                                clients[j].num = numOfClients;
                                break;
                            }
                        }
                        printf("Node up: %d (%s)\n", numOfClients, name);
                        numOfClients++;

                        event.data.fd = clientFD;
                        event.events = EPOLLIN | EPOLLET;
                        epoll_ctl(socPoll, EPOLL_CTL_ADD, clientFD, &event);

                        send(clientFD, "1Y", 2, MSG_DONTWAIT);
                    }
                    else
                    {
                        write(clientFD, "1N", 2);
                        close(clientFD);
                    }
                }
                else
                {
                    printf("Unknown message");
                }

            }
            else
            {
                recv(events[i].data.fd, readBuff, 128, 0);
                if (readBuff[0] == '3')
                {
                    printf("Result: %s, Node: %d\n", strtok(readBuff + 2 * sizeof(char), "\0"), (int)(readBuff[1]) - 1);
                }
            }
        }

        usleep(1000);
    }
}


void* ping(void* a)
{
    struct epoll_event event;
    while (1)
    {
        sleep(1);
        for (int i = 0; i < clusterSize; i++)
        {
            if (clients[i].fd != -1 && send(clients[i].fd, "2", 2, MSG_NOSIGNAL) == -1)
            {
                printf("Node down: %d\n", clients[i].num);

                event.data.fd = clients[i].fd;
                event.events = EPOLLIN | EPOLLET;
                epoll_ctl(socPoll, EPOLL_CTL_DEL, clients[i].fd, &event);

                close(clients[i].fd);
                clients[i].fd = -1;
                clients[i].name = "";
                clients[i].num = -1;
            }
        }
    }
}


void* calc(void* a)
{
    srand(time(0));
    char* buf = calloc(128, sizeof(char));
    size_t size = 127;
    while(1)
    {
        char* msg = calloc(130, sizeof(char));
        strcat(msg, "3");
        getline(&buf, &size, stdin);

        int target;
        int p;
        while ((target = clients[p = rand()%clusterSize].fd) == -1);

        char pos[] = {(char)(p + 1), '\0'};
        strcat(msg, pos);
        strcat(msg, buf);

        write(target, msg, 130);
        free(msg);
        usleep(1000);
    }
}


void signalHandling(int sig)
{
    for (int i=0; i<clusterSize; i++)
    {
        close(clients[i].fd);
    }
    free(clients);
    printf("\n");
    exit(0);
}


int main(int argc, char** argv)
{
    clients = calloc(clusterSize, sizeof(struct node));
    for (int i=0; i<clusterSize; i++)
    {
        clients[i].name = "";
        clients[i].fd = -1;
        clients[i].num = -1;
    }

    signal(SIGINT, signalHandling);

    pthread_t watcher, pinger, sender;

    struct soc* s = calloc(1, sizeof(struct soc));
    s->path = argv[2];
    s->port = atoi(argv[1]);

    pthread_create(&watcher, NULL, monitor, s);
    pthread_create(&pinger, NULL, ping, NULL);
    pthread_create(&sender, NULL, calc, NULL);

    pthread_join(watcher, NULL);
    pthread_join(pinger, NULL);
    pthread_join(sender, NULL);

    return 0;
}