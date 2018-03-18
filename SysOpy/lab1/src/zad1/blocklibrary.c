#include<stdlib.h>
#include<string.h>
#include<math.h>

char staticArray[8192][8192];


// dynamic allocation
char** createArray(int arraySize, int blockSize);
void deleteArrayDynamic(char** array, int arraySize);
void addBlockDynamic(char** array, char* block, int position, int blockSize);
void deleteBlockDynamic(char** array, int position);
char* findBlockDyn(char** array, int position, int arraySize, int blockSize);


// static allocation
void addBlockStatic(char* block, int position, int blockSize);
char* findBlockStat(int position, int arraySize, int blockSize);


// helper functions
int sumChar(char* block, int blockSize);



char** createArray(int arraySize, int blockSize)
{
	char** arr = calloc(arraySize, sizeof(char*));
	return arr;
}



void deleteArrayDynamic(char** array, int arraySize)
{
	for (int i=0; i<arraySize; i++)
	{
		free(array[i]);
	}
	free(array);
}



void addBlockStatic(char* block, int position, int blockSize)
{
	strncpy(block, staticArray[position], blockSize);
}



void addBlockDynamic(char** array, char* block, int position, int blockSize)
{
	array[position] = block;
}



void deleteBlockDynamic(char** array, int position)
{
	free(array[position]);
	array[position] = NULL;
}



int sumChar(char* block, int blockSize)
{
	int sum = 0;
	for (int i=0; i<blockSize; i++)
		sum += (int)block[i];
	return sum;
}



char* findBlockDyn(char** array, int position, int arraySize, int blockSize)
{
	if (array == NULL) return NULL;
	if (array[position] == NULL) return NULL;
	
	int first = 0;
	while ((array[first] == NULL || first == position) && first < arraySize)
	{
		first++;
	}

	if (first == arraySize) return NULL;
	
	int min = abs(sumChar(array[first], blockSize) - sumChar(array[position], blockSize));
	int best = first;
	for (int i=first+1; i<arraySize; i++)
	{
		if (array[i]!=NULL && i != position)
		{
			if (abs(sumChar(array[i], blockSize) - sumChar(array[position], blockSize)) < min) best = i;
		}
	}
	return array[best];
}



char* findBlockStat(int position, int arraySize, int blockSize)
{
        int best = 0;
	if (position == 0) best = 1;
	int min = abs(sumChar(staticArray[best], blockSize) - sumChar(staticArray[position], blockSize));
        for (int i=1; i<arraySize; i++)
        {
	        if (i != position)
	        {
		        if (abs(sumChar(staticArray[i], blockSize) - sumChar(staticArray[position], blockSize)) < min) best = i;
		}
        }
        return staticArray[best];
}



























