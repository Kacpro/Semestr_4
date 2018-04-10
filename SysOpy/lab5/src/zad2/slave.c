#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <time.h>

void slavLogic(char* fifoName, int N)
{
    FILE* fifo;
    printf("%d\n", getpid());
    char date[50];
    char msg[255];
    srand(time(0));

    for (int i=0; i<N; i++)
    {
        sprintf(msg, "%d", getpid());
        fifo = fopen(fifoName, "w");

        FILE* datePipe = popen("date", "r");
        fread(date, sizeof(char), 50, datePipe);
        pclose(datePipe);

        strcat(msg, "\t");
        strcat(msg, date);

        fwrite(msg, sizeof(char), 255, fifo);
        fclose(fifo);

        sleep(rand()%(4) + 2);
    }
    fifo = fopen(fifoName, "w");
    fwrite("\n\n", sizeof(char), 5, fifo);
    fclose(fifo);
}


int parse(int argc, char** argv)
{
    if (argc != 3) return -1;
    int N = atoi(argv[2]);
    slavLogic(argv[1], N);
    return 0;
}


int main(int argc, char** argv)
{
    if (parse(argc, argv) != 0)
    {
        printf("Error");
        exit(-1);
    }
}