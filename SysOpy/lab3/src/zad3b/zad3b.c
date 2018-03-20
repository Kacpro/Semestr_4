#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	if (!strcmp(argv[1], "0"))
	{
		while(1){}
	}
	else
	{
		while(1)
		{
			double* buf = calloc(1000, sizeof(double));
			for (int i=0; i<1000; i++)
			buf[i] = i;
		}
	}
	return 0;
}
