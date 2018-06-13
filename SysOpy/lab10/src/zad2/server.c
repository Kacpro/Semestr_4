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
#include <poll.h>
#include <errno.h>
#include <setjmp.h>
#include <stddef.h>


struct node
{
    char* name;
    struct sockaddr addr;
    int num;
    int type;
};


int numOfClients = 0;
int maxEvenets = 100;
int clusterSize = 20;
int socPoll;
int becauseIDontKnowHowToDoItDifferently = 0;


struct node* clients;


int initLocal(char* path)
{
    int listenfd = 0;
    struct sockaddr_un serv_addr;

    listenfd = socket(AF_UNIX, SOCK_DGRAM, 0);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, path);

//    size_t size = (offsetof (struct sockaddr_un, sun_path)
//    + strlen (serv_addr.sun_path));

//    unlink(path);
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr));

    return listenfd;
}


int initNet(int port)
{
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    int option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    return listenfd;
}

static int netFd;
static int localFd;

struct soc
{
    int port;
    char* path;
};


void* monitor(void* arg) {

    struct soc soc = *(struct soc*)arg;
    socPoll = epoll_create1(0);

    struct epoll_event event;

    netFd = initNet(soc.port);
    event.data.fd = netFd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(socPoll, EPOLL_CTL_ADD, netFd, &event);

//    perror("net");

    localFd = initLocal(soc.path);
//    perror("init");

    event.data.fd = localFd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(socPoll, EPOLL_CTL_ADD, localFd, &event);

//    perror("local");

    struct epoll_event *events = calloc(maxEvenets, sizeof(struct epoll_event));

    char readBuff[128];

    while (1)
    {
 //       perror("while");
        int n = epoll_wait(socPoll, events, maxEvenets, -1);
        for (int i = 0; i < n; i++)
        {
            for (int j=0; j<128; j++) readBuff[j] = '\0';

            if (events[i].data.fd == netFd || events[i].data.fd == localFd)
            {
//                int clientFD = accept(events[i].data.fd, (struct sockaddr *) NULL, 0);
//                if (clientFD == -1) continue;

                struct sockaddr addr;
                socklen_t addrSize = sizeof(struct sockaddr);
//                perror("a");
                recvfrom(events[i].data.fd, readBuff, 128, MSG_PEEK, &addr, &addrSize);
                if (readBuff[0] != '2')
                {
                    recvfrom(events[i].data.fd, readBuff, 128, 0, &addr, &addrSize);
                }
                else
                {
                    continue;
                }

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
                            if (clients[j].num == -1)
                            {
                                clients[j].name = name;
                                clients[j].addr = addr;
                                clients[j].type = events[i].data.fd == localFd ? 1 : 0;
                                clients[j].num = numOfClients;
                                break;
                            }
                        }
                        printf("Node up: %d (%s)\n", numOfClients, name);
                        numOfClients++;

                        sendto(events[i].data.fd, "1Y", 2, 0, &addr, addrSize);
//                        perror("sendto");
                    }
                    else
                    {
                        sendto(events[i].data.fd, "1N", 2, 0, &addr, addrSize);
                    }
                }
                else if (readBuff[0] == '3')
                {
                    printf("Result: %s, Node: %d\n", strtok(readBuff + 2 * sizeof(char), "\0"), (int)(readBuff[1]) - 1);
                }
                else if (readBuff[0] == '2')
                {
//                    printf(">>pong\n");
                    int n = sendto(events[i].data.fd, "2", 1, 0, &addr, addrSize);
//                    printf("%d\n", n);
//                    perror("sent");
                }
                else
                {
     //               printf("Unknown message\n");
                }


            }
            else
            {
//                perror("else");
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


void* ping(void* arg)
{
    while (1)
    {
        sleep(1);
        for (int i = 0; i < clusterSize; i++)
        {
                if (clients[i].num != -1)
                {
                    ssize_t res = sendto(clients[i].type?localFd:netFd, "2", 1, 0, &clients[i].addr, sizeof(struct sockaddr));

                    char *buf = calloc(128, sizeof(char));
                    socklen_t size = sizeof(clients[i].addr);
 //                   printf("%ld\n", res);
//                    perror("send_ping");
                    recvfrom(clients[i].type?localFd:netFd, buf, 128,0 , &clients[i].addr, &size);
//                    perror("recv");
//                    printf("%ld\n", res);
                    if (errno == EAGAIN)
                    {
                        printf("Node down: %d\n", clients[i].num);
                        clients[i].num = -1;
                        clients[i].name = "";
                        clients[i].type = -1;
                        errno = 0;
                    }
                    if (buf[0] != '2')
                    {
                        char* buf2 = calloc(130, sizeof(char));
                        strcpy(buf2, "5");
                        strcat(buf2, buf);
                        sendto(clients[i].type?localFd:netFd, buf2, 130, 0, &clients[i].addr, sizeof(struct sockaddr));
                    }
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

        int p;
        while (clients[p = rand()%clusterSize].num == -1);

        char pos[] = {(char)(p + 1), '\0'};
        strcat(msg, pos);
        strcat(msg, buf);

        sendto(netFd, msg, 130, 0, &clients[p].addr, sizeof(clients[p].addr));
        perror("send calc");
        free(msg);
        usleep(1000);
    }
}


void signalHandling(int sig)
{
    for (int i=0; i<clusterSize; i++)
    {
     //   close(clients[i].fd);
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
        clients[i].num = -1;
    }

    signal(SIGINT, signalHandling);

    pthread_t watcher, pinger, sender;

    struct soc* s = calloc(1, sizeof(struct soc));
    s->path = argv[2];
    s->port = atoi(argv[1]);

    pthread_create(&watcher, NULL, monitor, s);
//    pthread_create(&pinger, NULL, ping, s);
    pthread_create(&sender, NULL, calc, NULL);



    pthread_join(watcher, NULL);
 //   pthread_join(pinger, NULL);
    pthread_join(sender, NULL);

    return 0;
}