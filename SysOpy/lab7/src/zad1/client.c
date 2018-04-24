
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>


struct data
{
    int waitingClients;
    int barberIsSleeping;
    int queueLength;
    int clientStatus;
    int nextPid;
};


struct data* cMemPtr;
int semSet;
int queue;
int commonMemory;



struct pids
{
    pid_t pid;
    int code;
};


void clientLogic(int numberOfShaves)
{
    struct sembuf* sops = calloc(1, sizeof(struct sembuf));
    sops[0].sem_flg = 0;

    struct msgbuf
    {
        long mtype;
        struct pids pid;
        char mtext[0];
    };

    struct msgbuf msg;
    msg.mtype = 1;


    for (int i=0; i<numberOfShaves; i++)
    {
        while(cMemPtr->clientStatus != -1);

        sops[0].sem_num = 0;
        sops[0].sem_op = -1;
        semop(semSet, sops, 1);

        msg.pid.code = getpid()*(i+1);
        msg.pid.pid = getpid();

        if (cMemPtr->barberIsSleeping == 1)
        {
            cMemPtr->barberIsSleeping = 0;
            printf("Client wakes the barber up\n");
            msgsnd(queue, &msg, 10, 0);

            cMemPtr->nextPid = getpid() * (i+1);

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            while(cMemPtr->clientStatus != 1 || cMemPtr->nextPid != getpid()*(i+1));

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            printf("Client sits on the shaving chair\n");
            cMemPtr->clientStatus = 2;


            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);
            while(cMemPtr->clientStatus != 3 || cMemPtr->nextPid != getpid()*(i+1));
            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            printf("Client leaves shaved\n");
            cMemPtr->clientStatus = 0;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);
        }
        else if (cMemPtr->queueLength > cMemPtr->waitingClients)
        {
            cMemPtr->waitingClients++;
            msgsnd(queue, &msg, 10, 0);
            printf("Client sits in the waiting room\n");

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            while(cMemPtr->nextPid != getpid()*(i+1));

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            printf("Client sits on the shaving chair\n");
            cMemPtr->clientStatus = 1;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);


            while(cMemPtr->clientStatus != 2 || cMemPtr->nextPid != getpid()*(i+1));

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            printf("Client leaves shaved\n");
            cMemPtr->clientStatus = 0;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

        }
        else
        {
            printf("Client leaves due to the lack of free chairs\n");
            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);
        }
    }
    exit(0);
}


void makeChildren(int numOfClients, int numOfShaves)
{
    pid_t pid;
    for (int i=0; i<numOfClients; i++)
    {
        if ((pid = fork()) == 0)
        {
            clientLogic(numOfShaves);
        }
    }

    while (wait(NULL) > 0);
}


int main(int argc, char** argv)
{
    key_t key = ftok(getenv("HOME"), 'c');
    commonMemory = shmget(key, 0, 0622);
    cMemPtr = shmat(commonMemory, NULL, 0);
    queue = msgget(key, 0);
    semSet = semget(key, 1, 0);
    struct data sharedData = *cMemPtr;

    makeChildren(atoi(argv[1]), atoi(argv[2]));
}