#include <bits/types/time_t.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int STOPPED = 0;
pid_t CHILD;


void signalHandling(int sig)
{
    switch (sig)
    {
        case SIGTSTP:
        {
            if (!STOPPED)
            {
                STOPPED = 1;
                printf("\nOczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
                kill(CHILD, SIGSTOP);
            }
            else
            {
                STOPPED = 0;
                kill(CHILD, SIGCONT);
            }
            break;
        }

        case SIGINT:
        {
            printf("\nOdebrano sygnał SIGINT\n");
            kill(CHILD, SIGKILL);
            raise(SIGKILL);
            break;
        }

        default:
        {
            exit(-1);
        }
    }
}


int main()
{
    if ((CHILD = fork()) < 0)
    {
        printf("Fork error");
        exit(-1);
    }
    else if (CHILD > 0)
    {
        signal(SIGTSTP, signalHandling);

        struct sigaction act;
        act.sa_handler = signalHandling;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, NULL);

        wait(NULL);
    }
    else if (CHILD == 0)
    {
        pid_t child2;
        while (1)
        {
            if ((child2 = fork()) == 0)
            {
                execlp("date", "date", "+%H:%M:%S", NULL);
            }
            else if (child2 < 0)
            {
                printf("Fork error");
                exit(-1);
            }
            else
            {
                wait(NULL);
                sleep(1);
            }
        }
    }
}