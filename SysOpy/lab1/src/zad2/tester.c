#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/times.h>
#include<unistd.h>

#include "../zad1/blocklibrary.c"


void calculateTimes(FILE* fp, char* operation, clock_t realEnd, clock_t realStart, float sysEnd, float sysStart, float userEnd, float userStart)
{
       	long clk = sysconf(_SC_CLK_TCK);
       	long double nseconds = (long double)((unsigned long)realEnd - (unsigned long)realStart)/(long double)CLOCKS_PER_SEC;
	long double nsecondss = (long double)((unsigned long)sysEnd - (unsigned long)sysStart)/clk;
	long double nsecondsu = (long double)((unsigned long)userEnd - (unsigned long)userStart)/clk;
	printf("\t%s:\t\t Real: %Lf    User: %Lf    System: %Lf\n", operation, nseconds, nsecondsu, nsecondss);
	fprintf(fp,"%s:\t\t Real: %Lf\t User: %Lf\t System: %Lf\n", operation, nseconds, nsecondsu, nsecondss);
}



void fillArrayDyn(char** array, int arraySize, int blockSize)
{
	for (int i=0; i< arraySize; i++)
	{
		char* block = calloc(blockSize, sizeof(char));
		for (int j=0; j<blockSize; j++)
			block[j] = rand()%('z' - 'a') + 'a';
		addBlockDynamic(array, block, i, blockSize);
	}
}



void fillArrayStat(int arraySize, int blockSize)
{
	for (int i=0; i<arraySize; i++)
		for (int j=0; j<blockSize; j++)
			staticArray[i][j] = rand()%('z' - 'a') + 'a';
}


void showHelp()
{
	printf("\nRequired arguments:\n\tarraySize\n\tblockSize\n\tallockMode (0 - static; 1 - dynamic)\n\tfunArg0 (createArray)\n\tfunArg1 (delete and insert n blocks)\n\tfunArg2 (findBlock)\n\tfunArg3 (delete and insert block n times)\n\r\nPositive values in arguments funArg0 - funArg3 are parameters to the specific functions. Other values mean that the function should not be called\n\n");
}


int checkArgs(int argc, char** argv)
{	
	if (argc != 8) return 1;
	if (atoi(argv[3]) != 1 && atoi(argv[3]) != 0) return 1;
	if (atoi(argv[5]) > atoi(argv[1])) return 1;
	return 0;
}


int main(int argc, char** argv)
{
	
	// Checking the arguments
	if (checkArgs(argc, argv) == 1)
	{
		showHelp();
		return 1;
	}

	int arraySize = atoi(argv[1]);
	int blockSize = atoi(argv[2]);
	int allockMode = atoi(argv[3]);
	int funArg0 = atoi(argv[4]);
	int funArg1 = atoi(argv[5]);
	int funArg2 = atoi(argv[6]);
	int funArg3 = atoi(argv[7]);
		
	
	// Opening a file
	FILE *fp;
	if ((fp=fopen("../zad2/raport2.txt", "w")) == NULL) 
	{
		printf ("Can not open the file\n");
		exit(1);
	}
	
	//Time structures 
	struct tms startSys, endSys;
	time_t realStart, realEnd;
	
	srand(time(0));
	char** array;



	// Creating the array
	if (allockMode == 1)
	{	
		if (funArg0 > 0)
		{
			realStart = clock();
			times(&startSys);
				array = createArray(arraySize, blockSize);
				fillArrayDyn(array, arraySize, blockSize);
			realEnd = clock();			
			times(&endSys);
			calculateTimes(fp, "Creating an array", realEnd, realStart, endSys.tms_stime, startSys.tms_stime, endSys.tms_utime, startSys.tms_utime);
		}
		else
		{
			array = createArray(arraySize, blockSize);
                        fillArrayDyn(array, arraySize, blockSize);
		}
	}
	else if (allockMode == 0)
	{
		for (int i=0; i<arraySize; i++)
			for (int j=0; j<blockSize; j++)
				staticArray[i][j] = (char)0;
	}
	

	
	// Deleting and inserting n blocks
	if (funArg1 > 0)
	{
		if (allockMode == 1)
		{
			realStart = clock();			
			times(&startSys);
				for (int i=0; i<funArg1; i++) 
					free(array[i]);
			fillArrayDyn(array, funArg1, blockSize);
			realEnd = clock();		
			times(&endSys);
		}
		else if (allockMode == 0)
		{
			realStart = clock();
			times(&startSys);
				for (int i=0; i<funArg1; i++)
					for (int j=0; j<blockSize; j++)
						staticArray[i][j] = (char)0;
			fillArrayStat(funArg1, blockSize);
			realEnd = clock();
			times(&endSys);
		}
		calculateTimes(fp, "Del. and ins. n elements", realEnd, realStart, endSys.tms_stime, startSys.tms_stime, endSys.tms_utime, startSys.tms_utime);
	}



	// Finding a block
	if (funArg2 >= 0)
	{
		if (allockMode == 1)
		{
			realStart = clock();			
			times(&startSys);
				findBlockDyn(array, funArg2, arraySize, blockSize);
			realEnd = clock();			
			times(&endSys);
		}
		else if (allockMode == 0)
	        {
			realStart = clock();
	                times(&startSys);
		                findBlockStat(funArg2, arraySize, blockSize);
			realEnd = clock();		        
			times(&endSys);
		}
		calculateTimes(fp, "Finding a block", realEnd, realStart, endSys.tms_stime, startSys.tms_stime, endSys.tms_utime, startSys.tms_utime);						             	   }



	//Deleting and inserting block n times
	if (funArg3 > 0)
	{
		if (allockMode == 1)
		{
			realStart = clock();
			times(&startSys);
				for (int i=0; i<funArg3; i++)
				{
					free(array[0]);
					char* block = calloc(blockSize, sizeof(char));
	                		for (int j=0; j<blockSize; j++)
		                		block[j] = rand()%('z'-'a')+'a';
					addBlockDynamic(array, block, 0, blockSize);
				}
			realEnd = clock();			
			times(&endSys);
		}
		else if (allockMode == 0)
		{
                        realStart = clock();
			times(&startSys);
				for (int i=0; i<funArg3; i++)
				{	
					char block[blockSize];
					for (int j=0; j<blockSize; j++)
						block[j] = rand()%('z'-'a')+'a';
					addBlockStatic(block, 0, blockSize);
				}
			realEnd = clock();			
			times(&endSys);
		}
		calculateTimes(fp, "Del. and ins. elem. n times", realEnd, realStart, endSys.tms_stime, startSys.tms_stime, endSys.tms_utime, startSys.tms_utime);
	}
	

	// Deleting a dynamic array
	if (allockMode == 1)
		deleteArrayDynamic(array, arraySize);

	printf("\n");

	// Closing the file
	fclose(fp);
	return 0;
}
