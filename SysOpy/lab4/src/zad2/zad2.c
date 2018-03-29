#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>


enum stats {PID = 1, ASK = 2, PERM = 4, SIG = 8, KILL = 16};

int LEFTREQUESTS;
int REQUESTS;
pid_t* PENDING;
int OPTIONS = 0;


int countEnumValue(char** args, int howMany)
{
    int result = 0;
    for (int i=0; i<howMany; i++)
    {
        if (args[i] == NULL) return result;
        if (!strcmp(args[i], "pid"))  result += PID;
        else if (!strcmp(args[i], "ask"))  result += ASK;
        else if (!strcmp(args[i], "perm")) result += PERM;
        else if (!strcmp(args[i], "sig"))  result += SIG;
        else if (!strcmp(args[i], "kill")) result += KILL;
        else result += 0;
    }

    return result;
}



void signalHandling(int sig, siginfo_t* stats, void* data)
{
    switch(sig)
    {
        case SIGUSR2:
        {
            int sigNumber = rand()%32;
            usleep((rand()%500 + 250)*1000);
            kill(getppid(), SIGRTMIN+sigNumber);

            break;
        }

        case SIGINT:
        {
            printf("Terminated\n");
            kill(-getpid(), SIGKILL);

            break;
        }

        case SIGUSR1:
        {
            if (OPTIONS & ASK) printf("Request received: PID = %d\n", stats->si_pid);

            LEFTREQUESTS--;
            if (LEFTREQUESTS >= 0)
            {
                  PENDING[LEFTREQUESTS] = stats->si_pid;
            }
            else
            {
                if (OPTIONS & PERM) printf("Permission given: PID = %d\n", stats->si_pid);
                kill(stats->si_pid, SIGUSR2);
            }

            if (LEFTREQUESTS == 0)
            {
                for (int i=0; i<REQUESTS; i++)
                {
                    if (OPTIONS & PERM) printf("Permission given: PID = %d\n", PENDING[i]);
                    kill(PENDING[i], SIGUSR2);
                }
            }

            break;
        }

        default:
        {
            if (sig >= SIGRTMIN && sig <= SIGRTMAX)
            {
                if (OPTIONS & SIG) printf("Received real-time signal: PID = %d, Signal number = %d\n", stats->si_pid, sig);
            }
            else
            {
                printf("Unknown signal\n");
                exit(-1);
            }

            break;
        }
    }
}



void childLabour(sigset_t mask)
{
    srand(time(NULL) ^ (getpid()<<16));
    int sleepLength = rand()%11;
    sleep(sleepLength);

    sigset_t newMask = mask;
    sigdelset(&newMask, SIGUSR2);
    usleep((rand()%1000)*1000);
    kill(getppid(), SIGUSR1);

    sigsuspend(&newMask);
    exit(sleepLength);
}



void makeChildren(int numberOfChildren)
{
    pid_t child = 1;

    for (int i=0; i<numberOfChildren && child>0; i++)
    {
        child = fork();
        if ((OPTIONS & PID) && (child > 0)) printf("Creating process %d: PID = %d\n", i+1, child);
        if (child == 0) usleep(1e5);
    }

    sigset_t fullMask;
    sigfillset(&fullMask);

    struct sigaction act;
    act.sa_sigaction = signalHandling;
    act.sa_mask = fullMask;
    act.sa_flags = SA_SIGINFO;

    if (child < 0)
    {
        printf("Fork error");
        exit(-1);
    }
    else if (child == 0)
    {
        sigaction(SIGUSR2, &act, NULL);
        childLabour(fullMask);
    }
    else
    {
        sigdelset(&act.sa_mask, SIGINT);
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGINT, &act, NULL);
        for (int i=0; i<32; i++)
        {
            sigaction(SIGRTMIN+i, &act, NULL);
        }

        int children = numberOfChildren;
        int result;
        pid_t childPID;

        struct exitStatus
        {
            pid_t pid;
            int status;
        };

        struct exitStatus* exitArray = calloc(numberOfChildren, sizeof(exitArray));
        struct exitStatus oneProcess;

        while (children > 0)
        {
            if ((childPID = wait(&result)) > 0)
            {
                oneProcess.pid = childPID;
                oneProcess.status = WEXITSTATUS(result);
                exitArray[numberOfChildren - children] = oneProcess;
                children--;
            }
        }

        if (OPTIONS & KILL)
        {
            for (int i = 0; i < numberOfChildren; i++)
            {
                printf("Exiting process: PID = %d, Status = %d\n", exitArray[i].pid, exitArray[i].status);
            }
        }
    }
}



int parse(int argc, char** argv)
{
    if (argc < 3) return -1;
    if (argc > 8) return -1;
    int numberOfChildren;
    int numberOfRequests;
    if (sscanf(argv[1], "%d", &numberOfChildren) < 1) return -1;
    if (sscanf(argv[2], "%d", &numberOfRequests) < 1) return -1;

    char** stats = calloc(5, sizeof(char*));
    for (int i=3; argv[i]!=NULL; i++)
    {
        char* arg = argv[i];
        if (strcmp(arg, "pid") && strcmp(arg, "ask") && strcmp(arg, "perm") && strcmp(arg, "sig") && strcmp(arg, "kill")) return -1;
        stats[i-3] = calloc(strlen(argv[i]), sizeof(char));
        strcpy(stats[i-3], argv[i]);
    }

    LEFTREQUESTS = REQUESTS = numberOfRequests;
    PENDING = calloc(LEFTREQUESTS, sizeof(pid_t));
    OPTIONS = countEnumValue(stats, argc-3);

    makeChildren(numberOfChildren);

    return 0;
}



int main(int argc, char** argv)
{
    if (parse(argc, argv) == -1)
    {
        printf("Parsing error");
        exit(-1);
    }
}
