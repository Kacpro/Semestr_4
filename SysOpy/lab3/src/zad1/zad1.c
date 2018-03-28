#define _XOPEN_SOURCE 500
#include<dirent.h>
#include<string.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<stdio.h>
#include <time.h>
#include <ftw.h>
#include <printf.h>
#include <unistd.h>
#include <sys/wait.h>


void dirInfo_stat(char* filePath);
int parse(int argc, char** argv);
void dirInfo_nftw(char* filePath);


char* date;
char* operator;




void printHelp()
{
    printf("Required arguments:\n\t- directory path\n\t- operator (< > =)\n\t- date (yyyy-mm-dd)\n\t- mode (0 1)\n");
}




int main(int argc, char** argv)
{
    if (parse(argc, argv) != 0)
        printHelp();
    return 0;
}




int parse(int argc, char** argv)
{
    if (argc != 5) return -1;
    if (!strcmp(argv[2], "=") && !strcmp(argv[2], "<") && !strcmp(argv[2], ">")) return -1;
    char* filePath = argv[1];
    operator = argv[2];
    date = argv[3];
    int mode = atoi(argv[4]);
    if (mode != 1 && mode != 0) return -1;

    if (mode == 0) dirInfo_stat(filePath);
    else dirInfo_nftw(filePath);

    return 0;
}




time_t stringToDate(char *date)
{
    int year = 0, month = 0, day = 0;

    time_t argDate = 0;
    if (sscanf(date, "%4d-%2d-%2d", &year, &month, &day) == 3)
    {
        struct tm argTimeStruct = {0};
        argTimeStruct.tm_year = year;
        argTimeStruct.tm_mon = month;
        argTimeStruct.tm_mday = day;
        argDate = mktime(&argTimeStruct);
    }
    return argDate;
}




char* dateToString(time_t date)
{
    char* timeStr = calloc(20, sizeof(char));
    struct tm* timeinfo = localtime(&date);
    strftime(timeStr, 20, "%F", timeinfo);
    return timeStr;
}




void printInfo(const char* path, const struct stat* stats)
{
    char* sysDate = dateToString((stats->st_mtime));

    printf( (S_ISDIR(stats->st_mode)) ? "d" : "-");
    printf( (stats->st_mode & S_IRUSR) ? "r" : "-");
    printf( (stats->st_mode & S_IWUSR) ? "w" : "-");
    printf( (stats->st_mode & S_IXUSR) ? "x" : "-");
    printf( (stats->st_mode & S_IRGRP) ? "r" : "-");
    printf( (stats->st_mode & S_IWGRP) ? "w" : "-");
    printf( (stats->st_mode & S_IXGRP) ? "x" : "-");
    printf( (stats->st_mode & S_IROTH) ? "r" : "-");
    printf( (stats->st_mode & S_IWOTH) ? "w" : "-");
    printf( (stats->st_mode & S_IXOTH) ? "x" : "-");

    printf("\t%li",stats->st_size);
    printf("\t%s", sysDate);
    printf("\t%s\n", path);
}




int fn(const char* fullPath, const struct stat* stats, int flagType, struct FTW* ftwBuf)
{
    if (flagType == FTW_F)
    {
        time_t argDate = stringToDate(date);
        time_t systemDate = stringToDate(dateToString(stats->st_mtime));

        if (!strcmp(operator, "="))
        {
            if (difftime(argDate, systemDate) == 0)
            {
                printInfo(fullPath, stats);
            }
        }
        else if (!strcmp(operator, ">"))
        {
            if (difftime(argDate, systemDate) < 0)
            {
                printInfo(fullPath, stats);
            }
        }
        else if (!strcmp(operator, "<"))
        {
            if (difftime(argDate, systemDate) > 0)
            {
                printInfo(fullPath, stats);
            }
        }
    }
    return 0;
}




void dirInfo_nftw(char* filePath)
{
    char path[1000];
    nftw(realpath(filePath, path), fn, 20, FTW_PHYS);
}




void dirInfo_stat(char* filePath)
{
    DIR* directory = opendir(filePath);
    if (directory == NULL)
    {
        printf("Directory error");
        exit(-1);
    }

    struct dirent* fileIterator = readdir(directory);
    struct stat stats;

    while (fileIterator != NULL)
    {
        if (strcmp(fileIterator->d_name,".")==0 || strcmp(fileIterator->d_name,"..")==0) {fileIterator = readdir(directory); continue; }

        char buffer[1000];
        strcpy(buffer,filePath);
        strcat(buffer,"/");
        strcat(buffer,fileIterator->d_name);
        lstat(buffer, &stats);

        char path[1000];

        if((S_ISDIR(stats.st_mode)))
        {
            pid_t childPID = vfork();
            if (childPID < 0)
            {
                printf("Fork error");
                exit(-1);
            }
            else if (childPID == 0) {
                dirInfo_stat(realpath(buffer, path));
                exit(0);
            }
        }

        if(!(S_ISREG(stats.st_mode)))
        {
            fileIterator = readdir(directory);
            continue;
        }

        time_t argDate = stringToDate(date);
        time_t systemDate = stringToDate(dateToString(stats.st_mtime));

        if (!strcmp(operator, "="))
        {
            if (difftime(argDate, systemDate) != 0)
            {
                fileIterator = readdir(directory);
                continue;
            }
        }
        else if (!strcmp(operator, ">"))
        {
            if (difftime(argDate, systemDate) >= 0)
            {
                fileIterator = readdir(directory);
                continue;
            }
        }
        else if (!strcmp(operator, "<"))
        {
            if (difftime(argDate, systemDate) <= 0)
            {
                fileIterator = readdir(directory);
                continue;
            }
        }

        printInfo(realpath(buffer, path), &stats);

        fileIterator = readdir(directory);
    }
    closedir(directory);
}
