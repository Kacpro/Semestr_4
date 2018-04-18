
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>


#include "properties.h"



struct queueInfo
{
    key_t key;
    int queue;
};

struct queueInfo init()
{
    srand(time(0));
    key_t key = ftok("/home", (char)rand()%(256));
    int queue = msgget(key, IPC_CREAT | 0622);
    struct queueInfo info = {key, queue};
    return info;
}


struct msgbuf {
    long mtype;
    pid_t pid;
    int key;
    char mtext[MAX_MSG_LENGTH];
};



void connect(struct queueInfo info)
{
    key_t key = ftok("/home", KEY_CHAR);
    printf("%d\n", key);
    int queue = msgget(key, 0);
    perror(NULL);
    struct msgbuf msg;
    msg.mtype = INIT;
    msg.pid = getpid();
    msg.key = info.key;
    msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
    perror(NULL);

    while(!msgrcv(info.queue, &msg, MAX_MSG_LENGTH, 0, MSG_NOERROR));
    perror(NULL);
    printf("Connected! ID = %d %d\n ", msg.mtype, info.queue);
}




int main()
{
    struct queueInfo info = init();
    connect(info);
}


