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


pid_t client;

void clientLogic(int numberOfShaves)
{
    struct sembuf* sops = calloc(1, sizeof(struct sembuf));
    sops[0].sem_flg = 0;

    struct msgbuf
    {
        long mtype;
        pid_t pid;
        char mtext[1];
    };

    struct msgbuf msg;
    msg.mtype = 1;

    msg.pid = getpid();


    for (int i=0; i<numberOfShaves; i++)
    {
        while(1)
        {
            usleep(1000);
            sem_wait(sem);



            if (cMemPtr->status == SLEEP && cMemPtr->clientPid == 0 && client == 0)
            {
                cMemPtr->clientPid = getpid();
                cMemPtr->status = WAKES;
                printf("Client wakes the barber up\t%d\t%ld\n", getpid(), getTime());
                sem_post(sem);
                continue;
            }
            else if (cMemPtr->status == SITS_FIRST && cMemPtr->clientPid == getpid())
            {
                cMemPtr->status = START;
                client = getpid();
                printf("Client sits on the chair\t%d\t%ld\n", getpid(), getTime());
                sem_post(sem);
                continue;
            }
            else if (cMemPtr->status == SITS && cMemPtr->clientPid == getpid())
            {
                cMemPtr->status = START;
                cMemPtr->waitingClients--;
                printf("Client sits on the chair\t%d\t%ld\n", getpid(), getTime());
                sem_post(sem);
                continue;
            }
            else if (cMemPtr->status == LEAVES && cMemPtr->clientPid == getpid())
            {
                cMemPtr->status = INVITES;
                cMemPtr->clientPid = 0;
                client = 0;
                printf("Client leaves\t\t\t%d\t%ld\n", getpid(), getTime());
                sem_post(sem);
                break;
            }
            else if (client == 0 && cMemPtr->clientPid != 0)
            {
                if (cMemPtr->queueLength <= cMemPtr->waitingClients)
                {
                    printf("Client leaves due to the full queue\t%d\t%ld\n", getpid(), getTime());
                    sem_post(sem);
                    break;
                } else {
                    client = getpid();
                    msg.pid = getpid();
                    cMemPtr->waitingClients++;
                    msgsnd(queue, &msg, 10, 0);
                    printf("Client sits in the waiting room\t%d\t%ld\n", getpid(), getTime());
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
    exit(0);
}


void makeChildren(int numOfClients, int numOfShaves)
{
    client = 0;

    for (int i=0; i<numOfClients; i++)
    {
        if (fork() == 0)
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
    printf("%d\n", queue);

    sem = sem_open("sem", O_RDWR, 0622, 1);

    makeChildren(atoi(argv[1]), atoi(argv[2]));
}
