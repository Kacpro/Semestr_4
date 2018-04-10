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
                kill(CHILD, SIGKILL);
            }
            else
            {
                STOPPED = 0;
                CHILD = fork();
		if (CHILD < 0)
		{
			printf("Fork error");
			exit(-1);
		}
		else if (CHILD == 0) execl("./script", "script", NULL);
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
    	execl("./script", "script", NULL);
    }
}
