#include <semaphore.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>



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


/*
 * Doesn't work for some reason. Had to implement explicitly.
 * Stopping on the sem_wait in this attempt and in every attempt to come.
 * kurwa...
 *
void thingsThatBarbersDo(pid_t pid)
{
    sem_post(sem);

    while(cMemPtr->clientStatus != 2);

    sem_wait(sem);

    printf("Shaving starts\t\t%d\t%ld\n", pid, getTime());
    printf("Shaving ends\t\t%d\t%ld\n", pid, getTime());
    cMemPtr->clientStatus = 3;

    sem_post(sem);

    while(cMemPtr->clientStatus != 0);

    sem_wait(sem);

    cMemPtr->nextPid = getpid();
}
*/


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
        sem_wait(sem);

        cMemPtr->clientStatus = -1;

        if (cMemPtr->barberIsSleeping == 0 && check == 1)
        {
            msgrcv(queue, &msg, 10, 0, IPC_NOWAIT);
            printf("Barber wakes up\t\t\t%ld\n", getTime());

            cMemPtr->clientStatus = 1;

            sem_post(sem);
            while(cMemPtr->clientStatus != 2);
            sem_wait(sem);

            printf("Shaving starts\t\t%d\t%ld\n", msg.pid.pid, getTime());
            printf("Shaving ends\t\t%d\t%ld\n", msg.pid.pid, getTime());
            cMemPtr->clientStatus = 3;

            sem_post(sem);
            while(cMemPtr->clientStatus != 0);
            sem_wait(sem);

            cMemPtr->nextPid = getpid();

        }
        else if (cMemPtr->waitingClients > 0 && cMemPtr->barberIsSleeping == 0)
        {
            msgrcv(queue, &msg, 10, 0, IPC_NOWAIT);
            printf("Inviting a client\t%d\t%ld\n", msg.pid.pid, getTime());
            cMemPtr->waitingClients--;
            cMemPtr->nextPid = msg.pid.code;

            sem_post(sem);
            while(cMemPtr->clientStatus != 2);
            sem_wait(sem);

            printf("Shaving starts\t\t%d\t%ld\n", msg.pid.pid, getTime());
            printf("Shaving ends\t\t%d\t%ld\n", msg.pid.pid, getTime());
            cMemPtr->clientStatus = 3;

            sem_post(sem);
            while(cMemPtr->clientStatus != 0);
            sem_wait(sem);

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

        sem_post(sem);
    }
}



void signalHandler()
{
    msgctl(queue, IPC_RMID, NULL);

    munmap(cMemPtr, sizeof(struct data));
    shm_unlink("mem");

    sem_close(sem);

    exit(0);
}



int main(int argc, char** argv)
{
    struct data sharedData;
    sharedData.barberIsSleeping = 0;
    sharedData.waitingClients = 0;
    sharedData.queueLength = atoi(argv[1]);
    sharedData.clientStatus = 0;
    sharedData.nextPid = getpid();

    commonMemory = shm_open("mem", O_CREAT | O_RDWR, 0622);
    ftruncate(commonMemory, sizeof(struct data));
    cMemPtr = malloc(sizeof(struct data));
    cMemPtr = mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, commonMemory, 0);

    key_t key = ftok(getenv("HOME"), 'c');
    queue = msgget(key, IPC_CREAT | 0622);

    sem = sem_open("sem", O_CREAT | O_RDWR, 0622, 1);

    *cMemPtr = sharedData;

    signal(SIGTERM, signalHandler);

    barberLogic();
}
