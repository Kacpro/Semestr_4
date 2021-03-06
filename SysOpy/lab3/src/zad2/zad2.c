#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/wait.h>


void readAndExecute(char* fileName)
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
            if (execv(arguments[0], arguments) == -1)
            {
                if (execvp(basename(arguments[0]), arguments) == -1)
                {
                    exit(-1);
                }
            }
            exit(0);
        }
        else
        {
            wait(&result);
            if (WIFEXITED(result) && WEXITSTATUS(result) != 0)
            {
                printf("!!!Operation terminated!!!\nError in: %s\n", arguments[0]);
                exit(-1);
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
    if (argc != 2) return -1;
    char* fileName = argv[1];
    readAndExecute(fileName);
    return 0;
}


void printHelp()
{
    printf("Possible arguments:\n\tfilePath\n");
}


int main(int argc, char** argv)
{
    if (parse(argc, argv) != 0) printHelp();
    return 0;
}
