
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>


void readAndExecute(char* fileName, int time, int memory)
{
    FILE* file = fopen(fileName, "r");
    if (file == NULL)
    {
        printf("File opening error");
        exit(-1);
    }

    size_t bufSize = 100;
    char* buffer = calloc(bufSize, sizeof(char));

    while (getline(&buffer, &bufSize, file) != -1)
    {

        char* bufferCopy = calloc(bufSize, sizeof(char));
        strcpy(bufferCopy, buffer);

        char** arguments = NULL;
        char* p = strtok(bufferCopy, " ");
        int n_spaces = 0;

        while(p)
        {
            arguments = realloc(arguments, sizeof(char*)* ++n_spaces);
            if (arguments == NULL) exit(-1);

            arguments[n_spaces-1] = p;
            p = strtok(NULL, " ");
        }

        arguments = realloc(arguments, sizeof(char*) * (n_spaces + 1));
        arguments[n_spaces] = 0;

        for (int i=0; i<=n_spaces; i++)
            strtok(arguments[i], "\n");

        int result = 0;
        pid_t child = vfork();

        if (child < 0) exit(-1);
        else if (child == 0)
        {
            struct rlimit limits;

            limits.rlim_max = time;
            limits.rlim_cur = RLIM_INFINITY;
            setrlimit(RLIMIT_CPU, &limits);

            limits.rlim_max = memory * 1048576;
            limits.rlim_cur = RLIM_INFINITY;
            setrlimit(RLIMIT_AS, &limits);

            if (execvp(arguments[0], arguments) == -1)
            {
                if (execv(basename(arguments[0]), arguments) == -1)
                {
                    exit(-1);
                }
            }
            exit(0);
        }
        else
        {
            struct rusage stats;
            wait3(&result, 0, &stats);
            if (WIFEXITED(result) && WEXITSTATUS(result) != 0)
            {
                printf("!!!Operation terminated!!!");
                exit(-1);
            }
            else
            {
                printf("Resources:\tUser time: %f\tSystem time: %f\n\n", stats.ru_utime.tv_usec*1.0/1e6 + stats.ru_utime.tv_sec,
                       stats.ru_stime.tv_usec*1.0/1e6 + stats.ru_stime.tv_sec);
            }
        }

        free(bufferCopy);
        free(arguments);
    }

    free(buffer);
    fclose(file);
}


int parse(int argc, char** argv)
{
    if (argc != 4) return -1;
    char* fileName = argv[1];
    int time = atoi(argv[2]);
    int memory = atoi(argv[3]);
    readAndExecute(fileName, time, memory);
    return 0;
}


void printHelp()
{
    printf("Possible arguments:\n\tfilePath");
}


int main(int argc, char** argv)
{
    if (parse(argc, argv) != 0) printHelp();
    return 0;
}
