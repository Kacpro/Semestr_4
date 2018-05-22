#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv)
{

    if (argc != 3) return -1;
    char* fileName = argv[1];
    int size = atoi(argv[2]);

    srand(time(0));

    FILE* file = fopen(fileName, "w");

    fprintf(file, "%d\n", size);

    for(int i=0; i<size*size; i++)
    {
        fprintf(file, "%lf ", rand()%100/(100.0 * size * size));
        if ((i+1)%9 == 0) fprintf(file, "\n");
    }

    fclose(file);

}