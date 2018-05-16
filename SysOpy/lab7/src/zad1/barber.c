#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>



enum status
{
    SLEEP,
    WAKES,
    SITS,
    SITS_FIRST,
    START,
    END,
    LEAVES,
    INVITES
};

struct data
{
    int waitingClients;
    int queueLength;
    enum status status;
    pid_t clientPid;
    pid_t barberPid;
};



struct data* cMemPtr;
int semSet;
int queue;
int commonMemory;



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
        pid_t pid;
        char mtext[0];
    };

    struct msgbuf msg;
    msg.mtype = 1;

    perror(NULL);


    while(1)
    {
        sops[0].sem_num = 0;
        sops[0].sem_op = -1;
        semop(semSet, sops, 1);

//        perror(NULL);

        if (cMemPtr->status == WAKES && cMemPtr->barberPid == getpid())
        {
            cMemPtr->status = SITS_FIRST;
            printf("Barber wakes up\t\t\t\t%ld\n", getTime());

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            continue;
        }
        else if (cMemPtr->status == START && cMemPtr->barberPid == getpid())
        {
            cMemPtr->status = END;
            printf("Barber starts cutting\t\t%d\t%ld\n", cMemPtr->clientPid, getTime());

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            continue;
        }
        else if (cMemPtr->status == END && cMemPtr->barberPid == getpid())
        {
            cMemPtr->status = LEAVES;
            printf("Barber ends cutting\t\t%d\t%ld\n", cMemPtr->clientPid, getTime());

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            continue;
        }
        else if (cMemPtr->status == INVITES && cMemPtr->barberPid == getpid())
        {
            //           usleep(1000);
            if (cMemPtr->waitingClients > 0)
            {
                msgrcv(queue, &msg, 10, 0, IPC_NOWAIT);
                cMemPtr->clientPid = msg.pid;
                cMemPtr->status = SITS;
                printf("Barber invites next client\t%d\t%ld\n", cMemPtr->clientPid, getTime());
            }
            else
            {
                cMemPtr->status = SLEEP;
                cMemPtr->clientPid = 0;
                printf("Barber falls asleep\t\t\t%ld\n", getTime());
            }

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

            continue;
        }
        else
        {

            sops[0].sem_num = 0;
            sops[0].sem_op = 1;
            semop(semSet, sops, 1);

        }

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
    sharedData.status = INVITES;
    sharedData.waitingClients = 0;
    sharedData.queueLength = atoi(argv[1]);
    sharedData.clientPid = 0;
    sharedData.barberPid = getpid();


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

