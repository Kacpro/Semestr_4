
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <mqueue.h>


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
        word[i] = (char)(rand()%(122-97) + 97);
    word[length] = '\0';
    word[0] = '/';
    return word;
}

struct queueInfo init()
{
    srand(time(0));
    perror("init");
//    key_t key = ftok("/home", (char)rand()%(256));
//    int queue = msgget(key, IPC_CREAT | 0622);

    struct mq_attr stats;
    stats.mq_msgsize = MAX_MSG_LENGTH;
    stats.mq_flags = 0;
    stats.mq_curmsgs = 0;
    stats.mq_maxmsg = 10;

    char* name = getRandomName();
    printf("name: %s\n", name);
    mqd_t queue = mq_open(name, O_RDWR | O_CREAT, 0666, &stats);
    struct queueInfo info = {name, queue};
    perror("init end");

    mq_getattr(queue, &stats);
    printf("%ld\n", stats.mq_msgsize);
    return info;
}


//struct msgbuf {
//    long mtype;
//    pid_t pid;
//    int key;
//    char mtext[MAX_MSG_LENGTH];
//};



mqd_t connect(struct queueInfo info)
{
//    key_t key = ftok("/home", KEY_CHAR);
//    int queueName = msgget(key, 0);
//    struct msgbuf msg;
//    msg.mtype = INIT;
//    msg.pid = getpid();
//    msg.key = info.key;
//    msgsnd(queueName, &msg, MAX_MSG_LENGTH, 0);

//    while(!msgrcv(info.queueName, &msg, MAX_MSG_LENGTH, 0, MSG_NOERROR));
//    printf("Connected!\tQueueID = %d\n", info.queueName);
//    return key;

    perror("connect start");

    struct mq_attr stats;
    stats.mq_msgsize = MAX_MSG_LENGTH;
    stats.mq_flags = 0;
    stats.mq_curmsgs = 0;
    stats.mq_maxmsg = 20;

    mqd_t serverQueue = mq_open(SERVER, O_WRONLY, 0666, &stats);

    mq_getattr(serverQueue, &stats);
    printf("%ld\n", stats.mq_msgsize);

    char buf[MAX_MSG_LENGTH];
    char mode[2];
    char* clientQueueName = calloc(20, sizeof(char));

    snprintf(mode, 2, "%d", INIT);
    snprintf(clientQueueName, 20, "%s", info.name);

    printf("%d\n", info.queue);

    strcpy(buf, mode);
    strcat(buf, "|");
    strcat(buf, clientQueueName);
    strcat(buf, "|");

    printf("buf: %s\n", buf);

    perror("connect");

    if (mq_send(serverQueue, buf, MAX_MSG_LENGTH, 3) == -1)
    {
        perror("error");
        printf("Problem with sending msg to server");
        exit(-1);
    };
    while (!mq_receive(info.queue, buf, MAX_MSG_LENGTH, NULL));
    printf("Connected!\tQueueID = %d\n", info.queue);

    perror("connect end");
    return serverQueue;
}



void sendMessages(FILE* fd, mqd_t serverQueue, struct queueInfo info)
{
//    int queue = msgget(key, 0);
//    struct msgbuf msg;
//    msg.pid = getpid();
//    msg.key = 0;
//    char* buffer = calloc(MAX_MSG_LENGTH, sizeof(char));
//    buffer = buffer;

    size_t length = MAX_MSG_LENGTH;
    char* arguments[2];
    char mode[2];

    char queue[20];
    snprintf(queue, 20, "%s", info.name);

    while(1)
    {
        char* buffer = calloc(MAX_MSG_LENGTH, sizeof(char));        //free
        getline(&buffer, &length, fd);

        char* bufferCopy = calloc(MAX_MSG_LENGTH, sizeof(char));
        strcpy(bufferCopy, buffer);

        arguments[0] = strtok(bufferCopy, " \n");
        arguments[1] = strtok(NULL, "\n");

        printf("arg1: %s\n", arguments[1]);

        char* dupa = arguments[1];

        if (arguments[0] == NULL) continue;


        if (!strcmp(arguments[0], "TIME"))
        {
            perror("time start");
//            msg.mtype = TIME;
//            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
            snprintf(mode, 2, "%d", TIME);
            strcpy(bufferCopy, mode);
            strcat(bufferCopy, "|");
            strcat(bufferCopy, queue);
            strcat(bufferCopy, "|");

            printf("buffer: %s\n", bufferCopy);

            mq_send(serverQueue, bufferCopy, MAX_MSG_LENGTH, 3);

//            while(!msgrcv(info.queue, &msg, MAX_MSG_LENGTH, 0, MSG_NOERROR));
//            printf("%s", msg.mtext);

            while (!mq_receive(info.queue, bufferCopy, MAX_MSG_LENGTH, NULL));

            strtok(bufferCopy, "|");
            strtok(NULL, "|");
            printf("%s\n", strtok(NULL, "\n"));
        }

        else if (!strcmp(arguments[0], "END"))
        {
//            msg.mtype = END;
//            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);
            //          struct msqid_ds *buf = NULL;
            snprintf(mode, 2, "%d", END);
            strcat(bufferCopy, mode);
            strcat(bufferCopy, "|");
            strcat(bufferCopy, queue);
            strcat(bufferCopy, "|");
            mq_send(serverQueue, bufferCopy, MAX_MSG_LENGTH, 3);


//            msgctl(info.queue, IPC_RMID, NULL);
            while (!mq_receive(info.queue, bufferCopy, MAX_MSG_LENGTH, NULL));

            mq_close(info.queue);
            mq_unlink(info.name);

            break;
        }

        else if (!strcmp(arguments[0], "CALC"))
        {
//            msg.mtype = CALC;
//            strcpy(msg.mtext, arguments[1]);
//            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);

            snprintf(mode, 2, "%d", CALC);

            char* senderBuf = calloc(MAX_MSG_LENGTH, sizeof(char));
            strcpy(senderBuf, bufferCopy);

            strcpy(senderBuf, mode);

            printf("buffer: %s\n", senderBuf);

            strcat(senderBuf, "|");
            strcat(senderBuf, queue);
            strcat(senderBuf, "|");

            strcat(senderBuf, arguments[1]);
            mq_send(serverQueue, senderBuf, MAX_MSG_LENGTH, 3);


            while (!mq_receive(info.queue, senderBuf, MAX_MSG_LENGTH, NULL));

            strtok(senderBuf, "|");
            strtok(NULL, "|");
            printf("%s\n", strtok(NULL, "\n"));

            free(senderBuf);

        }

        else if (!strcmp(arguments[0], "MIRROR"))
        {
//            msg.mtype = MIRROR;
//            strcpy(msg.mtext, arguments[1]);
//            msgsnd(queue, &msg, MAX_MSG_LENGTH, 0);

            snprintf(mode, 2, "%d", MIRROR);
            strcat(bufferCopy, mode);



            strcat(bufferCopy, "|");
            strcat(bufferCopy, queue);
            strcat(bufferCopy, "|");
            strcat(bufferCopy, arguments[1]);
            mq_send(serverQueue, bufferCopy, MAX_MSG_LENGTH, 3);

 //           while(!msgrcv(info.queue, &msg, MAX_MSG_LENGTH, 0, MSG_NOERROR));
 //           printf("%s\n", msg.mtext);
            while (!mq_receive(info.queue, bufferCopy, MAX_MSG_LENGTH, NULL));
            printf("%s\n", bufferCopy+22*sizeof(char));
        }

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

