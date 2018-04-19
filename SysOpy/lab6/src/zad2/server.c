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
    struct mq_attr stats;
    stats.mq_msgsize = MAX_MSG_LENGTH;
    stats.mq_flags = 0;
    stats.mq_curmsgs = 0;
    stats.mq_maxmsg = 10;

    mqd_t queue = mq_open(SERVER, O_RDONLY | O_CREAT, 0666, &stats);

    return queue;
}



char* getDate()
{
    time_t now;
    time(&now);
    char* result = calloc(50, sizeof(char));
    snprintf(result, MAX_MSG_LENGTH, "%s", ctime(&now));
    return result;
}



char* prepareMessage(int mode, char* arg)
{
    char* senderBuf = calloc(MAX_MSG_LENGTH, sizeof(char));
    char* modeString = calloc(2, sizeof(char));

    snprintf(modeString, 2, "%d", mode);
    strcpy(senderBuf, modeString);
    strcat(senderBuf, "|");
    strcat(senderBuf, SERVER);
    strcat(senderBuf, "|");
    strcat(senderBuf, arg);

    free(modeString);
    return senderBuf;
}



void receive(mqd_t que)
{
    mqd_t* clients = calloc(0, sizeof(mqd_t));
    int numberOfClients = 0;

    while (1)
    {
        char* buffer = calloc(MAX_MSG_LENGTH, sizeof(char));
        mq_receive(que, buffer, MAX_MSG_LENGTH, NULL);

        char bufferCopy[MAX_MSG_LENGTH];
        strcpy(bufferCopy, buffer);

        struct mq_attr stats;
        stats.mq_msgsize = MAX_MSG_LENGTH;
        stats.mq_flags = 0;
        stats.mq_curmsgs = 0;
        stats.mq_maxmsg = 10;

        strtok(bufferCopy, "|");
        char* clientQueueName = strtok(NULL, "|");

        mqd_t clientQueue = mq_open(clientQueueName, O_WRONLY, 0644, &stats);

        usleep(10000);

        char serverQueue[20];
        snprintf(serverQueue, 20, "%d", que);

        switch(buffer[0] - '1' + 1)
        {
            case INIT:
            {
                printf("init\t%s", getDate());

                clients = realloc(clients, sizeof(mqd_t)*(numberOfClients+1));
                clients[numberOfClients] = clientQueue;
                numberOfClients++;

                mq_send(clientQueue,  prepareMessage(INIT, ""), MAX_MSG_LENGTH, 3);

                break;
            }

            case CALC:
            {
                printf("calc\t%s", getDate());

                int numbers[2];
                char bufferCopy[MAX_MSG_LENGTH];
                strcpy(bufferCopy, buffer);

                strtok(bufferCopy, "|");
                strtok(NULL, "|");
                char* expression = strtok(NULL, "\n");

                char op = *(strpbrk(expression, "+-/*"));
                numbers[0] = atoi(strtok(expression, " +-*/\n"));
                numbers[1] = atoi(strtok(NULL, "\n"));

                double result;
                switch(op)
                {
                    case '+': { result = numbers[0] + numbers[1]; break; }
                    case '-': { result = numbers[0] - numbers[1]; break; }
                    case '*': { result = numbers[0] * numbers[1]; break; }
                    case '/': { result = (double)numbers[0] / (double)numbers[1]; break; }
                }

                char out[20];
                snprintf(out, MAX_MSG_LENGTH, "%f", result);

                mq_send(clientQueue, prepareMessage(CALC, out), MAX_MSG_LENGTH, 3);

                break;
            }

            case TIME:
            {
                printf("time\t%s",  getDate());

                char out[40];
                snprintf(out, MAX_MSG_LENGTH, "%s", getDate());

                mq_send(clientQueue, prepareMessage(TIME, out), MAX_MSG_LENGTH, 3);

                break;
            }

            case END:
            {
                printf("end\t%s",  getDate());

                for (int i=0; i<numberOfClients; i++)
                {
                    mq_close(clients[i]);
                }

                mq_unlink(SERVER);

                return;
            }

            case MIRROR:
            {
                printf("mirror\t%s", getDate());

                char bufferCopy[MAX_MSG_LENGTH];
                strcpy(bufferCopy, buffer);
                strtok(bufferCopy, "|");
                strtok(NULL, "|");
                char* sentence = strtok(NULL, "\n");
                char* result = calloc(strlen(sentence) + 1, sizeof(char));

                for (int i=strlen(sentence) - 1; i>=0; i--)
                {
                    strcat(result, (char[2]) {sentence[i], '\0'});
                }
                result[strlen(sentence)] = '\0';

                mq_send(clientQueue, prepareMessage(MIRROR, result), MAX_MSG_LENGTH, 3);

                free(result);

                break;
            }

            default:
            {
                printf("Unknown msg type\n");
                exit(-1);
            }

        }
        free(buffer);
    }
}



int main()
{
    mqd_t queue = init();
    receive(queue);
}