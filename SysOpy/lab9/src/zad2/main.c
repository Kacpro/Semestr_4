#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <semaphore.h>


struct conf
{
    int numberOfProducers;
    int numberOfConsumers;
    int bufferLength;
    char* fileName;
    int stringLength;
    int searchMode;
    int verbalMode;
    int timeout;
};


struct line
{
    char* text;
    int lineNumber;
};


struct conf CONF;
struct line* BUFFER;
FILE* SOURCE;
int endOfFile = 0;

int consumerPos = 0;
int producerPos = 0;
int numberOfElements = 0;
int lineNumber = 0;

sem_t sem, emptySem, fullSem;


struct conf* parseConf(char* confFileName)
{
    struct conf* configuration = calloc(1, sizeof(struct conf));

    FILE* confFile = fopen(confFileName, "r");

    fscanf(confFile, "%d", &configuration->numberOfProducers);
    fscanf(confFile, "%d", &configuration->numberOfConsumers);
    fscanf(confFile, "%d", &configuration->bufferLength);
    char* buffer = calloc(1000, sizeof(char));
    fscanf(confFile, "%s", buffer);
    (configuration->fileName) = buffer;
    fscanf(confFile, "%d", &configuration->stringLength);
    fscanf(confFile, "%d", &configuration->searchMode);
    fscanf(confFile, "%d", &configuration->verbalMode);
    fscanf(confFile, "%d", &configuration->timeout);

    fclose(confFile);

    return configuration;
}


int addToBuffer()
{
    sem_wait(&sem);

    if (numberOfElements == CONF.bufferLength)
    {
        sem_post(&sem);
        sem_wait(&fullSem);
        sem_post(&fullSem);
        return 0;
    }

    char* buffer = calloc(1000, sizeof(char));

    if (fgets(buffer, 1000, SOURCE))
    {
        struct line* currentLine = calloc(1, sizeof(struct line));
        currentLine->text = calloc(1000, sizeof(char));
        lineNumber++;
        currentLine->lineNumber = lineNumber;
        strcpy(currentLine->text, buffer);

        BUFFER[producerPos] = *currentLine;

        if (CONF.verbalMode) printf("Adding to buffer: %s", currentLine->text);

        numberOfElements++;
        producerPos++;
        if (producerPos == CONF.bufferLength)
            producerPos = 0;

        free(currentLine);
    }
    else if (CONF.timeout == 0)
    {
        endOfFile = 1;
        free(buffer);
        sem_post(&emptySem);
        sem_post(&sem);

        return -1;
    }

    if (numberOfElements == 1)
    {
        sem_post(&emptySem);
    }

    free(buffer);
    sem_post(&sem);

    return 0;
}


enum searchMode
{
    EQUAL = 0,
    LESSER = 1,
    GREATER = 2
};


int removeFromBuffer()
{
    sem_wait(&sem);

    if (numberOfElements == 0 && endOfFile == 0)
    {
        sem_post(&sem);
        sem_wait(&emptySem);
        sem_post(&emptySem);

        return 0;
    }

    if (endOfFile == 1 && numberOfElements == 0)
    {
        sem_post(&sem);
        return -1;
    }

    size_t length = strlen(BUFFER[consumerPos].text) - 1;

    if (CONF.verbalMode)
        printf("Removing from buffer: %s", BUFFER[consumerPos].text);

    if ((length == CONF.stringLength && CONF.searchMode == EQUAL)  ||
        (length <  CONF.stringLength && CONF.searchMode == LESSER) ||
        (length >  CONF.stringLength && CONF.searchMode == GREATER))
    {
        printf("Line: %d\tText: %s", BUFFER[consumerPos].lineNumber, BUFFER[consumerPos].text);
    }

    free(BUFFER[consumerPos].text);
    consumerPos++;
    if (consumerPos == CONF.bufferLength)
        consumerPos = 0;
    numberOfElements--;

    if (numberOfElements == CONF.bufferLength - 1)
    {
        sem_post(&fullSem);
    }

    sem_post(&sem);

    return 0;
}


void* producerLogic(void* arg)
{
    while(1)
    {
        if (addToBuffer() == -1 && CONF.timeout == 0)
        {
            perror("p");
            return NULL;
        }
        usleep(500);
    }
}


void* consumerLogic(void* arg)
{
    while(1)
    {
        if (removeFromBuffer() == -1  && CONF.timeout == 0)
        {
            perror("c");
            return NULL;
        }
        usleep(1000);
    }
}


int main(int argc, char** argv)
{
    if (argc != 2) return -1;
    char* confFileName = argv[1];

    struct conf* confPtr = parseConf(confFileName);
    CONF = *confPtr;
    free(confPtr);

    sem_init(&sem, 0, 1);
    sem_init(&emptySem, 0, 0);
    sem_init(&fullSem, 0, 1);


    BUFFER = calloc(CONF.bufferLength, sizeof(struct line));

    SOURCE = fopen(CONF.fileName, "r");

    pthread_t* threads = calloc(CONF.numberOfConsumers + CONF.numberOfProducers, sizeof(pthread_t));


    for (int i=0; i<CONF.numberOfConsumers; i++)
    {
        pthread_create(&threads[i], NULL, consumerLogic, "b");
    }
    for (int i=0; i<CONF.numberOfProducers; i++)
    {
        pthread_create(&threads[i+CONF.numberOfConsumers], NULL, producerLogic, "a");
    }

    if (CONF.timeout == 0)
    {
        for (int i=0; i< CONF.numberOfConsumers + CONF.numberOfProducers; i++)
            pthread_join(threads[i], NULL);
    }
    else
    {
        sleep(CONF.timeout);
        for (int i=0; i< CONF.numberOfConsumers + CONF.numberOfProducers; i++)
            pthread_cancel(threads[i]);
    }

    free(threads);
    fclose(SOURCE);

    return 0;
}