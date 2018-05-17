#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <unistd.h>


int** INPUT;
int** OUTPUT;
double** FILTER;
int WIDTH;
int HEIGHT;
int NUM_OF_THREADS;
int FILTER_SIZE;
int* FINISHED;


int** parseImage(char* fileName)
{
    FILE* file = fopen(fileName, "r");

    char* buf = calloc(10, sizeof(char));
    fgets(buf, 10, file);

    fgets(buf, 10, file);
    WIDTH = atoi(strtok(buf, " "));
    HEIGHT = atoi(strtok(NULL, "\n\0"));

    fgets(buf, 10, file);
    free(buf);

    int** result;
    result = calloc(HEIGHT, sizeof(int*));
    for (int i=0; i<HEIGHT; i++)
        result[i] = calloc(WIDTH, sizeof(int));

    char* line = calloc(128, sizeof(char));
    int i = 0;
    while(fgets(line, 128, file))
    {
        result[i/WIDTH][i%WIDTH] = atoi(strtok(line, " \n"));
        i++;
        char* buf;
        while((buf = strtok(NULL, " \n")) != NULL)
        {
            result[i/WIDTH][i%WIDTH] = atoi(buf);
            i++;
        }
    }

    free(line);

    return result;
}


double** parseFilter(char* fileName)
{
    FILE* file = fopen(fileName, "r");

    char* buf = calloc(10, sizeof(char));
    fgets(buf, 10, file);
    FILTER_SIZE = atoi(buf);

    double** result;
    result = calloc(FILTER_SIZE, sizeof(double*));
    for (int i=0; i<HEIGHT; i++)
        result[i] = calloc(FILTER_SIZE, sizeof(double));

    char* line = calloc(128, sizeof(char));
    int i = 0;
    while(fgets(line, 128, file))
    {
        sscanf(strtok(line, " \n"), "%lf", &result[i/FILTER_SIZE][i%FILTER_SIZE]);

        i++;
        char* buf;
        while((buf = strtok(NULL, " \n")) != NULL)
        {
            sscanf(buf, "%lf", &result[i/FILTER_SIZE][i%FILTER_SIZE]);
            i++;
        }

    }

    free(line);

    return result;
}


void* calculate(void* segment)
{
    int c = ceil(FILTER_SIZE*1.0/2);
    int maxWidth = (*(int*)segment + 1 != NUM_OF_THREADS)?(((*(int*)segment)+1) * WIDTH/NUM_OF_THREADS):WIDTH;

    for (int x=1; x<=HEIGHT; x++)
    {
        for (int y=(*(int*)(segment) * WIDTH/NUM_OF_THREADS)+1; y<=maxWidth; y++)
        {
            double result = 0;
            for (int i=1; i<=FILTER_SIZE; i++)
            {
                for (int j=1; j<=FILTER_SIZE; j++)
                {
                    result  += (((1>=x-c+i)?1:(x-c+i))-1 < HEIGHT && ((1>=y-c+j)?1:(y-c+j))-1 < WIDTH)?INPUT[((1>=x-c+i)?1:(x-c+i))-1][((1>=y-c+j)?1:(y-c+j))-1]*FILTER[i-1][j-1]:0;
                }
            }
            OUTPUT[x-1][y-1] = round(result);
        }
    }

    FINISHED[*(int*)segment] = 1;

    return 0;
}


void doSomeWork()
{
    OUTPUT = calloc(HEIGHT, sizeof(int*));
    for (int i=0; i<HEIGHT; i++)
        OUTPUT[i] = calloc(WIDTH, sizeof(int));

    pthread_t* threads = calloc(NUM_OF_THREADS, sizeof(pthread_t));

    long start, end;
    start = times(NULL);

    int** ints = calloc(NUM_OF_THREADS, sizeof(int*));
    for (int i=0; i<NUM_OF_THREADS; i++)
    {
        ints[i] = calloc(1, sizeof(int));
        *ints[i] = i;
    }

    for (int i=0; i<NUM_OF_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, calculate, ints[i]);
    }

    while(1)
    {
        int flag = 1;
        for (int i=0; i<NUM_OF_THREADS; i++)
            if (FINISHED[i] == 0) flag = 0;
        if (flag == 1)
            break;
    }

    end = times(NULL);
    free(threads);
    free(ints);

    printf("%lf\n", (end-start)/(1e-6*sysconf(_SC_CLK_TCK)));
}


void writeToFile(char* fileName)
{
    FILE *output = fopen(fileName, "w");

    fprintf(output, "P2\n");
    fprintf(output, "%d %d\n",WIDTH, HEIGHT);
    fprintf(output, "255\n");

    for (int i=0; i<HEIGHT; i++)
    {
        for (int j=0; j<WIDTH; j++)
        {
            fprintf(output, "%d ", OUTPUT[i][j]);
        }
        fprintf(output, "\n");
    }
}


int main(int argc, char** argv)
{
    if(argc != 5) return -1;
    NUM_OF_THREADS = atoi(argv[1]);
    char* inputImage = argv[2];
    char* filterFile = argv[3];
    char* outputImage = argv[4];

    INPUT = parseImage(inputImage);

    FILTER = parseFilter(filterFile);

    FINISHED = calloc(NUM_OF_THREADS, sizeof(int));

    for (int i=0; i<NUM_OF_THREADS; i++)
        FINISHED[i] = 0;

    doSomeWork();

    writeToFile(outputImage);

    free(INPUT);
    free(OUTPUT);
    free(FILTER);

    return 0;
}