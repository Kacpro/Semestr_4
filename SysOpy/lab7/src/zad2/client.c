
#include <sys/param.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>


struct data
{
    int waitingClients;
    int barberIsSleeping;
    int queueLength;
    int clientStatus;
    int nextPid;
};


struct data* cMemPtr;
sem_t* sem;
int queue;
int commonMemory;


long getTime()
{
    struct timespec times;

    clock_gettime(CLOCK_MONOTONIC, &times);
    return times.tv_nsec;
}


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

        sem_wait(sem);

        msg.pid.code = getpid()*(i+1);
        msg.pid.pid = getpid();

        if (cMemPtr->barberIsSleeping == 1)
        {
            cMemPtr->barberIsSleeping = 0;
            printf("Client wakes the barber up\t\t\t%d\t%ld\n", getpid(), getTime());
            msgsnd(queue, &msg, 10, 0);

            cMemPtr->nextPid = getpid() * (i+1);

            sem_post(sem);
            while(cMemPtr->clientStatus != 1 || cMemPtr->nextPid != getpid()*(i+1));
            sem_wait(sem);

            printf("Client sits on the shaving chair\t\t%d\t%ld\n", getpid(), getTime());
            cMemPtr->clientStatus = 2;

            sem_post(sem);
            while(cMemPtr->clientStatus != 3 || cMemPtr->nextPid != getpid()*(i+1));
            sem_wait(sem);

            printf("Client leaves shaved\t\t\t\t%d\t%ld\n", getpid(), getTime());
            cMemPtr->clientStatus = 0;

            sem_post(sem);

        }
        else if (cMemPtr->queueLength > cMemPtr->waitingClients)
        {
            cMemPtr->waitingClients++;
            msgsnd(queue, &msg, 10, 0);
            printf("Client sits in the waiting room\t\t\t%d\t%ld\n", getpid(), getTime());

            sem_post(sem);
            while(cMemPtr->nextPid != getpid()*(i+1));
            sem_wait(sem);

            printf("Client sits on the shaving chair\t\t%d\t%ld\n", getpid(), getTime());
            cMemPtr->clientStatus = 2;

            sem_post(sem);
            while(cMemPtr->clientStatus != 3 || cMemPtr->nextPid != getpid()*(i+1));
            sem_wait(sem);

            printf("Client leaves shaved\t\t\t\t%d\t%ld\n", getpid(), getTime());
            cMemPtr->clientStatus = 0;

            sem_post(sem);
        }
        else
        {
            printf("Client leaves due to the lack of free chairs\t%d\t%ld\n", getpid(), getTime());

            sem_post(sem);

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
    commonMemory = shm_open("mem", O_RDWR, 0622);
    ftruncate(commonMemory, sizeof(struct data));
    cMemPtr = mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, commonMemory, 0);

    key_t key = ftok(getenv("HOME"), 'c');
    queue = msgget(key, 0);

    sem = sem_open("sem", O_RDWR, 0622, 1);

    makeChildren(atoi(argv[1]), atoi(argv[2]));
}