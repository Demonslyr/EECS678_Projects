#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) 
{
	if (argv[1] == NULL)
	{
		printf("\nI gots no arguments on me!\n");

	}
	else if(argv[2] == NULL)
	{
		printf("\nI gots %d arguments on me! ==> %s\n",argc, argv[1]);
	}
	else
	{
		printf("\nI gots %d arguments on me! ==> %s and %s\n",argc,argv[1],argv[2]);	
	}
	return (52);
} 