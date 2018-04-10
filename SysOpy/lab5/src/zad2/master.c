#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>


void masterLogic(char* fifoName)
{
    mkfifo(fifoName, 0644);
    size_t bufSize = 255;
    char* buffer = calloc(bufSize, sizeof(char));

    FILE* file;

    while (1)
    {
	file = fopen(fifoName, "r");
        if (getline(&buffer, &bufSize, file) > 0)
            printf("%s", buffer);
	fclose(file);
    }
}


int parse(int argc, char** argv)
{
    if (argc != 2) return -1;
    masterLogic(argv[1]);
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
