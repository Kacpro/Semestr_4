
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

#include "properties.h"



int init()
{
    key_t key = ftok("/home", KEY_CHAR);
    int queue = msgget(key, IPC_CREAT | 0622);
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

char* getDate()
{
    time_t now;
    time(&now);
    char* result = calloc(50, sizeof(char));
    snprintf(result, MAX_MSG_LENGTH, "%s", ctime(&now));
    return result;
}


int getQueue(pid_t pid, struct client* clients, int numberOfClients)
{
    for (int i=0; i<numberOfClients; i++)
    {
        if (clients[i].pid == pid) return clients[i].queue;
    }
    return -1;
}


void receive(int queue)
{
    struct msgbuf msg;
    int clientID = 0;
    struct client* clients = calloc(0, sizeof(struct client));

    int closeFlag = 0;

    while (1)
    {
        msg.mtype = 0;
        msgrcv(queue, &msg, MAX_MSG_LENGTH, 0,  IPC_NOWAIT);
        if (!msg.mtype && closeFlag) break;
        if (!msg.mtype) continue;
        usleep(10000);

        switch(msg.mtype)
        {
            case INIT:
            {
                printf("init\t%d\t%s", msg.pid, getDate());

                int clientQueue = msgget(msg.key, 0);

                clientID++;
                clients = realloc(clients, sizeof(struct client) * clientID);
                clients[clientID - 1].pid = msg.pid;
                clients[clientID - 1].queue = clientQueue;
                clients[clientID - 1].clientID = clientID;

                msg.mtype = clientID;
                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);

                break;
            }

            case CALC:
            {
                printf("calc\t%d\t%s", msg.pid, getDate());
                int numbers[2];
                char op = *(strpbrk(msg.mtext, "+-/*"));
                numbers[0] = atoi(strtok(msg.mtext, " +-*/\n"));
                numbers[1] = atoi(strtok(NULL, "\n"));


                double result;
                switch(op)
                {
                    case '+':
                    {
                        result = numbers[0] + numbers[1];
                        break;
                    }

                    case '-':
                    {
                        result = numbers[0] - numbers[1];
                        break;
                    }

                    case '*':
                    {
                        result = numbers[0] * numbers[1];
                        break;
                    }

                    case '/':
                    {
                        result = (double)numbers[0] / (double)numbers[1];
                        break;
                    }
                }


                snprintf(msg.mtext, MAX_MSG_LENGTH, "%f", result);
                int clientQueue = getQueue(msg.pid, clients, clientID);
                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);



                break;
            }

            case TIME:
            {
                printf("time\t%d\t%s", msg.pid, getDate());
                time_t now;
                time(&now);

                int clientQueue = getQueue(msg.pid, clients, clientID);
                snprintf(msg.mtext, MAX_MSG_LENGTH, "%s", ctime(&now));
                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);

                break;
            }

            case END:
            {
                printf("end\t%d\t%s", msg.pid, getDate());
                closeFlag = 1;
                break;
            }

            case MIRROR:
            {
                printf("mirror\t%d\t%s", msg.pid, getDate());
                char* result = calloc(strlen(msg.mtext) + 1, sizeof(char));

                for (int i=strlen(msg.mtext) - 1; i>=0; i--)
                {
                    strcat(result, (char[2]) {(char)msg.mtext[i], '\0'});
                }
                result[strlen(msg.mtext)] = '\0';

                int clientQueue = getQueue(msg.pid, clients, clientID);
                snprintf(msg.mtext, MAX_MSG_LENGTH, "%s", result);
                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);

                free(result);
                break;
            }

            default:
            {
                printf("Unknown msg type\n");
                exit(-1);
            }
        }
    }
 //   struct msqid_ds *buf;
    msgctl(queue, IPC_RMID, NULL);
}


int main()
{
    int queue = init();
    receive(queue);
}
