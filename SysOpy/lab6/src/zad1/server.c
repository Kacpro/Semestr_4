
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

#include "properties.h"



int init()
{
    key_t key = ftok("/home", KEY_CHAR);
    int queue = msgget(key, IPC_CREAT | 0622);
    printf("%d \n", key);
    perror(NULL);
    return queue;
}


struct msgbuf {
    long mtype;
    pid_t pid;
    int key;
    char mtext[MAX_MSG_LENGTH];
};


struct client
{
    pid_t pid;
    int clientID;
    int queue;
};


void receive(int queue)
{
    struct msgbuf msg;
    int clientID = 0;
    struct client* clients = calloc(0, sizeof(struct client));

    while (1)
    {
        msgrcv(queue, &msg, MAX_MSG_LENGTH, 0, MSG_NOERROR);
        if (!msg.mtype) continue;

        switch(msg.mtype)
        {
            case INIT:
            {
                printf("init\n");

                int clientQueue = msgget(msg.key, 0);

                clientID++;
                realloc(clients, sizeof(struct client) * clientID);
                clients[clientID - 1].pid = msg.pid;
                clients[clientID - 1].queue = queue;
                clients[clientID - 1].clientID = clientID;

                msg.mtype = clientID;
                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);
                perror(NULL);

                break;
            }

            case CALC:
            {
                printf("calc\n");
                break;
            }

            case TIME:
            {
                printf("time\n");
                break;
            }

            case END:
            {
                printf("end\n");
                break;
            }

            default:
            {
                printf("Unknown msg type\n");
                exit(-1);
            }
        }
    }
}


int main()
{
    int queue = init();
    receive(queue);
}