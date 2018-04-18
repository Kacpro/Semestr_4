#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
       #include <sys/wait.h>


char** prepareArguments(char *program, char* separator, int asciiz, int* length)
{  
    char **arguments = NULL;
    char *p = strtok(program, separator);
    int n_spaces = 0;

    while (p)
    {
        arguments = realloc(arguments, sizeof(char *) * ++n_spaces);
        if (arguments == NULL) exit(-1);
  
        arguments[n_spaces - 1] = p;
        p = strtok(NULL, separator); 
    }

    if (asciiz)                                 // adding NULL at the end
    {
        arguments = realloc(arguments, sizeof(char *) * (n_spaces + 1));
        arguments[n_spaces] = 0;
    }
 
    for (int i = 0; i < n_spaces; i++)         // deleting newlines
	if (strchr(arguments[i], '\n') != NULL)
	{ 
		char* buf = calloc(strlen(arguments[i])+1, sizeof(char));
		for (int j=0; j<strlen(arguments[i])-1; j++)
			buf[j] = arguments[i][j];		
	        buf[strlen(arguments[i])-1] = '\0';
		arguments[i] = buf;
	}

    if (length != NULL) *length = n_spaces;
    return arguments;
}



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
        int n_spaces2;

        char** programs = prepareArguments(buffer, "|", 0, &n_spaces2);
	

        int **fd = calloc(n_spaces2 - 1, sizeof(int *));
        for (int i = 0; i < n_spaces2 - 1; i++)
        {
            fd[i] = calloc(2, sizeof(int));
            pipe(fd[i]);
        }


        for (int i=0; i<n_spaces2; i++)
        {

            char **arguments = prepareArguments(programs[i], " ", 1, NULL);

            int result = 0;

            pid_t child = fork();

            if (child < 0) exit(-1);
            else if (child == 0)
            {
                if (i != 0)
                {
                    dup2(fd[i-1][0], 0);        // previous output -> current input
                    close(fd[i-1][0]);
                    close(fd[i-1][1]);
                }
                if (i != n_spaces2 - 1)
                {
                    dup2(fd[i][1], 1);          // current output -> next input
                    close(fd[i][0]);
                    close(fd[i][1]);

                }



                if (execv(arguments[0], arguments) == -1)
                {                    
		    if (execvp(basename(arguments[0]), arguments) == -1)
                    {
				exit(-1);
                    }
                }
            }
            else
            {          
//		usleep(1000);
                if (WIFEXITED(result) && WEXITSTATUS(result) != 0)
                {
                    printf("!!!Operation terminated!!!\nError in: %s\n", arguments[0]);
                    exit(-1);
                }
            }
        }

        free(programs);
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
