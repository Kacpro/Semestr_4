#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>


pid_t CHILD;
const int SIGRT1 = 1;
const int SIGRT2 = 2;

int SENT = 0;
int RECEIVED_CHILD = 0;
int RECEIVED_PARENT = 0;



void signalHandling(int sig)
{
    switch (sig)
    {
        case SIGINT:
        {
            printf("Terminated\n");
            kill(CHILD, SIGUSR2);
            raise(SIGKILL);

            break;
        }

        case SIGUSR1:
        {
            if (CHILD > 0)
            {
                RECEIVED_PARENT++;
            }
            else
            {
                RECEIVED_CHILD ++;
                kill(getppid(), SIGUSR1);
            }

            break;
        }

        case SIGUSR2:
        {
            printf("Recieved_Child = %d\n", RECEIVED_CHILD);
            raise(SIGKILL);

            break;
        }

        default:
        {
            if (sig != SIGRTMIN+SIGRT1 && sig != SIGRTMIN+SIGRT2)
            {
                printf("Unknown signal\n");
                exit(-1);
            }

            if (sig == SIGRTMIN+SIGRT1)
            {
                if (CHILD > 0)
                {
                    RECEIVED_PARENT++;
                }
                else
                {
                    RECEIVED_CHILD++;
                    kill(getppid(), SIGRTMIN+SIGRT1);
                }
            }
            else
            {
                if (CHILD > 0)
                {
                    RECEIVED_PARENT++;
                }
                else
                {
                    RECEIVED_CHILD++;
                    kill(getppid(), SIGRTMIN+SIGRT2);
                }
            }

            break;
        }
    }
}



void licenceToKill(int numberOfSignals, int type)
{
    sigset_t fullMask;
    sigfillset(&fullMask);

    struct sigaction act;
    act.sa_handler = signalHandling;
    act.sa_mask = fullMask;
    act.sa_flags = 0;

    if ((CHILD = fork()) < 0)
    {
        printf("Fork error\n");
        exit(-1);
    }
    else if (CHILD == 0)
    {
        if (type < 3) sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGUSR2, &act, NULL);
        if (type == 3)
        {
            sigaction(SIGRTMIN + SIGRT1, &act, NULL);
            sigaction(SIGRTMIN + SIGRT2, &act, NULL);
        }

        sigset_t mask;
        sigfillset(&mask);
        if (type < 3)sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        if (type == 3)
        {
            sigdelset(&mask, SIGRTMIN + SIGRT1);
            sigdelset(&mask, SIGRTMIN + SIGRT2);
        }

        if (sigprocmask(SIG_SETMASK, &mask, NULL) < 0)
        {
            printf("Mask error");
            exit(-1);
        }

        while(1)
        {
            pause();
        }
    }
    else
    {
        sigdelset(&act.sa_mask, SIGINT);
        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGINT, &act, NULL);
        if (type == 3)
        {
            sigaction(SIGRTMIN + SIGRT1, &act, NULL);
            sigaction(SIGRTMIN + SIGRT2, &act, NULL);
        }

        sleep(1);       //child must prepare for signals

        if (type < 3)
        {
            for (int i = 0; i < numberOfSignals; i++)
            {
                SENT++;
                kill(CHILD, SIGUSR1);
                if (type ==2 ) pause();
            }
        }
        else
        {
            kill(CHILD, SIGRTMIN+SIGRT1);
            kill(CHILD, SIGRTMIN+SIGRT2);
            SENT = 2;
            sleep(1);       // to not interrupt signal processing with SIGKILL
        }

        kill(CHILD, SIGUSR2);
        waitpid(CHILD, NULL, 0);
    }
}



int parse(int argc, char** argv)
{
    if (argc != 3) return -1;
    int numberOfSignals;
    int type;
    if (sscanf(argv[1], "%d", &numberOfSignals) < 1) return -1;
    if (sscanf(argv[2], "%d", &type) < 1) return -1;
    if (type < 1 || type > 3) return -1;
    licenceToKill(numberOfSignals, type);

    printf("Sent: %d\nRecieved_Parent: %d\n", SENT, RECEIVED_PARENT);

    return 0;
}



int main(int argc, char** argv)
{
    if (parse(argc, argv) != 0)
    {
        printf("Error");
        return -1;
    }

    return 0;
}
