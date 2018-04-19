
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>


#include "properties.h"



struct queueInfo
{
    char* name;
    mqd_t queue;
};



char* getRandomName()
{
    int length = rand()%10 + 5;
    char* word = calloc(length + 2, sizeof(char));

    for (int i=1; i<length; i++)
    {
        word[i] = (char) (rand() % ('z' - 'a') + 'a');
    }

    word[length] = '\0';
    word[0] = '/';

    return word;
}



struct queueInfo init()
{
    srand(time(0));

    char* name = getRandomName();
    struct mq_attr stats;
    stats.mq_msgsize = MAX_MSG_LENGTH;
    stats.mq_flags = 0;
    stats.mq_curmsgs = 0;
    stats.mq_maxmsg = 10;

    mqd_t queue = mq_open(name, O_RDWR | O_CREAT, 0666, &stats);

    struct queueInfo info = {name, queue};

    return info;
}



char* prepareMessage(int mode, char* queueName, char* arg)
{
    char* senderBuf = calloc(MAX_MSG_LENGTH, sizeof(char));
    char* modeString = calloc(2, sizeof(char));

    snprintf(modeString, 2, "%d", mode);
    strcpy(senderBuf, modeString);
    strcat(senderBuf, "|");
    strcat(senderBuf, queueName);
    strcat(senderBuf, "|");
    strcat(senderBuf, arg);

    free(modeString);
    return senderBuf;
}



mqd_t connect(struct queueInfo info)
{
    struct mq_attr stats;
    stats.mq_msgsize = MAX_MSG_LENGTH;
    stats.mq_flags = 0;
    stats.mq_curmsgs = 0;
    stats.mq_maxmsg = 10;

    mqd_t serverQueue = mq_open(SERVER, O_WRONLY, 0666, &stats);
    char buf[MAX_MSG_LENGTH];

    mq_send(serverQueue, prepareMessage(INIT, info.name, ""), MAX_MSG_LENGTH, 3);

    while (!mq_receive(info.queue, buf, MAX_MSG_LENGTH, NULL));
    printf("Connected!\n");

    return serverQueue;
}



void sendMessages(FILE* fd, mqd_t serverQueue, struct queueInfo info)
{
    size_t length = MAX_MSG_LENGTH;
    char* arguments[2];

    char queue[20];
    snprintf(queue, 20, "%s", info.name);

    while(1)
    {
        char* buffer = calloc(MAX_MSG_LENGTH, sizeof(char));
        getline(&buffer, &length, fd);

        char* bufferCopy = calloc(MAX_MSG_LENGTH, sizeof(char));
        strcpy(bufferCopy, buffer);

        arguments[0] = strtok(bufferCopy, " \n");
        arguments[1] = strtok(NULL, "\n");

        if (arguments[0] == NULL) continue;


        if (!strcmp(arguments[0], "TIME"))
        {
            mq_send(serverQueue, prepareMessage(TIME, info.name, ""), MAX_MSG_LENGTH, 3);

            while (!mq_receive(info.queue, bufferCopy, MAX_MSG_LENGTH, NULL));

            strtok(bufferCopy, "|");
            strtok(NULL, "|");
            printf("%s\n", strtok(NULL, "\n"));
        }

        else if (!strcmp(arguments[0], "END"))
        {
            mq_send(serverQueue, prepareMessage(END, info.name, ""), MAX_MSG_LENGTH, 3);

            usleep(100);

            mq_close(info.queue);
            mq_unlink(info.name);

            break;
        }

        else if (!strcmp(arguments[0], "CALC"))
        {
            char* senderBuf = calloc(MAX_MSG_LENGTH, sizeof(char));

            mq_send(serverQueue, prepareMessage(CALC, info.name, arguments[1]), MAX_MSG_LENGTH, 3);

            while (!mq_receive(info.queue, senderBuf, MAX_MSG_LENGTH, NULL));

            strtok(senderBuf, "|");
            strtok(NULL, "|");
            printf("%s\n", strtok(NULL, "\n"));

            free(senderBuf);

        }

        else if (!strcmp(arguments[0], "MIRROR"))
        {
            char* senderBuf = calloc(MAX_MSG_LENGTH, sizeof(char));

            mq_send(serverQueue, prepareMessage(MIRROR, info.name, arguments[1]), MAX_MSG_LENGTH, 3);

            while (!mq_receive(info.queue, senderBuf, MAX_MSG_LENGTH, NULL));

            strtok(senderBuf, "|");
            strtok(NULL, "|");
            printf("%s\n", strtok(NULL, "\n"));

            free(senderBuf);
        }

        free(buffer);
        free(bufferCopy);
    }
}


void sender(int argc, char** argv)
{
    struct queueInfo info = init();
    mqd_t serverQueue = connect(info);

    if (argc == 1)
    {
        sendMessages(stdin, serverQueue, info);
    }
    else
    {
        FILE* file = fopen(argv[1], "r");
        sendMessages(file, serverQueue, info);
    }
}


int main(int argc, char** argv)
{
    sender(argc, argv);
}

