#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>



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


long getTime()
{
    struct timespec times;

    clock_gettime(CLOCK_MONOTONIC, &times);
    return times.tv_nsec;
}


void barberLogic()
{
    struct sembuf* sops = calloc(1, sizeof(struct sembuf));
    sops[0].sem_flg = 0;

    struct msgbuf {
        long mtype;
        struct pids pid;
        char mtext[0];
    };

    struct msgbuf msg;
    msg.mtype = 1;
    int check = 0;



    while(1)
    {
        sops[0].sem_num = 0;
        sops[0].sem_op = -1;
        semop(semSet, sops, 1);

        cMemPtr->clientStatus = -1;

        if (cMemPtr->barberIsSleeping == 0 && check == 1)
        {
            msgrcv(queue, &msg, 10, 0, IPC_NOWAIT);
            printf("Barber wakes up\t\t\t%ld\n", getTime());

            cMemPtr->clientStatus = 1;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            while(cMemPtr->clientStatus != 2);

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            printf("Shaving starts\t\t%d\t%ld\n", msg.pid.pid, getTime());
            printf("Shaving ends\t\t%d\t%ld\n", msg.pid.pid, getTime());
            cMemPtr->clientStatus = 3;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            while(cMemPtr->clientStatus != 0);

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            cMemPtr->nextPid = getpid();

        }
        else if (cMemPtr->waitingClients > 0 && cMemPtr->barberIsSleeping == 0)
        {
            msgrcv(queue, &msg, 10, 0, IPC_NOWAIT);
            printf("Inviting a client\t%d\t%ld\n", msg.pid.pid, getTime());
            cMemPtr->waitingClients--;
            cMemPtr->nextPid = msg.pid.code;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            while(cMemPtr->clientStatus != 1);

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            printf("Shaving starts\t\t%d\t%ld\n", msg.pid.pid, getTime());
            printf("Shaving ends\t\t%d\t%ld\n", msg.pid.pid, getTime());
            cMemPtr->clientStatus = 2;

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            while(cMemPtr->clientStatus != 0);

            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            cMemPtr->nextPid = getpid();
        }
        else
        {
            if (cMemPtr->barberIsSleeping == 0)
            {
                cMemPtr->barberIsSleeping = 1;
                printf("Barber falls asleep\t\t%ld\n\n", getTime());
            }
        }



        check = cMemPtr->barberIsSleeping;

        sops[0].sem_num = 0;
        sops[0].sem_op = 1;
        semop(semSet, sops, 1);

    }
}


void signalHandler()
{
    msgctl(queue, IPC_RMID, NULL);

    shmdt(cMemPtr);
    shmctl(commonMemory, IPC_RMID, NULL);

    semctl(semSet, 0, IPC_RMID, NULL);

    exit(0);
}


int main(int argc, char** argv)
{
    key_t key = ftok(getenv("HOME"), 'c');

    struct data sharedData;
    sharedData.barberIsSleeping = 0;
    sharedData.waitingClients = 0;
    sharedData.queueLength = atoi(argv[1]);
    sharedData.clientStatus = 0;
    sharedData.nextPid = getpid();


    commonMemory = shmget(key, sizeof(sharedData), IPC_CREAT | 0622);
    cMemPtr = malloc(sizeof(struct data));
    cMemPtr = shmat(commonMemory, NULL, 0);
    queue = msgget(key, IPC_CREAT | 0622);
    semSet = semget(key, 1, IPC_CREAT | 0622);

    struct sembuf* sops = calloc(1, sizeof(struct sembuf));
    sops[0].sem_flg = 0;
    sops[0].sem_num = 0;
    sops[0].sem_op = 1;
    semop(semSet, sops, 1);

    *cMemPtr = sharedData;

    signal(SIGINT, signalHandler);      //TODO switch to SIGTERM

    barberLogic();
}

