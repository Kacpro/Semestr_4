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
    int clientPid;
    int barberPid;
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


void barberLogic()
{
    struct sembuf* sops = calloc(1, sizeof(struct sembuf));
    sops[0].sem_flg = 0;

    cMemPtr->barberPid = getpid();

    struct msgbuf
    {
        long mtype;
        pid_t pid;
        char mtext[1];
    };

    struct msgbuf msg;
    msg.mtype = 1;


    while(1)
    {
        sem_wait(sem);


        if (cMemPtr->status == WAKES && cMemPtr->barberPid == getpid())
        {
            cMemPtr->status = SITS_FIRST;
            printf("Barber wakes up\t\t\t\t%ld\n", getTime());
            sem_post(sem);
            continue;
        }
        else if (cMemPtr->status == START && cMemPtr->barberPid == getpid())
        {
            cMemPtr->status = END;
            printf("Barber starts cutting\t\t%d\t%ld\n", cMemPtr->clientPid, getTime());
            sem_post(sem);
            continue;
        }
        else if (cMemPtr->status == END && cMemPtr->barberPid == getpid())
        {
            cMemPtr->status = LEAVES;
            printf("Barber ends cutting\t\t%d\t%ld\n", cMemPtr->clientPid, getTime());
            sem_post(sem);
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
                printf("Barber invites next client\t%d\t%ld\n", msg.pid, getTime());
            }
            else
            {
                cMemPtr->status = SLEEP;
                cMemPtr->clientPid = 0;
                printf("Barber falls asleep\t\t\t%ld\n", getTime());
            }
            sem_post(sem);
            continue;
        }
        else
        {
            sem_post(sem);
        }
    }
}



void signalHandler(int sig)
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
    sharedData.status = INVITES;
    sharedData.waitingClients = 0;
    sharedData.queueLength = atoi(argv[1]);
    sharedData.clientPid = 0;


    commonMemory = shm_open("mem", O_CREAT | O_RDWR, 0622);
    ftruncate(commonMemory, sizeof(struct data));
    cMemPtr = malloc(sizeof(struct data));
    cMemPtr = mmap(NULL, sizeof(struct data), PROT_READ | PROT_WRITE, MAP_SHARED, commonMemory, 0);

    key_t key = ftok(getenv("HOME"), 'c');
    queue = msgget(key, IPC_CREAT | 0622);
//    printf("%d\n", queue);

    sem = sem_open("sem", O_CREAT | O_RDWR, 0622, 1);
    sem_post(sem);

    *cMemPtr = sharedData;

    signal(SIGINT, signalHandler);      //TODO switch to SIGTERM

    barberLogic();
}
