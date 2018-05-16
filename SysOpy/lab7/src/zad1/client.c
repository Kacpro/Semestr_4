
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/wait.h>
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

pid_t client;

void clientLogic(int numberOfShaves)
{
    struct sembuf* sops = calloc(1, sizeof(struct sembuf));
    sops[0].sem_flg = 0;

    struct msgbuf
    {
        long mtype;
        pid_t pid;
        char mtext[0];
    };

    struct msgbuf msg;
    msg.mtype = 1;


    for (int i=0; i<numberOfShaves; i++)
    {
        while(1)
        {
            sops[0].sem_num = 0;
            sops[0].sem_op = -1;
            semop(semSet, sops, 1);

            if (cMemPtr->status == SLEEP && cMemPtr->clientPid == 0 && client == 0)
            {
                cMemPtr->clientPid = getpid();
                cMemPtr->status = WAKES;
                printf("Client wakes the barber up\t%d\t%ld\n", getpid(), getTime());

                sops[0].sem_num = 0;
                sops[0].sem_op = 1;
                semop(semSet, sops, 1);

                continue;
            }
            else if (cMemPtr->status == SITS_FIRST && cMemPtr->clientPid == getpid())
            {
                cMemPtr->status = START;
                client = getpid();
                printf("Client sits on the chair\t%d\t%ld\n", getpid(), getTime());

                sops[0].sem_num = 0;
                sops[0].sem_op = 1;
                semop(semSet, sops, 1);

                continue;
            }
            else if (cMemPtr->status == SITS && cMemPtr->clientPid == getpid())
            {
                cMemPtr->status = START;
                cMemPtr->waitingClients--;
                printf("Client sits on the chair\t%d\t%ld\n", getpid(), getTime());

                sops[0].sem_num = 0;
                sops[0].sem_op = 1;
                semop(semSet, sops, 1);

                continue;
            }
            else if (cMemPtr->status == LEAVES && cMemPtr->clientPid == getpid())
            {
                cMemPtr->status = INVITES;
                cMemPtr->clientPid = 0;
                client = 0;
                printf("Client leaves\t\t\t%d\t%ld\n", getpid(), getTime());

                sops[0].sem_num = 0;
                sops[0].sem_op = 1;
                semop(semSet, sops, 1);

                break;
            }
            else if (client == 0 && cMemPtr->clientPid != 0)
            {
                if (cMemPtr->queueLength <= cMemPtr->waitingClients)
                {
                    printf("Client leaves due to the full queue\t%d\t%ld\n", getpid(), getTime());

                    sops[0].sem_num = 0;
                    sops[0].sem_op = 1;
                    semop(semSet, sops, 1);

                    break;
                } else {
                    client = getpid();
                    msg.pid = getpid();
                    cMemPtr->waitingClients++;
                    msgsnd(queue, &msg, 10, 0);
                    printf("Client sits in the waiting room\t%d\t%ld\n", getpid(), getTime());
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
    key_t key = ftok(getenv("HOME"), 'c');
    commonMemory = shmget(key, 0, 0622);
    cMemPtr = shmat(commonMemory, NULL, 0);
    queue = msgget(key, 0);
    semSet = semget(key, 1, 0);

    makeChildren(atoi(argv[1]), atoi(argv[2]));
}