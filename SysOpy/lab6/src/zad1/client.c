
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <signal.h>


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



key_t connect(struct queueInfo info)
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
    return key;
}



void sendMessages(FILE* fd, key_t key)
{
    int queue = msgget(key, 0);
    struct msgbuf msg;
    msg.pid = getpid();
    msg.key = 0;
    char* buffer = calloc(MAX_MSG_LENGTH, sizeof(char));
    size_t length = MAX_MSG_LENGTH;
    char* arguments[2];

    while(1)
    {
        getline(&buffer, &length, fd);

        arguments[0] = strtok(buffer, " \n");
        arguments[1] = strtok(buffer, " \n");

        if (arguments[0] == NULL) continue;


        if (!strcmp(arguments[0], "TIME"))
        {
            printf("time\n");
            msg.mtype = TIME;
            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
        }

        else if (!strcmp(arguments[0], "END"))
        {
            printf("end\n");
            msg.mtype = END;
            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
            break;
        }

        else if (!strcmp(arguments[0], "CALC"))
        {
            printf("calc\n");
            msg.mtype = CALC;
            strcpy(msg.mtext, arguments[1]);
            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
        }

        else if (!strcmp(arguments[0], "MIRROR"))
        {
            printf("mirror\n");
            msg.mtype = MIRROR;
            strcpy(msg.mtext, arguments[1]);
            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
        }
    }
}


void sender(int argc, char** argv)
{
    struct queueInfo info = init();
    key_t key = connect(info);

    if (argc == 1)
    {
        sendMessages(stdin, key);
    }
    else
    {
        FILE* file = fopen(argv[1], "r");
        sendMessages(file, key);
    }
}


int main(int argc, char** argv)
{
    sender(argc, argv);
}


