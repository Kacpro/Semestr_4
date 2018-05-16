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
    return result;
}


void* calculate(void* segment)
{
    perror("calc");
    int c = ceil(FILTER_SIZE*1.0/2);
    printf("seg %d\n", *(int*)segment);
    printf("num %d\n", NUM_OF_THREADS);
    printf("wid %d\n", WIDTH);
    perror("calc start");


    for (int x=1; x<=HEIGHT; x++)
    {
        int maxWidth = (*(int*)segment + 1 != NUM_OF_THREADS)?(*(int*)(segment+1) * WIDTH/NUM_OF_THREADS):WIDTH;
        printf("max %d\n", maxWidth);
        for (int y=(*(int*)(segment) * WIDTH/NUM_OF_THREADS)+1; y<=maxWidth; y++)
        {
            perror(NULL);
            double result = 0;
            for (int i=1; i<=FILTER_SIZE; i++)
            {
                perror("a");
                for (int j=1; j<=FILTER_SIZE; j++)
                {
                    perror("b");
                    printf("i = %d  j = %d  x = %d  y = %d\n", i, j, x, y);
                    result  += (((1>=x-c+i)?1:(x-c+i))-1 < HEIGHT && ((1>=y-c+j)?1:(y-c+j))-1 < WIDTH)?INPUT[((1>=x-c+i)?1:(x-c+i))-1][((1>=y-c+j)?1:(y-c+j))-1]*FILTER[i-1][j-1]:0;
                    perror("c");
                }
            }
            perror("d");
            printf("%d  %d\n", x, y);
            OUTPUT[x-1][y-1] = round(result);
            perror("e");
        }
    }
    perror("calc end");
    return 0;
}


void lifeOfThread()
{
    perror("life");
    OUTPUT = calloc(HEIGHT, sizeof(int*));
    for (int i=0; i<HEIGHT; i++)
        OUTPUT[i] = calloc(WIDTH, sizeof(int));

    pthread_t* threads = calloc(NUM_OF_THREADS, sizeof(pthread_t));

    long start, end;

    start = times(NULL);
perror("life mid");



// tablica na inty do wskaznikow TODO
    // i nie można przekazywać przez referencję bo się zmienia i wyjebuje
    for (int i=0; i<NUM_OF_THREADS; i++)
    {
        perror("st");
        void* ptr = &i;
        printf("ptr %d\n", *(int*)ptr);
        pthread_create(&threads[i], NULL, calculate, ptr);
        perror(NULL);
        sleep(5);
    }

    perror("life after");

    end = times(NULL);

    printf("%ld\n", (end-start)/(sysconf(_SC_CLK_TCK)));
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

    lifeOfThread();

    FILE *output = fopen(outputImage, "w");
    fwrite("P2\n", sizeof(char), 4, output);

    char* height = calloc(10, sizeof(char));
    char* width = calloc(10, sizeof(char));
    sprintf(height, "%d", HEIGHT);
    sprintf(width, "%d", WIDTH);
    strcat(width, " ");
    strcat(width, height);
    strcat(width, "\n");
    fwrite(width, sizeof(char), 10, output);

    fwrite("15\n", sizeof(char), 5, output);

    char* buf = calloc(10, sizeof(char));
    for (int i=0; i<HEIGHT; i++)
    {
        for (int j=0; j<WIDTH; j++)
        {
            sprintf(buf, "%d", OUTPUT[i][j]);
            strcat(buf, " ");
            fwrite(buf, sizeof(char), sizeof(buf), output);
        }
        fwrite("\n", sizeof(char), 2, output);
    }


    return 0;
}