#include <stdlib.h>
#include <bits/types/FILE.h>
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>


int parse(int argc, char** argv);

void generate(char* fileName, int numberOfRecords, int recordSize);

void sort(char* fileName, int numberOfRecords, int recordSize, char* mode);

void copy(char* sourceName, char* destinationName, int howManyRecords, int bufferSize, int recordSize, char* mode);


void showHelp()
{
	printf("Possible arguments:\n");
	printf("\tgenerate <file_name> <num_of_records> <record_size>\n");
	printf("\tsort <file_name> <num_of_records> <record_size> lib|sys\n");
	printf("\tcopy <source_file_name> <destination_file_name> <num_of_records> <buffer_size> <record_size> lib|sys\n");	
}



int main(int argc, char** argv)
{
    srand(time(NULL));
    if (parse(argc, argv) == -1)
	    showHelp();

    return 0;
}


void showResults(struct rusage start, struct rusage end, char* operation)
{
    long double timeU = (end.ru_utime.tv_sec + (end.ru_utime.tv_usec * 1.0)/1e6 - start.ru_utime.tv_sec - (start.ru_utime.tv_usec * 1.0)/1e6);
    long double timeS = (end.ru_stime.tv_sec + (end.ru_stime.tv_usec * 1.0)/1e6 - start.ru_stime.tv_sec - (start.ru_stime.tv_usec * 1.0)/1e6);
    printf("\nUser time:\t%Lf\nSystem time:\t%Lf\n\n", timeU, timeS);
}


int parse(int argc, char** argv)
{
    struct rusage start, end;

    if (!strcmp(argv[1], "generate"))
    {
        if (argc != 5) return -1;
        char* fileName = argv[2];
        int numberOfRecords = atoi(argv[3]);
        int recordSize = atoi(argv[4]);
        generate(fileName, numberOfRecords, recordSize);
        return 0;
    }
    else if (!strcmp(argv[1], "sort"))
    {
        if (argc != 6) return -1;
        if (strcmp(argv[5], "sys") && strcmp(argv[5], "lib")) return -1;
        char* filePath = argv[2];
        int numberOfRecords = atoi(argv[3]);
        int recordSize = atoi(argv[4]);
        char* mode = argv[5];

        getrusage(RUSAGE_SELF, &start);
        sort(filePath, numberOfRecords, recordSize, mode);
        getrusage(RUSAGE_SELF, &end);
        showResults(start, end, "Sort");

        return 0;
    }
    else if (!strcmp(argv[1], "copy"))
    {
        if (argc != 8) return -1;
        if (strcmp(argv[7], "sys") && strcmp(argv[7], "lib")) return -1;
        char* sourcePath = argv[2];
        char* destinationName = argv[3];
	int howManyRecords = atoi(argv[4]);
        int bufferSize = atoi(argv[5]);
	int recordSize = atoi(argv[6]);
        char* mode = argv[7];

        getrusage(RUSAGE_SELF, &start);
        copy(sourcePath, destinationName, howManyRecords, bufferSize, recordSize, mode);
        getrusage(RUSAGE_SELF, &end);
        showResults(start, end, "Copy");

        return 0;
    }
    else
    {
        return -1;
    }
}


unsigned char* generateRandomString(int length)
{
    unsigned char* result = calloc(length, sizeof(char));
    for (int i=0; i<length; i++)
    {
        result[i] = (unsigned char) (rand() % (256));
    }
    return result;
}


void generate(char* fileName, int numberOfRecords, int recordSize)
{
    FILE* handler = fopen(fileName, "w");
    for (int i=0; i<numberOfRecords; i++)
    {
        unsigned char* record = generateRandomString(recordSize);
        fwrite(record, sizeof(unsigned char), recordSize, handler); //dodać sprawdzanie czy się zapisało
    }
    fclose(handler);
}



void sort(char *fileName, int numberOfRecords, int recordSize, char *mode)
{
    unsigned char* buffer1 = calloc(recordSize, sizeof(char));
    unsigned char* buffer2 = calloc(recordSize, sizeof(char));

    if (!strcmp(mode, "lib"))
    {
        FILE* handler = fopen(fileName, "r+");

        for (int i=1; i<numberOfRecords; i++)
        {
            fseek(handler, recordSize*i, 0);
	    fread(buffer1, sizeof(unsigned char), recordSize, handler);

	    int j = i-1;
	    do
	    {
		fseek(handler, recordSize*j, 0);
		fread(buffer2, sizeof(unsigned char), recordSize, handler);
		if (buffer1[0] < buffer2[0])
		{
			fseek(handler, recordSize*(j+1),0);
			fwrite(buffer2, sizeof(unsigned char), recordSize, handler);
		}
		j--;
	    }
	    while (buffer1[0] < buffer2[0] && j>=0);
	    
	    if (buffer1[0] >= buffer2[0]) fseek(handler, recordSize*(j+2), 0);
	    else fseek(handler, recordSize*(j+1), 0);

	    fwrite(buffer1, sizeof(unsigned char), recordSize, handler);
        }

        fclose(handler);
    }
    else
    {
        int handler = open(fileName, O_RDWR);

        for (int i=1; i<numberOfRecords; i++)
        {
                lseek(handler, recordSize * i, SEEK_SET);
                read(handler, buffer1, recordSize);

	    int j = i-1;
	    do
	    {
		lseek(handler, recordSize*j, SEEK_SET);
		read(handler, buffer2, recordSize);
		if (buffer1[0] < buffer2[0])
		{
		    lseek(handler, recordSize * (j+1), SEEK_SET);
	            write(handler, buffer2, recordSize);
		}
		j--;
	    }
	    while (buffer1[0] < buffer2[0] && j>=0);
	    
	    if (buffer1[0] >= buffer2[0]) lseek(handler, recordSize*(j+2), SEEK_SET);
	    else lseek(handler, recordSize*(j+1), SEEK_SET);

	    write(handler, buffer1, recordSize);		

        }
        close(handler);
    }

    free(buffer1);
    free(buffer2);
}



void copy(char *sourceName, char *destinationName, int howManyRecords, int bufferSize, int recordSize, char *mode)
{
    if (!strcmp(mode, "lib"))
    {
        FILE* handler1 = fopen(sourceName, "r");
        FILE* handler2 = fopen(destinationName, "w");

        char* buffer = calloc(bufferSize, sizeof(char));
	
	int recNum = howManyRecords*recordSize;
        int buf;
        while((buf = fread(buffer, sizeof(char), bufferSize, handler1)) && recNum > 0)
        {
	    if (recNum - buf > 0)	
            	fwrite(buffer, sizeof(char), buf, handler2);
	    else
		fwrite(buffer, sizeof(char), recNum, handler2);
	    recNum -= buf;
        }

        fclose(handler1);
        fclose(handler2);
        free(buffer);
    }
    else
    {
        int handler1 = open(sourceName, O_RDONLY);
        int handler2 = open(destinationName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

        char* buffer = calloc(bufferSize, sizeof(char));

        int buf;
	int recNum = howManyRecords*recordSize;
        while((buf=read(handler1, buffer, bufferSize*sizeof(char))) && recNum > 0)
        {
            if (recNum - buf > 0)
	    	write(handler2, buffer, buf);
	    else
		write(handler2, buffer, recNum);
	    recNum -= buf;;
        }

        close(handler1);
        close(handler2);
        free(buffer);
    }
}


