#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>

#include "properties.h"



mqd_t init()
{
//    key_t key = ftok("/home", KEY_CHAR);
//    int queue = msgget(key, IPC_CREAT | 0622);
//    return queue;


    struct mq_attr stats;
    stats.mq_msgsize = MAX_MSG_LENGTH;
    stats.mq_flags = 0;
    stats.mq_curmsgs = 0;
    stats.mq_maxmsg = 20;

 //   if(mq_unlink(SERVER) == 0) fprintf(stdout, "Message queue %s removed from system.\n", SERVER);

    perror("init");
    mqd_t queue = mq_open(SERVER, O_RDONLY | O_CREAT, 0666, &stats);

    perror("init");
    mq_getattr(queue, &stats);
    printf("%ld\n", stats.mq_msgsize);
    printf("%d\n", queue);


    return queue;
}


//struct msgbuf {
//    long mtype;
//    pid_t pid;
//    int key;
//    char mtext[MAX_MSG_LENGTH];
//};


//struct client
//{
//    pid_t pid;
//    int clientID;
//    int queue;
//};

char* getDate()
{
    time_t now;
    time(&now);
    char* result = calloc(50, sizeof(char));
    snprintf(result, MAX_MSG_LENGTH, "%s", ctime(&now));
    return result;
}


//int getQueue(pid_t pid, struct client* clients, int numberOfClients)
//{
//    for (int i=0; i<numberOfClients; i++)
//    {
//        if (clients[i].pid == pid) return clients[i].queue;
//    }
//    return -1;
//}


void receive(mqd_t que)
{
//    struct msgbuf msg;
//    int clientID = 0;
//    struct client* clients = calloc(0, sizeof(struct client));

//    int closeFlag = 0;


    while (1)
    {
//        msg.mtype = 0;
//        msgrcv(queue, &msg, MAX_MSG_LENGTH, 0,  IPC_NOWAIT);
//        if (!msg.mtype && closeFlag) break;
//        if (!msg.mtype) continue;
        perror("rec start");
   //     char* buffer = calloc(MAX_MSG_LENGTH, sizeof(char));            //free

        char buffer[MAX_MSG_LENGTH];
        mq_receive(que, buffer, MAX_MSG_LENGTH, NULL);
        perror("rec");



        strtok(buffer, "|");
        mqd_t clientQueue = atoi(strtok(NULL, "|"));

        usleep(10000);

        char mode[2];

        char queue[20];
        snprintf(queue, 20, "%d", que);

        printf("buf: %s", buffer);

        switch(buffer[0] - '1' + 1)
        {
            case INIT:
            {
                printf("init\t%s", getDate());

//                int clientQueue = msgget(msg.key, 0);

//                clientID++;
//                clients = realloc(clients, sizeof(struct client) * clientID);
//                clients[clientID - 1].pid = msg.pid;
//                clients[clientID - 1].queue = clientQueue;
//                clients[clientID - 1].clientID = clientID;

//                msg.mtype = clientID;
//                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);
                snprintf(mode, 2, "%d", INIT);
                strcat(buffer, mode);
                strcat(buffer, "|");
                strcat(buffer, queue);
                strcat(buffer, "|");
                mq_send(clientQueue, buffer, MAX_MSG_LENGTH, 3);

                break;
            }

            case CALC:
            {
                printf("calc\t%s", getDate());
                int numbers[2];

                strtok(buffer, "|");
                strtok(NULL, "|");
                char* expression = strtok(NULL, "|");

                char op = *(strpbrk(expression, "+-/*"));
                numbers[0] = atoi(strtok(expression, " +-*/\n"));
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


                char out[20];
                snprintf(out, MAX_MSG_LENGTH, "%f", result);
//                int clientQueue = getQueue(msg.pid, clients, clientID);
//                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);

                snprintf(mode, 2, "%d", CALC);
                strcat(buffer, mode);
                strcat(buffer, "|");
                strcat(buffer, queue);
                strcat(buffer, "|");
                strcat(buffer, out);
                mq_send(clientQueue, buffer, MAX_MSG_LENGTH, 3);



                break;
            }

            case TIME:
            {
                printf("time\t%s",  getDate());
                time_t now;
                time(&now);

//                int clientQueue = getQueue(msg.pid, clients, clientID);

                char out[20];
                snprintf(out, MAX_MSG_LENGTH, "%s", ctime(&now));
//                msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);

                snprintf(mode, 2, "%d", TIME);
                strcat(buffer, mode);
                strcat(buffer, "|");
                strcat(buffer, queue);
                strcat(buffer, "|");
                strcat(buffer, out);
                mq_send(clientQueue, buffer, MAX_MSG_LENGTH, 3);

                break;
            }

            case END:
            {
                printf("end\t%s",  getDate());
 //               closeFlag = 1;
                mq_close(clientQueue);

                ///////////////////////////////////////////////////// TODO closing all queues

                break;
            }

            case MIRROR:
            {
                printf("mirror\t%s", getDate());

                strtok(buffer, "|");
                strtok(NULL, "|");
                char* sentence = strtok(NULL, "|");

                char* result = calloc(strlen(sentence) + 1, sizeof(char));

                for (int i=strlen(sentence) - 1; i>=0; i--)
                {
                    strcat(result, (char[2]) {sentence[i], '\0'});
                }
                result[strlen(sentence)] = '\0';

//                int clientQueue = getQueue(msg.pid, clients, clientID);
//                snprintf(msg.mtext, MAX_MSG_LENGTH, "%s", result);
 //               msgsnd(clientQueue, &msg, MAX_MSG_LENGTH, 0);

                snprintf(mode, 2, "%d", MIRROR);
                strcat(buffer, mode);
                strcat(buffer, "|");
                strcat(buffer, queue);
                strcat(buffer, "|");
                strcat(buffer, result);
                mq_send(clientQueue, buffer, MAX_MSG_LENGTH, 3);


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
//    msgctl(queue, IPC_RMID, NULL);
}


int main()
{
    mqd_t queue = init();
    receive(queue);
}