#define _GNU_SOURCE  

#include <stdio.h>
#include <stdlib.h>

int main ()
{

	// File pointers for pipes
	FILE *pipe1;
	FILE *pipe2;

	// Buffor size
	size_t nbytes = 256;

	// Tmp string for pasing between pipes
	char *tmp;

	// Open both pipes, one passing the list, second searching it with grep and saving output to file
	pipe1 = popen ("ls -l", "r");
	pipe2 = popen ("grep ^d > folders.txt", "w");


	// Check the pipes
	if ((!pipe1) || (!pipe2))
	{
	  fprintf (stderr, "One or two pipes failed.\n");
	  return 1;
	}

	// Read from ls results
	tmp = (char *) malloc (nbytes + 1);
	getdelim (&tmp, &nbytes, '\0', pipe1);

	// Close ls pipe, no longer needed
	if (pclose (pipe1) != 0)
	{
	  fprintf (stderr, "Could not run 'ps', or other error.\n");
	}

	// Pass ls results to grep
	fprintf (pipe2, "%s\n\n", tmp);

	// Close grep pipe
	if (pclose (pipe2) < 0)
	{
	 fprintf (stderr, "Could not run 'grep', or other error.\n");
	}

  return 0;
}